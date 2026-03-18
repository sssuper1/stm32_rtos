#include "app_menu.h"
#include "app_param_dict.h"
#include "app_param_update.h"
#include "bsp.h"
#include <stdbool.h>
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
static int MenuKeyHandler_OperationParam(MenuKey_t key);
static void Menu_RenderParamValue(ParamId_t id, int32_t value, char *out, size_t outSize);
static const char *Menu_AckStateText(int32_t state);

typedef struct
{
  ParamId_t id;
  const char *name;
} EditableParamItem_t;

static int32_t s_paramInputValue = 0;
static uint8_t s_paramInputDigits = 0;
static uint8_t s_paramEditIndex = 0;

static const EditableParamItem_t s_editableParams[] =
{
  { PARAM_ID_FREQ_HOP_MODE,    "HOP_MODE" },
  { PARAM_ID_SIGNAL_BANDWIDTH, "BANDWIDTH" },
  { PARAM_ID_FIXED_FREQ,       "FIX_FREQ" },
  { PARAM_ID_ADAPT_FREQ1,      "ADAPT_F1" },
  { PARAM_ID_ADAPT_FREQ2,      "ADAPT_F2" },
  { PARAM_ID_ADAPT_FREQ3,      "ADAPT_F3" },
  { PARAM_ID_ADAPT_FREQ4,      "ADAPT_F4" },
  { PARAM_ID_WAVEFORM_GEAR,    "WAVEFORM" }
};

static int Menu_KeyToDigit(MenuKey_t key)
{
  switch (key)
  {
    case MENU_KEY_NUM_0: return 0;
    case MENU_KEY_NUM_1: return 1;
    case MENU_KEY_NUM_2: return 2;
    case MENU_KEY_NUM_3: return 3;
    case MENU_KEY_NUM_4: return 4;
    case MENU_KEY_NUM_5: return 5;
    case MENU_KEY_NUM_6: return 6;
    case MENU_KEY_NUM_7: return 7;
    case MENU_KEY_NUM_8: return 8;
    case MENU_KEY_NUM_9: return 9;
    default:             return -1;
  }
}

static void Menu_FormatMilli(char *out, size_t outSize, int32_t value)
{
  long integerPart = (long)(value / 1000);
  long fracPart = (long)(value % 1000);

  if (fracPart < 0)
  {
    fracPart = -fracPart;
  }

  (void)snprintf(out, outSize, "%ld.%03ld", integerPart, fracPart);
}

static void Menu_RenderBlankRows(uint8_t startRow, uint8_t endRow)
{
  for (uint8_t row = startRow; row <= endRow; ++row)
  {
    BSP_Lcd_PrintLine(row, "");
  }
}

static void Menu_RenderParamValue(ParamId_t id, int32_t value, char *out, size_t outSize)
{
  if ((out == NULL) || (outSize == 0u))
  {
    return;
  }

  if ((id == PARAM_ID_FIXED_FREQ) ||
      (id == PARAM_ID_ADAPT_FREQ1) ||
      (id == PARAM_ID_ADAPT_FREQ2) ||
      (id == PARAM_ID_ADAPT_FREQ3) ||
      (id == PARAM_ID_ADAPT_FREQ4))
  {
    Menu_FormatMilli(out, outSize, value);
    return;
  }

  (void)snprintf(out, outSize, "%ld", (long)value);
}

static const char *Menu_AckStateText(int32_t state)
{
  switch (state)
  {
    case 1: return "OK";
    case 2: return "FAIL";
    case 3: return "PEND";
    default: return "IDLE";
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
  .key_cb    = MenuKeyHandler_OperationParam
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
  /* 上电即显示 0x08 开机显示参数页。 */
  s_current = &s_function;
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

  if ((node->id == MENU_ID_ROOT) || (node->id == MENU_ID_FUNCTION))
  {
    int32_t hour = 0, minute = 0, second = 0;
    int32_t netJoin = 0;
    int32_t waveform = 0, bandwidth = 0, hopMode = 0;
    int32_t fixedFreq = 0, f1 = 0, f2 = 0, f3 = 0, f4 = 0;
    int32_t spatialFilter = 0, syncMode = 0;
    int32_t txPower = 0, txAtten = 0;
    int32_t ackState = 0, ackCmd = 0;

    (void)APP_ParamDict_GetValue(PARAM_ID_TIME_HOUR, &hour);
    (void)APP_ParamDict_GetValue(PARAM_ID_TIME_MINUTE, &minute);
    (void)APP_ParamDict_GetValue(PARAM_ID_TIME_SECOND, &second);
    (void)APP_ParamDict_GetValue(PARAM_ID_NET_JOIN_STATE, &netJoin);
    (void)APP_ParamDict_GetValue(PARAM_ID_WAVEFORM_GEAR, &waveform);
    (void)APP_ParamDict_GetValue(PARAM_ID_SIGNAL_BANDWIDTH, &bandwidth);
    (void)APP_ParamDict_GetValue(PARAM_ID_FREQ_HOP_MODE, &hopMode);
    (void)APP_ParamDict_GetValue(PARAM_ID_FIXED_FREQ, &fixedFreq);
    (void)APP_ParamDict_GetValue(PARAM_ID_ADAPT_FREQ1, &f1);
    (void)APP_ParamDict_GetValue(PARAM_ID_ADAPT_FREQ2, &f2);
    (void)APP_ParamDict_GetValue(PARAM_ID_ADAPT_FREQ3, &f3);
    (void)APP_ParamDict_GetValue(PARAM_ID_ADAPT_FREQ4, &f4);
    (void)APP_ParamDict_GetValue(PARAM_ID_SPATIAL_FILTER, &spatialFilter);
    (void)APP_ParamDict_GetValue(PARAM_ID_SYNC_MODE, &syncMode);
    (void)APP_ParamDict_GetValue(PARAM_ID_TX_POWER, &txPower);
    (void)APP_ParamDict_GetValue(PARAM_ID_TX_POWER_ATTEN, &txAtten);
    (void)APP_ParamDict_GetValue(PARAM_ID_UART_ACK_STATE, &ackState);
    (void)APP_ParamDict_GetValue(PARAM_ID_UART_ACK_CMD, &ackCmd);

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "T:%02ld:%02ld:%02ld N:%ld",
                     (long)hour, (long)minute, (long)second, (long)netJoin);
      BSP_Lcd_PrintLine(1, buf);
    }

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "W:%ld B:%ld H:%ld",
                     (long)waveform, (long)bandwidth, (long)hopMode);
      BSP_Lcd_PrintLine(2, buf);
    }

    {
      char fbuf[16];
      char buf[32];
      Menu_FormatMilli(fbuf, sizeof(fbuf), fixedFreq);
      (void)snprintf(buf, sizeof(buf), "FIX:%s", fbuf);
      BSP_Lcd_PrintLine(3, buf);
    }

    {
      char fbuf1[16];
      char fbuf2[16];
      char buf[32];
      Menu_FormatMilli(fbuf1, sizeof(fbuf1), f1);
      Menu_FormatMilli(fbuf2, sizeof(fbuf2), f2);
      (void)snprintf(buf, sizeof(buf), "A1:%.7s A2:%.7s", fbuf1, fbuf2);
      BSP_Lcd_PrintLine(4, buf);
    }

    {
      char fbuf3[16];
      char fbuf4[16];
      char buf[32];
      Menu_FormatMilli(fbuf3, sizeof(fbuf3), f3);
      Menu_FormatMilli(fbuf4, sizeof(fbuf4), f4);
      (void)snprintf(buf, sizeof(buf), "A3:%.7s A4:%.7s", fbuf3, fbuf4);
      BSP_Lcd_PrintLine(5, buf);
    }

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "P:%ld ATT:%ld SF:%ld",
                     (long)txPower, (long)txAtten, (long)spatialFilter);
      BSP_Lcd_PrintLine(6, buf);
    }

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "SY:%ld ACK:%s C:%02lX",
                     (long)syncMode,
                     Menu_AckStateText(ackState),
                     (unsigned long)((uint32_t)ackCmd & 0xFFu));
      BSP_Lcd_PrintLine(7, buf);
    }
    return;
  }

  if (node->id == MENU_ID_OPERATION_PARAM)
  {
    const uint8_t itemCount = (uint8_t)(sizeof(s_editableParams) / sizeof(s_editableParams[0]));
    const EditableParamItem_t *item;
    int32_t current = 0;

    if (s_paramEditIndex >= itemCount)
    {
      s_paramEditIndex = 0u;
    }

    item = &s_editableParams[s_paramEditIndex];
    (void)APP_ParamDict_GetValue(item->id, &current);

    if (s_paramInputDigits == 0u)
    {
      s_paramInputValue = current;
    }

    {
      char valueBuf[16];
      char buf[32];
      Menu_RenderParamValue(item->id, current, valueBuf, sizeof(valueBuf));
      (void)snprintf(buf, sizeof(buf), "%u/%u %s",
                     (unsigned int)(s_paramEditIndex + 1u),
                     (unsigned int)itemCount,
                     item->name);
      BSP_Lcd_PrintLine(1, buf);
      (void)snprintf(buf, sizeof(buf), "CUR:%s", valueBuf);
      BSP_Lcd_PrintLine(2, buf);
    }

    {
      char inputBuf[16];
      char buf[32];
      Menu_RenderParamValue(item->id, s_paramInputValue, inputBuf, sizeof(inputBuf));
      (void)snprintf(buf, sizeof(buf), "IN :%s", inputBuf);
      BSP_Lcd_PrintLine(3, buf);
    }

    BSP_Lcd_PrintLine(4, "UP/DN:sel NUM:in");
    BSP_Lcd_PrintLine(5, "OK:apply *:clr");
    BSP_Lcd_PrintLine(6, "#:backspace");
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
      int32_t next = current - 1;
      if (APP_ParamDict_TrySetValue(PARAM_ID_WORK_MODE, next))
      {
        (void)APP_ParamUpdate_Request(PARAM_ID_WORK_MODE);
      }
    }
    return 1;
  }

  if (key == MENU_KEY_DOWN)
  {
    if (current < 3)
    {
      int32_t next = current + 1;
      if (APP_ParamDict_TrySetValue(PARAM_ID_WORK_MODE, next))
      {
        (void)APP_ParamUpdate_Request(PARAM_ID_WORK_MODE);
      }
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

static int MenuKeyHandler_OperationParam(MenuKey_t key)
{
  const uint8_t itemCount = (uint8_t)(sizeof(s_editableParams) / sizeof(s_editableParams[0]));

  if ((itemCount == 0u) || (s_paramEditIndex >= itemCount))
  {
    s_paramEditIndex = 0u;
  }

  if (key == MENU_KEY_UP)
  {
    if (s_paramEditIndex > 0u)
    {
      s_paramEditIndex--;
    }
    s_paramInputDigits = 0u;
    return 1;
  }

  if (key == MENU_KEY_DOWN)
  {
    if ((s_paramEditIndex + 1u) < itemCount)
    {
      s_paramEditIndex++;
    }
    s_paramInputDigits = 0u;
    return 1;
  }

  int digit = Menu_KeyToDigit(key);
  if (digit >= 0)
  {
    if (s_paramInputDigits < 10u)
    {
      s_paramInputValue = s_paramInputValue * 10 + digit;
      s_paramInputDigits++;
    }
    return 1;
  }

  if (key == MENU_KEY_STAR)
  {
    s_paramInputValue = 0;
    s_paramInputDigits = 0u;
    return 1;
  }

  if (key == MENU_KEY_HASH)
  {
    s_paramInputValue /= 10;
    if (s_paramInputDigits > 0u)
    {
      s_paramInputDigits--;
    }
    return 1;
  }

  if (key == MENU_KEY_OK)
  {
    const EditableParamItem_t *item = &s_editableParams[s_paramEditIndex];
    ParamDef_t *def = APP_ParamDict_FindById(item->id);
    int32_t applyValue = s_paramInputValue;

    if (def != NULL)
    {
      if (applyValue < def->min_value)
      {
        applyValue = def->min_value;
      }
      else if (applyValue > def->max_value)
      {
        applyValue = def->max_value;
      }
    }

    if (APP_ParamDict_TrySetValue(item->id, applyValue))
    {
      (void)APP_ParamUpdate_Request(item->id);
    }

    s_paramInputValue = applyValue;
    s_paramInputDigits = 0u;
    return 1;
  }

  return 0;
}

