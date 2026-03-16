#include "app_menu.h"
#include "app_param_dict.h"
#include "bsp.h"
#include <stdio.h>

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
static int MenuKeyHandler_WorkMode(MenuKey_t key);
static int MenuKeyHandler_UartBaud(MenuKey_t key);

static void Menu_RenderBlankRows(uint8_t startRow, uint8_t endRow)
{
  for (uint8_t row = startRow; row <= endRow; ++row)
  {
    BSP_Lcd_PrintLine(row, "");
  }
}

/* 叶子节点暂不指定 key_cb，后续在参数编辑时挂载 */

static const MenuNode_t s_operation_mode =
{
  .id       = MENU_ID_OPERATION_MODE,
  .title    = "OP MODE",
  .parent   = &s_operation,
  .child    = 0,
  .sibling  = &s_operation_param,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = MenuKeyHandler_WorkMode
};

static const MenuNode_t s_operation_param =
{
  .id       = MENU_ID_OPERATION_PARAM,
  .title    = "PARAM",
  .parent   = &s_operation,
  .child    = 0,
  .sibling  = 0,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};

static const MenuNode_t s_operation =
{
  .id       = MENU_ID_OPERATION,
  .title    = "OPERATION",
  .parent   = &s_root,
  .child    = &s_operation_mode,
  .sibling  = &s_function,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};

static const MenuNode_t s_setting_net =
{
  .id       = MENU_ID_SETTING_NET,
  .title    = "NET",
  .parent   = &s_setting,
  .child    = 0,
  .sibling  = &s_setting_uart,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};

static const MenuNode_t s_setting_uart =
{
  .id       = MENU_ID_SETTING_UART,
  .title    = "UART",
  .parent   = &s_setting,
  .child    = &s_setting_uart_baud,
  .sibling  = 0,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};

static const MenuNode_t s_setting_uart_baud =
{
  .id       = MENU_ID_SETTING_UART_BAUD,
  .title    = "BAUD",
  .parent   = &s_setting_uart,
  .child    = 0,
  .sibling  = 0,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = MenuKeyHandler_UartBaud
};

static const MenuNode_t s_setting =
{
  .id       = MENU_ID_SETTING,
  .title    = "SETTING",
  .parent   = &s_root,
  .child    = &s_setting_net,
  .sibling  = 0,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};

static const MenuNode_t s_function =
{
  .id       = MENU_ID_FUNCTION,
  .title    = "FUNCTION",
  .parent   = &s_root,
  .child    = 0,            /* 示例中暂不展开子菜单 */
  .sibling  = &s_setting,
  .render_cb = APP_Menu_DefaultRender,
  .key_cb    = 0
};

static const MenuNode_t s_root =
{
  .id       = MENU_ID_ROOT,
  .title    = "MAIN",
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

void APP_Menu_RefreshCurrent(void)
{
  if ((s_current != 0) && (s_current->render_cb != 0))
  {
    s_current->render_cb(s_current);
  }
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
      if (s_current->render_cb != 0)
      {
        s_current->render_cb(s_current);
      }
      return s_current;
    }
  }

  /* 未被攔截則執行默認導航行為 */
  menu_default_navigate(key);
  return s_current;
}

void APP_Menu_DefaultRender(const MenuNode_t *node)
{
  if (node == 0)
  {
    return;
  }

  /* 行 0 显示当前菜单标题 */
  BSP_Lcd_PrintLine(0, node->title);

  /* 默认先清理子行，防止页面切换残留 */
  Menu_RenderBlankRows(1, 7);

  /* 如果是“串口设置 -> 波特率”节点，顺便在第 1 行显示当前波特率数值 */
  if (node->id == MENU_ID_SETTING_UART_BAUD)
  {
    int32_t baud = 0;
    if (APP_ParamDict_GetValue(PARAM_ID_UART_BAUDRATE, &baud))
    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "BAUD:%ld", (long)baud);
      BSP_Lcd_PrintLine(1, buf);
    }
    return;
  }

  if (node->id == MENU_ID_OPERATION_MODE)
  {
    int32_t mode = 0;
    if (APP_ParamDict_GetValue(PARAM_ID_WORK_MODE, &mode))
    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "MODE:%ld", (long)mode);
      BSP_Lcd_PrintLine(1, buf);
    }
    return;
  }

  if (node->id == MENU_ID_FUNCTION)
  {
    int32_t ip = 0;
    int32_t et = 0, er = 0, vt = 0, vr = 0;
    int32_t bat = 0, tmp = 0, sat = 0, fan = 0;
    int32_t nb = 0, rssi = 0;

    (void)APP_ParamDict_GetValue(PARAM_ID_NET_IP_ADDR, &ip);
    (void)APP_ParamDict_GetValue(PARAM_ID_ETH_TX_CNT, &et);
    (void)APP_ParamDict_GetValue(PARAM_ID_ETH_RX_CNT, &er);
    (void)APP_ParamDict_GetValue(PARAM_ID_VOICE_TX_CNT, &vt);
    (void)APP_ParamDict_GetValue(PARAM_ID_VOICE_RX_CNT, &vr);
    (void)APP_ParamDict_GetValue(PARAM_ID_BATTERY_CAP, &bat);
    (void)APP_ParamDict_GetValue(PARAM_ID_ENV_TEMPERATURE, &tmp);
    (void)APP_ParamDict_GetValue(PARAM_ID_SAT_LOCK, &sat);
    (void)APP_ParamDict_GetValue(PARAM_ID_FAN_STATE, &fan);
    (void)APP_ParamDict_GetValue(PARAM_ID_NEIGHBOR_COUNT, &nb);
    (void)APP_ParamDict_GetValue(PARAM_ID_NEIGHBOR_RSSI, &rssi);

    {
      uint32_t ipu = (uint32_t)ip;
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "IP:%lu.%lu.%lu.%lu",
                     (unsigned long)((ipu >> 24) & 0xFFu),
                     (unsigned long)((ipu >> 16) & 0xFFu),
                     (unsigned long)((ipu >> 8) & 0xFFu),
                     (unsigned long)(ipu & 0xFFu));
      BSP_Lcd_PrintLine(1, buf);
    }

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "ET:%ld ER:%ld", (long)et, (long)er);
      BSP_Lcd_PrintLine(2, buf);
    }

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "VT:%ld VR:%ld", (long)vt, (long)vr);
      BSP_Lcd_PrintLine(3, buf);
    }

    {
      char buf[32];
      long t10 = (long)tmp;
      char sign = (t10 < 0) ? '-' : '+';
      if (t10 < 0)
      {
        t10 = -t10;
      }
      (void)snprintf(buf, sizeof(buf), "BAT:%ld T:%c%ld.%ld", (long)bat, sign, t10 / 10, t10 % 10);
      BSP_Lcd_PrintLine(4, buf);
    }

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "SAT:%ld FAN:%ld", (long)sat, (long)fan);
      BSP_Lcd_PrintLine(5, buf);
    }

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "NB:%ld RSSI:%ld", (long)nb, (long)rssi);
      BSP_Lcd_PrintLine(6, buf);
    }

    {
      int32_t mode = 0;
      (void)APP_ParamDict_GetValue(PARAM_ID_WORK_MODE, &mode);
      char buf[24];
      (void)snprintf(buf, sizeof(buf), "MODE:%ld", (long)mode);
      BSP_Lcd_PrintLine(7, buf);
    }
    return;
  }
}

/* 工作模式编辑逻辑示例：
 * - 在“操作模式设置”菜单下，UP/DOWN 在 0~3 范围内切换
 * - OK / BACK 不拦截，交给默认导航
 */
static int MenuKeyHandler_WorkMode(MenuKey_t key)
{
  int32_t current = 0;
  if (!APP_ParamDict_GetValue(PARAM_ID_WORK_MODE, &current))
  {
    return 0;
  }

  if (key == MENU_KEY_UP)
  {
    if (current > 0)
    {
      (void)APP_ParamDict_TrySetValue(PARAM_ID_WORK_MODE, current - 1);
    }
    return 1;
  }

  if (key == MENU_KEY_DOWN)
  {
    if (current < 3)
    {
      (void)APP_ParamDict_TrySetValue(PARAM_ID_WORK_MODE, current + 1);
    }
    return 1;
  }

  return 0;
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

