#include "app_menu.h"
#include "app_param_dict.h"

/* -------------------- 静态菜单节点定义（示例） -------------------- */

/* 预声明节点，以便互相引用 */
static const MenuNode_t s_root;
static const MenuNode_t s_operation;
static const MenuNode_t s_function;
static const MenuNode_t s_setting;
static const MenuNode_t s_operation_mode;
static const MenuNode_t s_operation_param;
static const MenuNode_t s_setting_net;
static const MenuNode_t s_setting_uart;
static const MenuNode_t s_setting_uart_baud;

/* 前置声明：串口波特率编辑按键处理回调 */
static int MenuKeyHandler_UartBaud(MenuKey_t key);

/* 叶子节点暂不指定 key_cb，后续在参数编辑时挂载 */

static const MenuNode_t s_operation_mode =
{
  .id       = MENU_ID_OPERATION_MODE,
  .title    = "操作模式设置",
  .parent   = &s_operation,
  .child    = 0,
  .sibling  = &s_operation_param,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};

static const MenuNode_t s_operation_param =
{
  .id       = MENU_ID_OPERATION_PARAM,
  .title    = "参数设置",
  .parent   = &s_operation,
  .child    = 0,
  .sibling  = 0,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};

static const MenuNode_t s_operation =
{
  .id       = MENU_ID_OPERATION,
  .title    = "操作",
  .parent   = &s_root,
  .child    = &s_operation_mode,
  .sibling  = &s_function,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};

static const MenuNode_t s_setting_net =
{
  .id       = MENU_ID_SETTING_NET,
  .title    = "网口设置",
  .parent   = &s_setting,
  .child    = 0,
  .sibling  = &s_setting_uart,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};

static const MenuNode_t s_setting_uart =
{
  .id       = MENU_ID_SETTING_UART,
  .title    = "串口设置",
  .parent   = &s_setting,
  .child    = &s_setting_uart_baud,
  .sibling  = 0,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};

static const MenuNode_t s_setting_uart_baud =
{
  .id       = MENU_ID_SETTING_UART_BAUD,
  .title    = "波特率",
  .parent   = &s_setting_uart,
  .child    = 0,
  .sibling  = 0,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = MenuKeyHandler_UartBaud
};

static const MenuNode_t s_setting =
{
  .id       = MENU_ID_SETTING,
  .title    = "设置",
  .parent   = &s_root,
  .child    = &s_setting_net,
  .sibling  = 0,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};

static const MenuNode_t s_function =
{
  .id       = MENU_ID_FUNCTION,
  .title    = "功能",
  .parent   = &s_root,
  .child    = 0,            /* 示例中暂不展开子菜单 */
  .sibling  = &s_setting,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};

static const MenuNode_t s_root =
{
  .id       = MENU_ID_ROOT,
  .title    = "主菜单",
  .parent   = 0,
  .child    = &s_operation, /* 第一級：操作 / 功能 / 設置 */
  .sibling  = 0,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};

/* -------------------- 狀態機內部變量 -------------------- */

static const MenuNode_t *s_current = &s_root;

/* -------------------- 對外接口實現 -------------------- */

const MenuNode_t *APP_Menu_Init(void)
{
  s_current = &s_root;
  if (s_current->render_cb != 0)
  {
    s_current->render_cb(s_current);
  }
  return s_current;
}

const MenuNode_t *APP_Menu_GetCurrent(void)
{
  return s_current;
}

static void menu_default_navigate(MenuKey_t key)
{
  if (s_current == 0)
  {
    return;
  }

  switch (key)
  {
    case MENU_KEY_UP:
      /* 查找上一個兄弟節點（需要從父節點開始遍歷） */
      if ((s_current->parent != 0) && (s_current->parent->child != 0))
      {
        const MenuNode_t *node = s_current->parent->child;
        const MenuNode_t *prev = 0;
        while ((node != 0) && (node != s_current))
        {
          prev = node;
          node = node->sibling;
        }
        if (prev != 0)
        {
          s_current = prev;
        }
      }
      break;

    case MENU_KEY_DOWN:
      if (s_current->sibling != 0)
      {
        s_current = s_current->sibling;
      }
      break;

    case MENU_KEY_OK:
      if (s_current->child != 0)
      {
        s_current = s_current->child;
      }
      break;

    case MENU_KEY_BACK:
      if (s_current->parent != 0)
      {
        s_current = s_current->parent;
      }
      break;

    default:
      /* do nothing */
      break;
  }

  if ((s_current != 0) && (s_current->render_cb != 0))
  {
    s_current->render_cb(s_current);
  }
}

const MenuNode_t *APP_Menu_HandleKey(MenuKey_t key)
{
  if (key == MENU_KEY_NONE)
  {
    return s_current;
  }

  /* 先讓當前節點有機會“攔截”按鍵（例如在參數編輯模式） */
  if ((s_current != 0) && (s_current->key_cb != 0))
  {
    int handled = s_current->key_cb(key);
    if (handled != 0)
    {
      return s_current;
    }
  }

  /* 未被攔截則執行默認導航行為 */
  menu_default_navigate(key);
  return s_current;
}

void APP_Menu_DefaultRender(const MenuNode_t *node)
{
  (void)node;
  /* 這裡暫時不做具體輸出，只是為 LCD/UI 層預留接口。
   * 例如將來可以在這裡調用: LCD_ShowString(0, 0, node->title, ...);
   */
}

/* 串口波特率编辑逻辑示例：
 * - 在“波特率”菜单下，UP/DOWN 用于在几个常见波特率间切换
 * - OK / BACK 目前不做特殊处理，返回 0 让默認導航生效
 */
static int MenuKeyHandler_UartBaud(MenuKey_t key)
{
  int32_t current = 0;
  if (!APP_ParamDict_GetValue(PARAM_ID_UART_BAUDRATE, &current))
  {
    return 0;
  }

  /* 常见波特率表 */
  static const int32_t baudList[] = {9600, 19200, 38400, 57600, 115200, 230400};
  const uint32_t baudCount = (uint32_t)(sizeof(baudList) / sizeof(baudList[0]));

  /* 找到当前所在索引 */
  uint32_t index = 0U;
  for (uint32_t i = 0U; i < baudCount; ++i)
  {
    if (baudList[i] == current)
    {
      index = i;
      break;
    }
  }

  int handled = 0;

  if (key == MENU_KEY_UP)
  {
    if (index > 0U)
    {
      index--;
    }
    handled = 1;
  }
  else if (key == MENU_KEY_DOWN)
  {
    if ((index + 1U) < baudCount)
    {
      index++;
    }
    handled = 1;
  }
  else
  {
    /* 其他按鍵暫不攔截，交給默認導航（例如 BACK 返回上級） */
    handled = 0;
  }

  if (handled != 0)
  {
    (void)APP_ParamDict_TrySetValue(PARAM_ID_UART_BAUDRATE, baudList[index]);
    /* 理論上此處還應觸發 UI 重繪當前數值，後續可在渲染函數中
     * 根據 PARAM_ID_UART_BAUDRATE 的值展示具體數字。 */
  }

  return handled;
}

