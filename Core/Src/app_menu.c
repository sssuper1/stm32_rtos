#include "app_menu.h"
#include "app_param_dict.h"
#include "app_param_update.h"
#include "app_uart_proto.h"
#include "app_uart_rx.h"
#include "bsp.h"
#include "cmsis_os.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* -------------------- 静态菜单节点定义（示例） -------------------- */

/* 预声明节点，以便互相引用 */
static const MenuNode_t s_root;
static const MenuNode_t s_operation;
static const MenuNode_t s_function;
static const MenuNode_t s_setting;
static const MenuNode_t s_operation_mode_group;
static const MenuNode_t s_operation_param_group;
static const MenuNode_t s_operation_ch1_group;
static const MenuNode_t s_operation_workfreq_group;
static const MenuNode_t s_operation_mode;
static const MenuNode_t s_operation_spatial;
static const MenuNode_t s_operation_sync;
static const MenuNode_t s_operation_route;
static const MenuNode_t s_operation_access;
static const MenuNode_t s_operation_hopmode;
static const MenuNode_t s_operation_freq;
static const MenuNode_t s_operation_freq1;
static const MenuNode_t s_operation_freq2;
static const MenuNode_t s_operation_freq3;
static const MenuNode_t s_operation_freq4;
static const MenuNode_t s_operation_bw;
static const MenuNode_t s_operation_mcs;
static const MenuNode_t s_operation_txpower;
static const MenuNode_t s_operation_txatten;

static const MenuNode_t s_function_member;
static const MenuNode_t s_function_member_count;
static const MenuNode_t s_function_member_node;
static const MenuNode_t s_function_member_node_id;
static const MenuNode_t s_function_member_node_ip;
static const MenuNode_t s_function_member_node_hops;
static const MenuNode_t s_function_member_node_rssi;
static const MenuNode_t s_function_member_node_delay;
static const MenuNode_t s_function_member_pos;
static const MenuNode_t s_function_member_pos_lon;
static const MenuNode_t s_function_member_pos_lat;
static const MenuNode_t s_function_member_pos_alt;
static const MenuNode_t s_function_member_detail;
static const MenuNode_t s_function_detail_id;
static const MenuNode_t s_function_detail_filter;
static const MenuNode_t s_function_detail_ip;
static const MenuNode_t s_function_detail_ch1;
static const MenuNode_t s_function_detail_ch1_hop;
static const MenuNode_t s_function_detail_ch1_freq;
static const MenuNode_t s_function_detail_ch1_bw;
static const MenuNode_t s_function_detail_ch1_wave;
static const MenuNode_t s_function_detail_ch1_txpwr;
static const MenuNode_t s_function_detail_ch1_txatten;
static const MenuNode_t s_function_detail_ch1_route;
static const MenuNode_t s_function_detail_ch1_access;
static const MenuNode_t s_function_traffic;
static const MenuNode_t s_function_total_tx;
static const MenuNode_t s_function_total_rx;
static const MenuNode_t s_function_voice;
static const MenuNode_t s_function_voice_tx;
static const MenuNode_t s_function_voice_rx;
static const MenuNode_t s_function_eth;
static const MenuNode_t s_function_eth_tx;
static const MenuNode_t s_function_eth_rx;
static const MenuNode_t s_function_traffic_op;
static const MenuNode_t s_function_selftest;
static const MenuNode_t s_function_self_state;
static const MenuNode_t s_function_sysinfo;
static const MenuNode_t s_function_sys_temp;
static const MenuNode_t s_function_sys_fan;
static const MenuNode_t s_function_sys_sat;
static const MenuNode_t s_function_sys_clock;
static const MenuNode_t s_function_sys_adc;
static const MenuNode_t s_function_clk_src;
static const MenuNode_t s_function_clk_temp;
static const MenuNode_t s_function_clk_cnt;
static const MenuNode_t s_function_clk_sense;

static const MenuNode_t s_setting_device_id;
static const MenuNode_t s_setting_device_name;
static const MenuNode_t s_setting_net;
static const MenuNode_t s_setting_net_biz;
static const MenuNode_t s_setting_net_biz_ip;
static const MenuNode_t s_setting_net_biz_port;
static const MenuNode_t s_setting_net_mgmt;
static const MenuNode_t s_setting_net_mgmt_ip;
static const MenuNode_t s_setting_net_mgmt_port;
static const MenuNode_t s_setting_net_sense;
static const MenuNode_t s_setting_net_sense_ip;
static const MenuNode_t s_setting_net_sense_port;
static const MenuNode_t s_setting_uart;
static const MenuNode_t s_setting_uart_baud;
static const MenuNode_t s_setting_uart_databits;
static const MenuNode_t s_setting_uart_stopbits;
static const MenuNode_t s_setting_uart_parity;
static const MenuNode_t s_setting_uart_flow;
static const MenuNode_t s_setting_location;
static const MenuNode_t s_setting_loc_auto;
static const MenuNode_t s_setting_loc_lon;
static const MenuNode_t s_setting_loc_lat;
static const MenuNode_t s_setting_loc_alt;
static const MenuNode_t s_setting_time;
static const MenuNode_t s_setting_time_auto;
static const MenuNode_t s_setting_time_manual;
static const MenuNode_t s_setting_time_hour;
static const MenuNode_t s_setting_time_min;
static const MenuNode_t s_setting_time_sec;

/* 前置声明：串口波特率编辑按键处理回调 */
static int MenuKeyHandler_OperationItem(MenuKey_t key);
static int MenuKeyHandler_TrafficOp(MenuKey_t key);
static int MenuKeyHandler_MemberNodeList(MenuKey_t key);
static int MenuKeyHandler_DetailRequest(MenuKey_t key);
static int MenuKeyHandler_UartFormat(MenuKey_t key);
static int MenuKeyHandler_UartBaud(MenuKey_t key);
static int MenuKeyHandler_TimeManual(MenuKey_t key);
static void Render_MemberNodeList(const MenuNode_t *node);
static void Render_OperationTree(const MenuNode_t *node);

static int32_t s_paramInputValue = 0;
static uint8_t s_paramInputDigits = 0;
static bool s_paramEditing = false;
static int32_t s_trafficOpValue = 0;
static int32_t s_uartDataBitsIdx = 3;  /* 0:5bit,1:6bit,2:7bit,3:8bit */
static int32_t s_uartStopBitsIdx = 0;  /* 0:1,1:1.5,2:2 */
static int32_t s_uartParityIdx   = 0;  /* 0:none,1:odd,2:even,3:mark,4:space */
static int32_t s_uartFlowIdx     = 0;  /* 0:none,1:XON/XOFF,2:RTS/CTS,3:DTR/DSR,4:mixed,5:DTR/DSR */
static uint8_t s_memberNodeSelect = 1u;
static uint8_t s_adaptFreqEditMask = 0u; /* bit0~bit3: F1~F4 */
static bool s_adaptHopProtectActive = false;

enum
{
  MENU_ID_OPERATION_MODE_GROUP = 94,
  MENU_ID_OPERATION_PARAM_GROUP = 95,
  MENU_ID_OPERATION_CH1_GROUP = 96,
  MENU_ID_OPERATION_WORKFREQ_GROUP = 97,
  MENU_ID_OPERATION_FIXED_GROUP = 98,
  MENU_ID_OPERATION_ADAPT_GROUP = 99,
  MENU_ID_OPERATION_SPATIAL = 100,
  MENU_ID_OPERATION_SYNC = 101,
  MENU_ID_OPERATION_ROUTE = 102,
  MENU_ID_OPERATION_ACCESS = 103,
  MENU_ID_OPERATION_HOPMODE = 104,
  MENU_ID_OPERATION_CH_FREQ = 105,
  MENU_ID_OPERATION_FREQ1 = 106,
  MENU_ID_OPERATION_FREQ2 = 107,
  MENU_ID_OPERATION_FREQ3 = 108,
  MENU_ID_OPERATION_FREQ4 = 109,
  MENU_ID_OPERATION_BW = 110,
  MENU_ID_OPERATION_MCS = 111,
  MENU_ID_OPERATION_TX_POWER = 112,
  MENU_ID_OPERATION_TX_ATTEN = 113,

  MENU_ID_FUNCTION_MEMBER = 200,
  MENU_ID_FUNCTION_MEMBER_COUNT = 201,
  MENU_ID_FUNCTION_MEMBER_NODE = 202,
  MENU_ID_FUNCTION_MEMBER_NODE_ID = 203,
  MENU_ID_FUNCTION_MEMBER_NODE_IP = 204,
  MENU_ID_FUNCTION_MEMBER_NODE_HOPS = 205,
  MENU_ID_FUNCTION_MEMBER_NODE_RSSI = 206,
  MENU_ID_FUNCTION_MEMBER_NODE_DELAY = 207,
  MENU_ID_FUNCTION_MEMBER_POS = 208,
  MENU_ID_FUNCTION_MEMBER_POS_LON = 209,
  MENU_ID_FUNCTION_MEMBER_POS_LAT = 210,
  MENU_ID_FUNCTION_MEMBER_POS_ALT = 211,
  MENU_ID_FUNCTION_MEMBER_DETAIL = 212,
  MENU_ID_FUNCTION_DETAIL_ID = 213,
  MENU_ID_FUNCTION_DETAIL_FILTER = 214,
  MENU_ID_FUNCTION_DETAIL_IP = 215,
  MENU_ID_FUNCTION_DETAIL_CH1 = 216,
  MENU_ID_FUNCTION_DETAIL_CH1_HOP = 217,
  MENU_ID_FUNCTION_DETAIL_CH1_FREQ = 218,
  MENU_ID_FUNCTION_DETAIL_CH1_BW = 219,
  MENU_ID_FUNCTION_DETAIL_CH1_WAVE = 220,
  MENU_ID_FUNCTION_DETAIL_CH1_TXPWR = 221,
  MENU_ID_FUNCTION_DETAIL_CH1_TXATTEN = 222,
  MENU_ID_FUNCTION_DETAIL_CH1_ROUTE = 223,
  MENU_ID_FUNCTION_DETAIL_CH1_ACCESS = 224,
  MENU_ID_FUNCTION_TRAFFIC = 225,
  MENU_ID_FUNCTION_TOTAL_TX = 226,
  MENU_ID_FUNCTION_TOTAL_RX = 227,
  MENU_ID_FUNCTION_VOICE = 228,
  MENU_ID_FUNCTION_VOICE_TX = 229,
  MENU_ID_FUNCTION_VOICE_RX = 230,
  MENU_ID_FUNCTION_ETH = 231,
  MENU_ID_FUNCTION_ETH_TX = 232,
  MENU_ID_FUNCTION_ETH_RX = 233,
  MENU_ID_FUNCTION_TRAFFIC_OP = 234,
  MENU_ID_FUNCTION_SELFTEST = 235,
  MENU_ID_FUNCTION_SELF_STATE = 236,
  MENU_ID_FUNCTION_SYSINFO = 237,
  MENU_ID_FUNCTION_SYS_TEMP = 238,
  MENU_ID_FUNCTION_SYS_FAN = 239,
  MENU_ID_FUNCTION_SYS_SAT = 240,
  MENU_ID_FUNCTION_SYS_CLOCK = 241,
  MENU_ID_FUNCTION_SYS_ADC = 242,
  MENU_ID_FUNCTION_CLK_SRC = 243,
  MENU_ID_FUNCTION_CLK_TEMP = 244,
  MENU_ID_FUNCTION_CLK_CNT = 245,
  MENU_ID_FUNCTION_CLK_SENSE = 246,

  MENU_ID_SETTING_DEVICE_ID = 300,
  MENU_ID_SETTING_DEVICE_NAME = 301,
  MENU_ID_SETTING_NET_BIZ = 302,
  MENU_ID_SETTING_NET_BIZ_IP = 303,
  MENU_ID_SETTING_NET_BIZ_PORT = 304,
  MENU_ID_SETTING_NET_MGMT = 305,
  MENU_ID_SETTING_NET_MGMT_IP = 306,
  MENU_ID_SETTING_NET_MGMT_PORT = 307,
  MENU_ID_SETTING_NET_SENSE = 308,
  MENU_ID_SETTING_NET_SENSE_IP = 309,
  MENU_ID_SETTING_NET_SENSE_PORT = 310,
  MENU_ID_SETTING_UART_DATABITS = 311,
  MENU_ID_SETTING_UART_STOPBITS = 312,
  MENU_ID_SETTING_UART_PARITY = 313,
  MENU_ID_SETTING_UART_FLOW = 314,
  MENU_ID_SETTING_LOCATION = 315,
  MENU_ID_SETTING_LOC_AUTO = 316,
  MENU_ID_SETTING_LOC_LON = 317,
  MENU_ID_SETTING_LOC_LAT = 318,
  MENU_ID_SETTING_LOC_ALT = 319,
  MENU_ID_SETTING_TIME = 320,
  MENU_ID_SETTING_TIME_AUTO = 321,
  MENU_ID_SETTING_TIME_MANUAL = 322,
  MENU_ID_SETTING_TIME_HOUR = 323,
  MENU_ID_SETTING_TIME_MIN = 324,
  MENU_ID_SETTING_TIME_SEC = 325
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

static void Menu_RenderBlankRows(uint8_t startRow, uint8_t endRow)
{
  for (uint8_t row = startRow; row <= endRow; ++row)
  {
    BSP_Lcd_PrintLine(row, "");
  }
}

static const char *Menu_NetStateText(int32_t netJoin)
{
  return (netJoin == 0) ? "IN" : "OUT";
}

static const char *Menu_BandwidthText(int32_t bandwidth)
{
  switch (bandwidth)
  {
    case 0: return "20";
    case 1: return "10";
    case 2: return "5";
    case 3: return "2.5";
    default: return "?";
  }
}

static const char *Menu_HopModeText(int32_t hopMode)
{
  return (hopMode == 0) ? "FIX" : "ADPT";
}

static const char *Menu_OnOffText(int32_t state)
{
  return (state == 0) ? "ON" : "OFF";
}

static const char *Menu_SyncText(int32_t syncMode)
{
  return (syncMode == 0) ? "INT" : "EXT";
}

static const char *Menu_TxPowerText(int32_t txPower)
{
  switch (txPower)
  {
    case 0: return "5W";
    case 1: return "10W";
    case 2: return "20W";
    default: return "?";
  }
}

static void Menu_PrintHeaderPath(const MenuNode_t *node)
{
  char buf[32];
  const MenuNode_t *stack[8];
  uint8_t depth = 0u;

  while ((node != 0) && (depth < (uint8_t)(sizeof(stack) / sizeof(stack[0]))))
  {
    stack[depth++] = node;
    node = node->parent;
  }

  buf[0] = '\0';
  for (int i = (int)depth - 1; i >= 0; --i)
  {
    const char *seg = (stack[i]->title != 0) ? stack[i]->title : "?";
    size_t used = strlen(buf);
    size_t remain = sizeof(buf) - used - 1u;

    if ((used > 0u) && (remain > 0u))
    {
      strncat(buf, "/", remain);
      used = strlen(buf);
      remain = sizeof(buf) - used - 1u;
    }

    if (remain > 0u)
    {
      strncat(buf, seg, remain);
    }
  }

  if (buf[0] == '\0')
  {
    (void)snprintf(buf, sizeof(buf), "MAIN");
  }

  BSP_Lcd_PrintLine(0, buf);
}

/* 叶子节点暂不指定 key_cb，后续在参数编辑时挂载 */

static const MenuNode_t s_operation_mode =
{
  .id       = MENU_ID_OPERATION_MODE,
  .title    = "CUR MODE",
  .parent   = &s_operation_mode_group,
  .child    = 0,
  .sibling  = &s_operation_spatial,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_spatial =
{
  .id       = MENU_ID_OPERATION_SPATIAL,
  .title    = "SP FILTER",
  .parent   = &s_operation_mode_group,
  .child    = 0,
  .sibling  = &s_operation_sync,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_sync =
{
  .id       = MENU_ID_OPERATION_SYNC,
  .title    = "SYNC",
  .parent   = &s_operation_mode_group,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_route =
{
  .id       = MENU_ID_OPERATION_ROUTE,
  .title    = "ROUTE",
  .parent   = &s_operation_ch1_group,
  .child    = 0,
  .sibling  = &s_operation_access,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_access =
{
  .id       = MENU_ID_OPERATION_ACCESS,
  .title    = "ACCESS",
  .parent   = &s_operation_ch1_group,
  .child    = 0,
  .sibling  = &s_operation_hopmode,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_hopmode =
{
  .id       = MENU_ID_OPERATION_HOPMODE,
  .title    = "HOP MODE",
  .parent   = &s_operation_ch1_group,
  .child    = 0,
  .sibling  = &s_operation_workfreq_group,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_freq =
{
  .id       = MENU_ID_OPERATION_CH_FREQ,
  .title    = "CENTER",
  .parent   = &s_operation_workfreq_group,
  .child    = 0,
  .sibling  = &s_operation_freq1,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_freq1 =
{
  .id       = MENU_ID_OPERATION_FREQ1,
  .title    = "CENTER F1",
  .parent   = &s_operation_workfreq_group,
  .child    = 0,
  .sibling  = &s_operation_freq2,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_freq2 =
{
  .id       = MENU_ID_OPERATION_FREQ2,
  .title    = "CENTER F2",
  .parent   = &s_operation_workfreq_group,
  .child    = 0,
  .sibling  = &s_operation_freq3,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_freq3 =
{
  .id       = MENU_ID_OPERATION_FREQ3,
  .title    = "CENTER F3",
  .parent   = &s_operation_workfreq_group,
  .child    = 0,
  .sibling  = &s_operation_freq4,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_freq4 =
{
  .id       = MENU_ID_OPERATION_FREQ4,
  .title    = "CENTER F4",
  .parent   = &s_operation_workfreq_group,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_bw =
{
  .id       = MENU_ID_OPERATION_BW,
  .title    = "BW",
  .parent   = &s_operation_ch1_group,
  .child    = 0,
  .sibling  = &s_operation_mcs,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_mcs =
{
  .id       = MENU_ID_OPERATION_MCS,
  .title    = "MOD TYPE",
  .parent   = &s_operation_ch1_group,
  .child    = 0,
  .sibling  = &s_operation_txpower,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_txpower =
{
  .id       = MENU_ID_OPERATION_TX_POWER,
  .title    = "TX POWER",
  .parent   = &s_operation_ch1_group,
  .child    = 0,
  .sibling  = &s_operation_txatten,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_txatten =
{
  .id       = MENU_ID_OPERATION_TX_ATTEN,
  .title    = "TX ATTEN",
  .parent   = &s_operation_ch1_group,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_operation_workfreq_group =
{
  .id       = MENU_ID_OPERATION_WORKFREQ_GROUP,
  .title    = "WORK FREQ",
  .parent   = &s_operation_ch1_group,
  .child    = &s_operation_freq,
  .sibling  = &s_operation_bw,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_operation_ch1_group =
{
  .id       = MENU_ID_OPERATION_CH1_GROUP,
  .title    = "CH1 SET",
  .parent   = &s_operation_param_group,
  .child    = &s_operation_route,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_operation_param_group =
{
  .id       = MENU_ID_OPERATION_PARAM_GROUP,
  .title    = "PARAM SET",
  .parent   = &s_operation,
  .child    = &s_operation_ch1_group,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_operation_mode_group =
{
  .id       = MENU_ID_OPERATION_MODE_GROUP,
  .title    = "MODE SET",
  .parent   = &s_operation,
  .child    = &s_operation_mode,
  .sibling  = &s_operation_param_group,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_operation =
{
  .id       = MENU_ID_OPERATION,
  .title    = "OPERATION",
  .parent   = &s_root,
  .child    = &s_operation_mode_group,
  .sibling  = &s_function,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_member_count =
{
  .id       = MENU_ID_FUNCTION_MEMBER_COUNT,
  .title    = "ONLINE CNT",
  .parent   = &s_function_member,
  .child    = 0,
  .sibling  = &s_function_member_node,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_member_node_id =
{
  .id       = MENU_ID_FUNCTION_MEMBER_NODE_ID,
  .title    = "MEM ID",
  .parent   = &s_function_member_node,
  .child    = 0,
  .sibling  = &s_function_member_node_ip,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_member_node_ip =
{
  .id       = MENU_ID_FUNCTION_MEMBER_NODE_IP,
  .title    = "IP",
  .parent   = &s_function_member_node,
  .child    = 0,
  .sibling  = &s_function_member_node_hops,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_member_node_hops =
{
  .id       = MENU_ID_FUNCTION_MEMBER_NODE_HOPS,
  .title    = "HOPS",
  .parent   = &s_function_member_node,
  .child    = 0,
  .sibling  = &s_function_member_node_rssi,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_member_node_rssi =
{
  .id       = MENU_ID_FUNCTION_MEMBER_NODE_RSSI,
  .title    = "RSSI",
  .parent   = &s_function_member_node,
  .child    = 0,
  .sibling  = &s_function_member_node_delay,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_member_node_delay =
{
  .id       = MENU_ID_FUNCTION_MEMBER_NODE_DELAY,
  .title    = "DELAY",
  .parent   = &s_function_member_node,
  .child    = 0,
  .sibling  = &s_function_member_pos,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_member_pos_lon =
{
  .id       = MENU_ID_FUNCTION_MEMBER_POS_LON,
  .title    = "LON",
  .parent   = &s_function_member_pos,
  .child    = 0,
  .sibling  = &s_function_member_pos_lat,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_member_pos_lat =
{
  .id       = MENU_ID_FUNCTION_MEMBER_POS_LAT,
  .title    = "LAT",
  .parent   = &s_function_member_pos,
  .child    = 0,
  .sibling  = &s_function_member_pos_alt,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_member_pos_alt =
{
  .id       = MENU_ID_FUNCTION_MEMBER_POS_ALT,
  .title    = "ALT",
  .parent   = &s_function_member_pos,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_member_pos =
{
  .id       = MENU_ID_FUNCTION_MEMBER_POS,
  .title    = "POS",
  .parent   = &s_function_member_node,
  .child    = &s_function_member_pos_lon,
  .sibling  = &s_function_member_detail,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_member_node =
{
  .id       = MENU_ID_FUNCTION_MEMBER_NODE,
  .title    = "NODE LIST",
  .parent   = &s_function_member,
  .child    = &s_function_member_node_id,
  .sibling  = 0,
  .render_cb = Render_MemberNodeList,
  .key_cb    = MenuKeyHandler_MemberNodeList
};

static const MenuNode_t s_function_detail_id =
{
  .id       = MENU_ID_FUNCTION_DETAIL_ID,
  .title    = "MEM ID",
  .parent   = &s_function_member_detail,
  .child    = 0,
  .sibling  = &s_function_detail_filter,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_detail_filter =
{
  .id       = MENU_ID_FUNCTION_DETAIL_FILTER,
  .title    = "SP FILTER",
  .parent   = &s_function_member_detail,
  .child    = 0,
  .sibling  = &s_function_detail_ip,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_detail_ip =
{
  .id       = MENU_ID_FUNCTION_DETAIL_IP,
  .title    = "IP",
  .parent   = &s_function_member_detail,
  .child    = 0,
  .sibling  = &s_function_detail_ch1,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_detail_ch1_hop =
{
  .id       = MENU_ID_FUNCTION_DETAIL_CH1_HOP,
  .title    = "HOP MODE",
  .parent   = &s_function_detail_ch1,
  .child    = 0,
  .sibling  = &s_function_detail_ch1_freq,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_detail_ch1_freq =
{
  .id       = MENU_ID_FUNCTION_DETAIL_CH1_FREQ,
  .title    = "WORK FREQ",
  .parent   = &s_function_detail_ch1,
  .child    = 0,
  .sibling  = &s_function_detail_ch1_bw,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_detail_ch1_bw =
{
  .id       = MENU_ID_FUNCTION_DETAIL_CH1_BW,
  .title    = "BW",
  .parent   = &s_function_detail_ch1,
  .child    = 0,
  .sibling  = &s_function_detail_ch1_wave,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_detail_ch1_wave =
{
  .id       = MENU_ID_FUNCTION_DETAIL_CH1_WAVE,
  .title    = "WAVE",
  .parent   = &s_function_detail_ch1,
  .child    = 0,
  .sibling  = &s_function_detail_ch1_txpwr,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_detail_ch1_txpwr =
{
  .id       = MENU_ID_FUNCTION_DETAIL_CH1_TXPWR,
  .title    = "TX PWR",
  .parent   = &s_function_detail_ch1,
  .child    = 0,
  .sibling  = &s_function_detail_ch1_txatten,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_detail_ch1_txatten =
{
  .id       = MENU_ID_FUNCTION_DETAIL_CH1_TXATTEN,
  .title    = "TX ATT",
  .parent   = &s_function_detail_ch1,
  .child    = 0,
  .sibling  = &s_function_detail_ch1_route,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_detail_ch1_route =
{
  .id       = MENU_ID_FUNCTION_DETAIL_CH1_ROUTE,
  .title    = "ROUTE",
  .parent   = &s_function_detail_ch1,
  .child    = 0,
  .sibling  = &s_function_detail_ch1_access,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_detail_ch1_access =
{
  .id       = MENU_ID_FUNCTION_DETAIL_CH1_ACCESS,
  .title    = "ACCESS",
  .parent   = &s_function_detail_ch1,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_detail_ch1 =
{
  .id       = MENU_ID_FUNCTION_DETAIL_CH1,
  .title    = "CH1 ST",
  .parent   = &s_function_member_detail,
  .child    = &s_function_detail_ch1_hop,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_member_detail =
{
  .id       = MENU_ID_FUNCTION_MEMBER_DETAIL,
  .title    = "NODE DETAIL N",
  .parent   = &s_function_member_node,
  .child    = &s_function_detail_id,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_DetailRequest
};

static const MenuNode_t s_function_member =
{
  .id       = MENU_ID_FUNCTION_MEMBER,
  .title    = "MEMBER ST",
  .parent   = &s_function,
  .child    = &s_function_member_count,
  .sibling  = &s_function_traffic,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_total_tx =
{
  .id       = MENU_ID_FUNCTION_TOTAL_TX,
  .title    = "TOTAL TX",
  .parent   = &s_function_traffic,
  .child    = 0,
  .sibling  = &s_function_total_rx,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_total_rx =
{
  .id       = MENU_ID_FUNCTION_TOTAL_RX,
  .title    = "TOTAL RX",
  .parent   = &s_function_traffic,
  .child    = 0,
  .sibling  = &s_function_voice,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_voice_tx =
{
  .id       = MENU_ID_FUNCTION_VOICE_TX,
  .title    = "VOICE TX",
  .parent   = &s_function_voice,
  .child    = 0,
  .sibling  = &s_function_voice_rx,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_voice_rx =
{
  .id       = MENU_ID_FUNCTION_VOICE_RX,
  .title    = "VOICE RX",
  .parent   = &s_function_voice,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_voice =
{
  .id       = MENU_ID_FUNCTION_VOICE,
  .title    = "VOICE",
  .parent   = &s_function_traffic,
  .child    = &s_function_voice_tx,
  .sibling  = &s_function_eth,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_eth_tx =
{
  .id       = MENU_ID_FUNCTION_ETH_TX,
  .title    = "ETH TX",
  .parent   = &s_function_eth,
  .child    = 0,
  .sibling  = &s_function_eth_rx,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_eth_rx =
{
  .id       = MENU_ID_FUNCTION_ETH_RX,
  .title    = "ETH RX",
  .parent   = &s_function_eth,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_eth =
{
  .id       = MENU_ID_FUNCTION_ETH,
  .title    = "ETH MSG",
  .parent   = &s_function_traffic,
  .child    = &s_function_eth_tx,
  .sibling  = &s_function_traffic_op,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_traffic_op =
{
  .id       = MENU_ID_FUNCTION_TRAFFIC_OP,
  .title    = "STAT OP",
  .parent   = &s_function_traffic,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_TrafficOp
};

static const MenuNode_t s_function_traffic =
{
  .id       = MENU_ID_FUNCTION_TRAFFIC,
  .title    = "TRAFFIC",
  .parent   = &s_function,
  .child    = &s_function_total_tx,
  .sibling  = &s_function_selftest,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_self_state =
{
  .id       = MENU_ID_FUNCTION_SELF_STATE,
  .title    = "SELF ST",
  .parent   = &s_function_selftest,
  .child    = 0,
  .sibling  = &s_function_sysinfo,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_sys_temp =
{
  .id       = MENU_ID_FUNCTION_SYS_TEMP,
  .title    = "TEMP",
  .parent   = &s_function_sysinfo,
  .child    = 0,
  .sibling  = &s_function_sys_fan,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_sys_fan =
{
  .id       = MENU_ID_FUNCTION_SYS_FAN,
  .title    = "FAN",
  .parent   = &s_function_sysinfo,
  .child    = 0,
  .sibling  = &s_function_sys_sat,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_sys_sat =
{
  .id       = MENU_ID_FUNCTION_SYS_SAT,
  .title    = "SAT LOCK",
  .parent   = &s_function_sysinfo,
  .child    = 0,
  .sibling  = &s_function_sys_clock,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_sys_clock =
{
  .id       = MENU_ID_FUNCTION_SYS_CLOCK,
  .title    = "CLK SEL",
  .parent   = &s_function_sysinfo,
  .child    = 0,
  .sibling  = &s_function_sys_adc,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_sys_adc =
{
  .id       = MENU_ID_FUNCTION_SYS_ADC,
  .title    = "ADC ST",
  .parent   = &s_function_sysinfo,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_sysinfo =
{
  .id       = MENU_ID_FUNCTION_SYSINFO,
  .title    = "SYS INFO",
  .parent   = &s_function_selftest,
  .child    = &s_function_sys_temp,
  .sibling  = &s_function_clk_src,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_clk_temp =
{
  .id       = MENU_ID_FUNCTION_CLK_TEMP,
  .title    = "SRC TEMP",
  .parent   = &s_function_clk_src,
  .child    = 0,
  .sibling  = &s_function_clk_cnt,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_clk_cnt =
{
  .id       = MENU_ID_FUNCTION_CLK_CNT,
  .title    = "FREQ CNT",
  .parent   = &s_function_clk_src,
  .child    = 0,
  .sibling  = &s_function_clk_sense,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_clk_sense =
{
  .id       = MENU_ID_FUNCTION_CLK_SENSE,
  .title    = "SENSE ST",
  .parent   = &s_function_clk_src,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_clk_src =
{
  .id       = MENU_ID_FUNCTION_CLK_SRC,
  .title    = "CLK SRC",
  .parent   = &s_function_selftest,
  .child    = &s_function_clk_temp,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function_selftest =
{
  .id       = MENU_ID_FUNCTION_SELFTEST,
  .title    = "SELF TEST",
  .parent   = &s_function,
  .child    = &s_function_self_state,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_function =
{
  .id       = MENU_ID_FUNCTION,
  .title    = "FUNCTION",
  .parent   = &s_root,
  .child    = &s_function_member,
  .sibling  = &s_setting,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_setting_device_id =
{
  .id       = MENU_ID_SETTING_DEVICE_ID,
  .title    = "DEVICE ID",
  .parent   = &s_setting,
  .child    = 0,
  .sibling  = &s_setting_device_name,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_setting_device_name =
{
  .id       = MENU_ID_SETTING_DEVICE_NAME,
  .title    = "DEVICE NAME",
  .parent   = &s_setting,
  .child    = 0,
  .sibling  = &s_setting_net,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_setting_net_biz_ip =
{
  .id       = MENU_ID_SETTING_NET_BIZ_IP,
  .title    = "IP",
  .parent   = &s_setting_net_biz,
  .child    = 0,
  .sibling  = &s_setting_net_biz_port,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_setting_net_biz_port =
{
  .id       = MENU_ID_SETTING_NET_BIZ_PORT,
  .title    = "PORT",
  .parent   = &s_setting_net_biz,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_setting_net_biz =
{
  .id       = MENU_ID_SETTING_NET_BIZ,
  .title    = "BIZ NET",
  .parent   = &s_setting_net,
  .child    = &s_setting_net_biz_ip,
  .sibling  = &s_setting_net_mgmt,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_setting_net_mgmt_ip =
{
  .id       = MENU_ID_SETTING_NET_MGMT_IP,
  .title    = "IP",
  .parent   = &s_setting_net_mgmt,
  .child    = 0,
  .sibling  = &s_setting_net_mgmt_port,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_setting_net_mgmt_port =
{
  .id       = MENU_ID_SETTING_NET_MGMT_PORT,
  .title    = "PORT",
  .parent   = &s_setting_net_mgmt,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_setting_net_mgmt =
{
  .id       = MENU_ID_SETTING_NET_MGMT,
  .title    = "MGMT NET",
  .parent   = &s_setting_net,
  .child    = &s_setting_net_mgmt_ip,
  .sibling  = &s_setting_net_sense,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_setting_net_sense_ip =
{
  .id       = MENU_ID_SETTING_NET_SENSE_IP,
  .title    = "IP",
  .parent   = &s_setting_net_sense,
  .child    = 0,
  .sibling  = &s_setting_net_sense_port,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_setting_net_sense_port =
{
  .id       = MENU_ID_SETTING_NET_SENSE_PORT,
  .title    = "PORT",
  .parent   = &s_setting_net_sense,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_setting_net_sense =
{
  .id       = MENU_ID_SETTING_NET_SENSE,
  .title    = "SENSE NET",
  .parent   = &s_setting_net,
  .child    = &s_setting_net_sense_ip,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_setting_net =
{
  .id       = MENU_ID_SETTING_NET,
  .title    = "NET",
  .parent   = &s_setting,
  .child    = &s_setting_net_biz,
  .sibling  = &s_setting_uart,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_setting_uart_databits =
{
  .id       = MENU_ID_SETTING_UART_DATABITS,
  .title    = "DATABITS",
  .parent   = &s_setting_uart,
  .child    = 0,
  .sibling  = &s_setting_uart_stopbits,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_UartFormat
};

static const MenuNode_t s_setting_uart_stopbits =
{
  .id       = MENU_ID_SETTING_UART_STOPBITS,
  .title    = "STOPBITS",
  .parent   = &s_setting_uart,
  .child    = 0,
  .sibling  = &s_setting_uart_parity,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_UartFormat
};

static const MenuNode_t s_setting_uart_parity =
{
  .id       = MENU_ID_SETTING_UART_PARITY,
  .title    = "PARITY",
  .parent   = &s_setting_uart,
  .child    = 0,
  .sibling  = &s_setting_uart_flow,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_UartFormat
};

static const MenuNode_t s_setting_uart_flow =
{
  .id       = MENU_ID_SETTING_UART_FLOW,
  .title    = "FLOW CTRL",
  .parent   = &s_setting_uart,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_UartFormat
};

static const MenuNode_t s_setting_uart =
{
  .id       = MENU_ID_SETTING_UART,
  .title    = "UART",
  .parent   = &s_setting,
  .child    = &s_setting_uart_baud,
  .sibling  = &s_setting_location,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_setting_uart_baud =
{
  .id       = MENU_ID_SETTING_UART_BAUD,
  .title    = "BAUD",
  .parent   = &s_setting_uart,
  .child    = 0,
  .sibling  = &s_setting_uart_databits,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_UartBaud
};

static const MenuNode_t s_setting_loc_auto =
{
  .id       = MENU_ID_SETTING_LOC_AUTO,
  .title    = "AUTO",
  .parent   = &s_setting_location,
  .child    = 0,
  .sibling  = &s_setting_loc_lon,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_setting_loc_lon =
{
  .id       = MENU_ID_SETTING_LOC_LON,
  .title    = "LON",
  .parent   = &s_setting_location,
  .child    = 0,
  .sibling  = &s_setting_loc_lat,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_setting_loc_lat =
{
  .id       = MENU_ID_SETTING_LOC_LAT,
  .title    = "LAT",
  .parent   = &s_setting_location,
  .child    = 0,
  .sibling  = &s_setting_loc_alt,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_setting_loc_alt =
{
  .id       = MENU_ID_SETTING_LOC_ALT,
  .title    = "ALT",
  .parent   = &s_setting_location,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_setting_location =
{
  .id       = MENU_ID_SETTING_LOCATION,
  .title    = "LOCATION",
  .parent   = &s_setting,
  .child    = &s_setting_loc_auto,
  .sibling  = &s_setting_time,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_setting_time_auto =
{
  .id       = MENU_ID_SETTING_TIME_AUTO,
  .title    = "AUTO",
  .parent   = &s_setting_time,
  .child    = 0,
  .sibling  = &s_setting_time_manual,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_OperationItem
};

static const MenuNode_t s_setting_time_hour =
{
  .id       = MENU_ID_SETTING_TIME_HOUR,
  .title    = "HOUR",
  .parent   = &s_setting_time_manual,
  .child    = 0,
  .sibling  = &s_setting_time_min,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_TimeManual
};

static const MenuNode_t s_setting_time_min =
{
  .id       = MENU_ID_SETTING_TIME_MIN,
  .title    = "MIN",
  .parent   = &s_setting_time_manual,
  .child    = 0,
  .sibling  = &s_setting_time_sec,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_TimeManual
};

static const MenuNode_t s_setting_time_sec =
{
  .id       = MENU_ID_SETTING_TIME_SEC,
  .title    = "SEC",
  .parent   = &s_setting_time_manual,
  .child    = 0,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = MenuKeyHandler_TimeManual
};

static const MenuNode_t s_setting_time_manual =
{
  .id       = MENU_ID_SETTING_TIME_MANUAL,
  .title    = "MANUAL",
  .parent   = &s_setting_time,
  .child    = &s_setting_time_hour,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_setting_time =
{
  .id       = MENU_ID_SETTING_TIME,
  .title    = "TIME",
  .parent   = &s_setting,
  .child    = &s_setting_time_auto,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
  .key_cb    = 0
};

static const MenuNode_t s_setting =
{
  .id       = MENU_ID_SETTING,
  .title    = "SETTING",
  .parent   = &s_root,
  .child    = &s_setting_device_id,
  .sibling  = 0,
  .render_cb = Render_OperationTree,
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

static bool Menu_IsWorkFreqItem(const MenuNode_t *node)
{
  if (node == 0)
  {
    return false;
  }

  switch (node->id)
  {
    case MENU_ID_OPERATION_CH_FREQ:
    case MENU_ID_OPERATION_FREQ1:
    case MENU_ID_OPERATION_FREQ2:
    case MENU_ID_OPERATION_FREQ3:
    case MENU_ID_OPERATION_FREQ4:
      return true;

    default:
      return false;
  }
}

static bool Menu_IsNodeVisible(const MenuNode_t *node)
{
  int32_t hopMode = 0;

  if (node == 0)
  {
    return false;
  }

  if (!Menu_IsWorkFreqItem(node))
  {
    return true;
  }

  (void)APP_ParamDict_GetValue(PARAM_ID_FREQ_HOP_MODE, &hopMode);

  if (node->id == MENU_ID_OPERATION_CH_FREQ)
  {
    return (hopMode == 0);
  }

  return (hopMode != 0);
}

static const MenuNode_t *Menu_FirstVisibleChild(const MenuNode_t *parent)
{
  const MenuNode_t *child;

  if ((parent == 0) || (parent->child == 0))
  {
    return 0;
  }

  child = parent->child;
  while ((child != 0) && !Menu_IsNodeVisible(child))
  {
    child = child->sibling;
  }

  return child;
}

static void Menu_EnsureCurrentVisible(void)
{
  const MenuNode_t *visible;

  if (s_current == 0)
  {
    return;
  }

  if (Menu_IsNodeVisible(s_current))
  {
    return;
  }

  if ((s_current->parent == 0) || (s_current->parent->child == 0))
  {
    return;
  }

  visible = Menu_FirstVisibleChild(s_current->parent);
  if (visible != 0)
  {
    s_current = visible;
  }
}

/* -------------------- 對外接口實現 -------------------- */

const MenuNode_t *APP_Menu_Init(void)
{
  /* 上电进入 MAIN 根菜单。 */
  s_current = &s_root;
  s_memberNodeSelect = 1u;
  s_paramEditing = false;
  s_paramInputDigits = 0u;
  s_adaptFreqEditMask = 0u;
  s_adaptHopProtectActive = false;
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

void APP_Menu_ReturnToRoot(void)
{
  s_current = &s_root;
  s_memberNodeSelect = 1u;
  s_paramEditing = false;
  s_paramInputDigits = 0u;
  s_adaptFreqEditMask = 0u;
  s_adaptHopProtectActive = false;
  if (s_current->render_cb != 0)
  {
    s_current->render_cb(s_current);
  }
}

uint8_t APP_Menu_IsAdaptHopProtectActive(void)
{
  return (uint8_t)(s_adaptHopProtectActive ? 1u : 0u);
}

void APP_Menu_ClearAdaptHopProtect(void)
{
  s_adaptHopProtectActive = false;
}

void APP_Menu_RefreshCurrent(void)
{
  Menu_EnsureCurrentVisible();

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

  Menu_EnsureCurrentVisible();

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
          if (Menu_IsNodeVisible(node))
          {
            prev = node;
          }
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
        const MenuNode_t *next = s_current->sibling;
        while ((next != 0) && !Menu_IsNodeVisible(next))
        {
          next = next->sibling;
        }
        if (next != 0)
        {
          s_current = next;
        }
      }
      break;

    case MENU_KEY_OK:
      if (s_current->child != 0)
      {
        const MenuNode_t *child = Menu_FirstVisibleChild(s_current);
        if (child != 0)
        {
          s_current = child;
        }
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

  /* 行 0 显示当前菜单标题 + 串口收包计数（临时调试） */
  Menu_PrintHeaderPath(node);

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

  if (node->id == MENU_ID_ROOT)
  {
    int32_t hour = 0, minute = 0, second = 0;
    int32_t netJoin = 0;
    int32_t waveform = 0, bandwidth = 0, hopMode = 0;
    int32_t fixedFreq = 0, f1 = 0, f2 = 0, f3 = 0, f4 = 0;
    int32_t spatialFilter = 0, syncMode = 0;
    int32_t txPower = 0, txAtten = 0;

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

    Menu_PrintHeaderPath(node);

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "TIME %02ld:%02ld:%02ld NET %s",
                     (long)hour, (long)minute, (long)second, Menu_NetStateText(netJoin));
      BSP_Lcd_PrintLine(1, buf);
    }

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "WF %ld BW %s H %s",
                     (long)waveform, Menu_BandwidthText(bandwidth), Menu_HopModeText(hopMode));
      BSP_Lcd_PrintLine(2, buf);
    }

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "FIX %ld", (long)(fixedFreq / 1000));
      BSP_Lcd_PrintLine(3, buf);
    }

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "AF1 %ld AF2 %ld", (long)(f1 / 1000), (long)(f2 / 1000));
      BSP_Lcd_PrintLine(4, buf);
    }

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "AF3 %ld AF4 %ld", (long)(f3 / 1000), (long)(f4 / 1000));
      BSP_Lcd_PrintLine(5, buf);
    }

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "FIL %s SYNC %s",
                     Menu_OnOffText(spatialFilter), Menu_SyncText(syncMode));
      BSP_Lcd_PrintLine(6, buf);
    }

    {
      char buf[32];
      (void)snprintf(buf, sizeof(buf), "PWR %s A%ld",
                     Menu_TxPowerText(txPower), (long)txAtten);
      BSP_Lcd_PrintLine(7, buf);
    }
    return;
  }

}

static bool Menu_GetOperationConfig(const MenuNode_t *node,
                                    ParamId_t *id,
                                    int32_t *minVal,
                                    int32_t *maxVal,
                                    bool *isFreq)
{
  if ((node == 0) || (id == 0) || (minVal == 0) || (maxVal == 0) || (isFreq == 0))
  {
    return false;
  }

  switch (node->id)
  {
    case MENU_ID_OPERATION_MODE:
      *id = PARAM_ID_WORK_MODE;
      *minVal = 0;
      *maxVal = 1;
      *isFreq = false;
      return true;

    case MENU_ID_OPERATION_SPATIAL:
      *id = PARAM_ID_SPATIAL_FILTER;
      *minVal = 0;
      *maxVal = 1;
      *isFreq = false;
      return true;

    case MENU_ID_OPERATION_SYNC:
      *id = PARAM_ID_SYNC_MODE;
      *minVal = 0;
      *maxVal = 1;
      *isFreq = false;
      return true;

    case MENU_ID_OPERATION_ROUTE:
      *id = PARAM_ID_ROUTING_PROTOCOL;
      *minVal = 0;
      *maxVal = 2;
      *isFreq = false;
      return true;

    case MENU_ID_OPERATION_ACCESS:
      *id = PARAM_ID_ACCESS_PROTOCOL;
      *minVal = 0;
      *maxVal = 0;
      *isFreq = false;
      return true;

    case MENU_ID_OPERATION_HOPMODE:
      *id = PARAM_ID_FREQ_HOP_MODE;
      *minVal = 0;
      *maxVal = 1;
      *isFreq = false;
      return true;

    case MENU_ID_OPERATION_CH_FREQ:
      *id = PARAM_ID_FIXED_FREQ;
      *minVal = 225;
      *maxVal = 2500;
      *isFreq = true;
      return true;

    case MENU_ID_OPERATION_FREQ1:
      *id = PARAM_ID_ADAPT_FREQ1;
      *minVal = 225;
      *maxVal = 2500;
      *isFreq = true;
      return true;

    case MENU_ID_OPERATION_FREQ2:
      *id = PARAM_ID_ADAPT_FREQ2;
      *minVal = 225;
      *maxVal = 2500;
      *isFreq = true;
      return true;

    case MENU_ID_OPERATION_FREQ3:
      *id = PARAM_ID_ADAPT_FREQ3;
      *minVal = 225;
      *maxVal = 2500;
      *isFreq = true;
      return true;

    case MENU_ID_OPERATION_FREQ4:
      *id = PARAM_ID_ADAPT_FREQ4;
      *minVal = 225;
      *maxVal = 2500;
      *isFreq = true;
      return true;

    case MENU_ID_OPERATION_BW:
      *id = PARAM_ID_SIGNAL_BANDWIDTH;
      *minVal = 0;
      *maxVal = 3;
      *isFreq = false;
      return true;

    case MENU_ID_OPERATION_MCS:
      *id = PARAM_ID_WAVEFORM_GEAR;
      *minVal = 0;
      *maxVal = 7;
      *isFreq = false;
      return true;

    case MENU_ID_OPERATION_TX_POWER:
      *id = PARAM_ID_TX_POWER;
      *minVal = 0;
      *maxVal = 2;
      *isFreq = false;
      return true;

    case MENU_ID_OPERATION_TX_ATTEN:
      *id = PARAM_ID_TX_POWER_ATTEN;
      *minVal = 0;
      *maxVal = 90;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_DEVICE_NAME:
      *id = PARAM_ID_DEVICE_NAME_TOKEN;
      *minVal = 0;
      *maxVal = 999999;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_NET_BIZ_IP:
      *id = PARAM_ID_NET_IP_ADDR;
      *minVal = (-2147483647 - 1);
      *maxVal = 2147483647;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_NET_BIZ_PORT:
      *id = PARAM_ID_NET_BIZ_PORT;
      *minVal = 0;
      *maxVal = 65535;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_NET_MGMT_IP:
      *id = PARAM_ID_NET_MGMT_IP;
      *minVal = (-2147483647 - 1);
      *maxVal = 2147483647;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_NET_MGMT_PORT:
      *id = PARAM_ID_NET_MGMT_PORT;
      *minVal = 0;
      *maxVal = 65535;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_NET_SENSE_IP:
      *id = PARAM_ID_NET_SENSE_IP;
      *minVal = (-2147483647 - 1);
      *maxVal = 2147483647;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_NET_SENSE_PORT:
      *id = PARAM_ID_NET_SENSE_PORT;
      *minVal = 0;
      *maxVal = 65535;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_LOC_AUTO:
      *id = PARAM_ID_LOC_AUTO_SYNC;
      *minVal = 0;
      *maxVal = 1;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_LOC_LON:
      *id = PARAM_ID_GPS_LONGITUDE;
      *minVal = (-2147483647 - 1);
      *maxVal = 2147483647;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_LOC_LAT:
      *id = PARAM_ID_GPS_LATITUDE;
      *minVal = (-2147483647 - 1);
      *maxVal = 2147483647;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_LOC_ALT:
      *id = PARAM_ID_GPS_ALTITUDE;
      *minVal = 0;
      *maxVal = 65535;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_TIME_AUTO:
      *id = PARAM_ID_TIME_AUTO_SYNC;
      *minVal = 0;
      *maxVal = 1;
      *isFreq = false;
      return true;

    default:
      return false;
  }
}

static bool Menu_GetDisplayConfig(const MenuNode_t *node,
                                  ParamId_t *id,
                                  bool *isFreq)
{
  int32_t minVal = 0;
  int32_t maxVal = 0;

  if ((node == 0) || (id == 0) || (isFreq == 0))
  {
    return false;
  }

  if (Menu_GetOperationConfig(node, id, &minVal, &maxVal, isFreq))
  {
    return true;
  }

  switch (node->id)
  {
    case MENU_ID_FUNCTION_MEMBER_COUNT:
      *id = PARAM_ID_NEIGHBOR_COUNT;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_MEMBER_NODE_RSSI:
      *id = PARAM_ID_NEIGHBOR_RSSI;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_MEMBER_NODE_ID:
      *id = PARAM_ID_NEIGHBOR_NODE_ID;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_MEMBER_NODE_IP:
      *id = PARAM_ID_NEIGHBOR_NODE_IP;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_MEMBER_NODE_HOPS:
      *id = PARAM_ID_NEIGHBOR_NODE_HOPS;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_MEMBER_NODE_DELAY:
      *id = PARAM_ID_NEIGHBOR_NODE_DELAY;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_MEMBER_POS_LON:
      *id = PARAM_ID_NEIGHBOR_NODE_LON;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_MEMBER_POS_LAT:
      *id = PARAM_ID_NEIGHBOR_NODE_LAT;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_MEMBER_POS_ALT:
      *id = PARAM_ID_NEIGHBOR_NODE_ALT;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_DETAIL_FILTER:
      *id = PARAM_ID_DETAIL_SPATIAL;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_DETAIL_ID:
      *id = PARAM_ID_DETAIL_MEMBER_ID;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_DETAIL_IP:
      *id = PARAM_ID_DETAIL_IP;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_DETAIL_CH1_HOP:
      *id = PARAM_ID_DETAIL_CH1_HOP;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_DETAIL_CH1_FREQ:
      *id = PARAM_ID_DETAIL_CH1_FREQ;
      *isFreq = true;
      return true;

    case MENU_ID_FUNCTION_DETAIL_CH1_BW:
      *id = PARAM_ID_DETAIL_CH1_BW;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_DETAIL_CH1_WAVE:
      *id = PARAM_ID_DETAIL_CH1_WAVE;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_DETAIL_CH1_TXPWR:
      *id = PARAM_ID_DETAIL_CH1_TXPWR;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_DETAIL_CH1_TXATTEN:
      *id = PARAM_ID_DETAIL_CH1_TXATTEN;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_DETAIL_CH1_ROUTE:
      *id = PARAM_ID_DETAIL_CH1_ROUTE;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_DETAIL_CH1_ACCESS:
      *id = PARAM_ID_DETAIL_CH1_ACCESS;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_VOICE_TX:
      *id = PARAM_ID_VOICE_TX_CNT;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_VOICE_RX:
      *id = PARAM_ID_VOICE_RX_CNT;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_ETH_TX:
      *id = PARAM_ID_ETH_TX_CNT;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_ETH_RX:
      *id = PARAM_ID_ETH_RX_CNT;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_TOTAL_TX:
      *id = PARAM_ID_ETH_TX_CNT;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_TOTAL_RX:
      *id = PARAM_ID_ETH_RX_CNT;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_TRAFFIC_OP:
      *id = PARAM_ID_TXRX_OPERATION;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_SYS_TEMP:
      *id = PARAM_ID_ENV_TEMPERATURE;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_SELF_STATE:
      *id = PARAM_ID_SELFTEST_STATE;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_SYS_FAN:
      *id = PARAM_ID_FAN_STATE;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_SYS_SAT:
      *id = PARAM_ID_SAT_LOCK;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_SYS_CLOCK:
      *id = PARAM_ID_CLOCK_SELECTION;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_SYS_ADC:
      *id = PARAM_ID_ADC_STATUS;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_CLK_TEMP:
      *id = PARAM_ID_CLOCK_SRC_TEMP;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_CLK_CNT:
      *id = PARAM_ID_FREQ_WORD_CNT;
      *isFreq = false;
      return true;

    case MENU_ID_FUNCTION_CLK_SENSE:
      *id = PARAM_ID_COMM_SENSE_ST;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_DEVICE_ID:
      *id = PARAM_ID_DEVICE_ID;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_DEVICE_NAME:
      *id = PARAM_ID_DEVICE_NAME_TOKEN;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_NET_BIZ_IP:
      *id = PARAM_ID_NET_IP_ADDR;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_NET_BIZ_PORT:
      *id = PARAM_ID_NET_BIZ_PORT;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_NET_MGMT_IP:
      *id = PARAM_ID_NET_MGMT_IP;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_NET_MGMT_PORT:
      *id = PARAM_ID_NET_MGMT_PORT;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_NET_SENSE_IP:
      *id = PARAM_ID_NET_SENSE_IP;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_NET_SENSE_PORT:
      *id = PARAM_ID_NET_SENSE_PORT;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_UART_BAUD:
      *id = PARAM_ID_UART_BAUDRATE;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_UART_DATABITS:
      *id = PARAM_ID_UART_DATABITS;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_UART_STOPBITS:
      *id = PARAM_ID_UART_STOPBITS;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_UART_PARITY:
      *id = PARAM_ID_UART_PARITY;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_UART_FLOW:
      *id = PARAM_ID_UART_FLOW;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_LOC_AUTO:
      *id = PARAM_ID_LOC_AUTO_SYNC;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_LOC_LON:
      *id = PARAM_ID_GPS_LONGITUDE;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_LOC_LAT:
      *id = PARAM_ID_GPS_LATITUDE;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_LOC_ALT:
      *id = PARAM_ID_GPS_ALTITUDE;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_TIME_HOUR:
      *id = PARAM_ID_TIME_HOUR;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_TIME_AUTO:
      *id = PARAM_ID_TIME_AUTO_SYNC;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_TIME_MIN:
      *id = PARAM_ID_TIME_MINUTE;
      *isFreq = false;
      return true;

    case MENU_ID_SETTING_TIME_SEC:
      *id = PARAM_ID_TIME_SECOND;
      *isFreq = false;
      return true;

    default:
      return false;
  }
}

static bool Menu_GetManualTimeConfig(const MenuNode_t *node,
                                     ParamId_t *id,
                                     int32_t *minVal,
                                     int32_t *maxVal)
{
  if ((node == 0) || (id == 0) || (minVal == 0) || (maxVal == 0))
  {
    return false;
  }

  switch (node->id)
  {
    case MENU_ID_SETTING_TIME_HOUR:
      *id = PARAM_ID_TIME_HOUR;
      *minVal = 0;
      *maxVal = 23;
      return true;

    case MENU_ID_SETTING_TIME_MIN:
      *id = PARAM_ID_TIME_MINUTE;
      *minVal = 0;
      *maxVal = 59;
      return true;

    case MENU_ID_SETTING_TIME_SEC:
      *id = PARAM_ID_TIME_SECOND;
      *minVal = 0;
      *maxVal = 59;
      return true;

    default:
      return false;
  }
}

static void Menu_FormatOperationValue(const MenuNode_t *node, bool editing, char *out, size_t outSize)
{
  ParamId_t id;
  bool isFreq = false;
  int32_t value = 0;

  if (node->id == MENU_ID_FUNCTION_TRAFFIC_OP)
  {
    static const char *const opts[] = {"START", "STOP", "CLEAR"};
    int32_t op = 0;
    if (editing)
    {
      op = s_paramInputValue;
    }
    else
    {
      (void)APP_ParamDict_GetValue(PARAM_ID_TXRX_OPERATION, &op);
      s_trafficOpValue = op;
    }
    if ((op >= 0) && (op <= 2))
    {
      (void)snprintf(out, outSize, "%s", opts[op]);
    }
    else
    {
      (void)snprintf(out, outSize, "?");
    }
    return;
  }

  if (node->id == MENU_ID_SETTING_DEVICE_NAME)
  {
    int32_t nameToken = 0;
    (void)APP_ParamDict_GetValue(PARAM_ID_DEVICE_NAME_TOKEN, &nameToken);
    (void)snprintf(out, outSize, "device%ld", (long)nameToken);
    return;
  }

  if (node->id == MENU_ID_SETTING_UART_DATABITS)
  {
    static const char *const opts[] = {"5", "6", "7", "8"};
    int32_t idx = 0;
    if (editing)
    {
      idx = s_paramInputValue;
    }
    else
    {
      (void)APP_ParamDict_GetValue(PARAM_ID_UART_DATABITS, &idx);
      s_uartDataBitsIdx = idx;
    }
    if ((idx >= 0) && (idx <= 3))
    {
      (void)snprintf(out, outSize, "%s", opts[idx]);
    }
    else
    {
      (void)snprintf(out, outSize, "?");
    }
    return;
  }

  if (node->id == MENU_ID_SETTING_UART_STOPBITS)
  {
    static const char *const opts[] = {"1", "1.5", "2"};
    int32_t idx = 0;
    if (editing)
    {
      idx = s_paramInputValue;
    }
    else
    {
      (void)APP_ParamDict_GetValue(PARAM_ID_UART_STOPBITS, &idx);
      s_uartStopBitsIdx = idx;
    }
    if ((idx >= 0) && (idx <= 2))
    {
      (void)snprintf(out, outSize, "%s", opts[idx]);
    }
    else
    {
      (void)snprintf(out, outSize, "?");
    }
    return;
  }

  if (node->id == MENU_ID_SETTING_UART_PARITY)
  {
    static const char *const opts[] = {"NONE", "ODD", "EVEN", "MARK", "SPACE"};
    int32_t idx = 0;
    if (editing)
    {
      idx = s_paramInputValue;
    }
    else
    {
      (void)APP_ParamDict_GetValue(PARAM_ID_UART_PARITY, &idx);
      s_uartParityIdx = idx;
    }
    if ((idx >= 0) && (idx <= 4))
    {
      (void)snprintf(out, outSize, "%s", opts[idx]);
    }
    else
    {
      (void)snprintf(out, outSize, "?");
    }
    return;
  }

  if (node->id == MENU_ID_SETTING_UART_FLOW)
  {
    static const char *const opts[] = {"NONE", "XON/XOFF", "RTS/CTS", "DTR/DSR", "RTS+XON", "DTR/DSR"};
    int32_t idx = 0;
    if (editing)
    {
      idx = s_paramInputValue;
    }
    else
    {
      (void)APP_ParamDict_GetValue(PARAM_ID_UART_FLOW, &idx);
      s_uartFlowIdx = idx;
    }
    if ((idx >= 0) && (idx <= 5))
    {
      (void)snprintf(out, outSize, "%s", opts[idx]);
    }
    else
    {
      (void)snprintf(out, outSize, "?");
    }
    return;
  }

  if (node->id == MENU_ID_FUNCTION_TOTAL_TX)
  {
    int32_t ethTx = 0;
    int32_t voiceTx = 0;
    (void)APP_ParamDict_GetValue(PARAM_ID_ETH_TX_CNT, &ethTx);
    (void)APP_ParamDict_GetValue(PARAM_ID_VOICE_TX_CNT, &voiceTx);
    (void)snprintf(out, outSize, "%ld", (long)(ethTx + voiceTx));
    return;
  }

  if (node->id == MENU_ID_FUNCTION_TOTAL_RX)
  {
    int32_t ethRx = 0;
    int32_t voiceRx = 0;
    (void)APP_ParamDict_GetValue(PARAM_ID_ETH_RX_CNT, &ethRx);
    (void)APP_ParamDict_GetValue(PARAM_ID_VOICE_RX_CNT, &voiceRx);
    (void)snprintf(out, outSize, "%ld", (long)(ethRx + voiceRx));
    return;
  }

  if (!Menu_GetDisplayConfig(node, &id, &isFreq))
  {
    (void)snprintf(out, outSize, "-");
    return;
  }

  if (editing)
  {
    value = s_paramInputValue;
  }
  else
  {
    int32_t raw = 0;
    (void)APP_ParamDict_GetValue(id, &raw);
    value = isFreq ? (raw / 1000) : raw;
  }

  if (node->id == MENU_ID_OPERATION_BW)
  {
    static const char *const opts[] = {"20", "10", "5", "2.5"};
    if ((value >= 0) && (value <= 3))
    {
      (void)snprintf(out, outSize, "%s", opts[value]);
    }
    else
    {
      (void)snprintf(out, outSize, "?");
    }
    return;
  }

  if (node->id == MENU_ID_FUNCTION_DETAIL_CH1_BW)
  {
    static const char *const opts[] = {"25K", "2.5", "5", "10", "20"};
    if ((value >= 0) && (value <= 4))
    {
      (void)snprintf(out, outSize, "%s", opts[value]);
    }
    else
    {
      (void)snprintf(out, outSize, "?");
    }
    return;
  }

  if (node->id == MENU_ID_OPERATION_MODE)
  {
    (void)snprintf(out, outSize, "%s", (value == 0) ? "WB" : "COG");
    return;
  }

  if ((node->id == MENU_ID_OPERATION_SPATIAL) ||
      (node->id == MENU_ID_FUNCTION_DETAIL_FILTER))
  {
    (void)snprintf(out, outSize, "%s", Menu_OnOffText(value));
    return;
  }

  if (node->id == MENU_ID_OPERATION_SYNC)
  {
    (void)snprintf(out, outSize, "%s", Menu_SyncText(value));
    return;
  }

  if ((node->id == MENU_ID_OPERATION_HOPMODE) ||
      (node->id == MENU_ID_FUNCTION_DETAIL_CH1_HOP))
  {
    (void)snprintf(out, outSize, "%s", Menu_HopModeText(value));
    return;
  }

  if ((node->id == MENU_ID_OPERATION_ROUTE) ||
      (node->id == MENU_ID_FUNCTION_DETAIL_CH1_ROUTE))
  {
    static const char *const opts[] = {"OLSR", "AODV", "BATMAN"};
    if ((value >= 0) && (value <= 2))
    {
      (void)snprintf(out, outSize, "%s", opts[value]);
    }
    else
    {
      (void)snprintf(out, outSize, "?");
    }
    return;
  }

  if ((node->id == MENU_ID_OPERATION_ACCESS) ||
      (node->id == MENU_ID_FUNCTION_DETAIL_CH1_ACCESS))
  {
    (void)snprintf(out, outSize, "TDMA");
    return;
  }

  if ((node->id == MENU_ID_OPERATION_TX_POWER) ||
      (node->id == MENU_ID_FUNCTION_DETAIL_CH1_TXPWR))
  {
    (void)snprintf(out, outSize, "%s", Menu_TxPowerText(value));
    return;
  }

  if (node->id == MENU_ID_FUNCTION_SYS_SAT)
  {
    (void)snprintf(out, outSize, "%s", (value == 0) ? "LOCK" : "SEARCH");
    return;
  }

  if ((node->id == MENU_ID_SETTING_NET_BIZ_IP) ||
      (node->id == MENU_ID_SETTING_NET_MGMT_IP) ||
      (node->id == MENU_ID_SETTING_NET_SENSE_IP) ||
      (node->id == MENU_ID_FUNCTION_MEMBER_NODE_IP) ||
      (node->id == MENU_ID_FUNCTION_DETAIL_IP))
  {
    uint32_t ip = (uint32_t)value;
    (void)snprintf(out, outSize, "%lu.%lu.%lu.%lu",
                   (unsigned long)((ip >> 24) & 0xFFu),
                   (unsigned long)((ip >> 16) & 0xFFu),
                   (unsigned long)((ip >> 8) & 0xFFu),
                   (unsigned long)(ip & 0xFFu));
    return;
  }

  if ((node->id == MENU_ID_SETTING_LOC_AUTO) ||
      (node->id == MENU_ID_SETTING_TIME_AUTO))
  {
    (void)snprintf(out, outSize, "%s", (value == 0) ? "ON" : "OFF");
    return;
  }

  if ((node->id == MENU_ID_SETTING_LOC_LON) ||
      (node->id == MENU_ID_SETTING_LOC_LAT) ||
      (node->id == MENU_ID_FUNCTION_MEMBER_POS_LON) ||
      (node->id == MENU_ID_FUNCTION_MEMBER_POS_LAT))
  {
    uint32_t mag = (value < 0) ? ((uint32_t)(-(value + 1)) + 1u) : (uint32_t)value;
    uint32_t scaled5 = (mag + 5u) / 10u;
    (void)snprintf(out, outSize, "%s%lu.%05lu",
                   (value < 0) ? "-" : "",
                   (unsigned long)(scaled5 / 100000u),
                   (unsigned long)(scaled5 % 100000u));
    return;
  }

  if ((node->id == MENU_ID_SETTING_LOC_ALT) ||
      (node->id == MENU_ID_FUNCTION_MEMBER_POS_ALT))
  {
    uint32_t mag = (value < 0) ? ((uint32_t)(-(value + 1)) + 1u) : (uint32_t)value;
    (void)snprintf(out, outSize, "%s%lu.%01lu",
                   (value < 0) ? "-" : "",
                   (unsigned long)(mag / 10u),
                   (unsigned long)(mag % 10u));
    return;
  }

  (void)snprintf(out, outSize, "%ld", (long)value);
}

static void Render_MemberNodeList(const MenuNode_t *node)
{
  uint8_t count;
  uint8_t row;
  uint8_t pageStart;

  if (node == 0)
  {
    return;
  }

  count = APP_UartRx_GetNeighborCountCached();
  if (count == 0u)
  {
    s_memberNodeSelect = 1u;
  }
  else
  {
    if (s_memberNodeSelect < 1u)
    {
      s_memberNodeSelect = 1u;
    }
    if (s_memberNodeSelect > count)
    {
      s_memberNodeSelect = count;
    }
  }

  Menu_PrintHeaderPath(node->parent != 0 ? node->parent : node);

  if (count == 0u)
  {
    BSP_Lcd_PrintLine(1, "NO MEMBER");
    Menu_RenderBlankRows(2, 7);
    return;
  }

  pageStart = (uint8_t)(((uint8_t)(s_memberNodeSelect - 1u) / 7u) * 7u);

  for (row = 1u; row <= 7u; ++row)
  {
    uint8_t idx = (uint8_t)(pageStart + row);
    if (idx <= count)
    {
      uint8_t memberId = 0u;
      char line[32];
      bool ok = APP_UartRx_GetNeighborNodeIdByIndex(idx, &memberId);

      (void)snprintf(line, sizeof(line), "%cNODE %u ID:%u",
                     (idx == s_memberNodeSelect) ? '*' : ' ',
                     (unsigned int)idx,
                     (unsigned int)(ok ? memberId : 0u));
      BSP_Lcd_PrintLine(row, line);
    }
    else
    {
      BSP_Lcd_PrintLine(row, "");
    }
  }
}

static void Render_OperationTree(const MenuNode_t *node)
{
  const MenuNode_t *head;
  const MenuNode_t *curr;
  int visibleIndex = 0;
  int pageStart;

  if ((node == 0) || (node->parent == 0) || (node->parent->child == 0))
  {
    return;
  }

  if (!Menu_IsNodeVisible(node))
  {
    node = Menu_FirstVisibleChild(node->parent);
    if (node == 0)
    {
      return;
    }
  }

  head = node->parent->child;
  curr = head;

  while ((curr != 0) && (curr != node))
  {
    if (Menu_IsNodeVisible(curr))
    {
      visibleIndex++;
    }
    curr = curr->sibling;
  }

  pageStart = (visibleIndex / 7) * 7;

  Menu_PrintHeaderPath(node->parent);

  curr = head;
  for (int i = 0; (curr != 0) && (i < pageStart); )
  {
    if (Menu_IsNodeVisible(curr))
    {
      ++i;
    }
    curr = curr->sibling;
  }

  for (int row = 1; row <= 7; ++row)
  {
    while ((curr != 0) && !Menu_IsNodeVisible(curr))
    {
      curr = curr->sibling;
    }

    if (curr == 0)
    {
      BSP_Lcd_PrintLine((uint8_t)row, "");
      continue;
    }

    {
      char valueBuf[16];
      char lineBuf[32];
      ParamId_t id;
      bool isFreq = false;
      bool hasValue;
      bool editing = (curr == node) && s_paramEditing;
      hasValue = Menu_GetDisplayConfig(curr, &id, &isFreq);

      if (hasValue)
      {
        Menu_FormatOperationValue(curr, editing, valueBuf, sizeof(valueBuf));
      }
      else
      {
        valueBuf[0] = '\0';
      }

      if (editing && hasValue)
      {
        (void)snprintf(lineBuf, sizeof(lineBuf), ">%s [*%s]", curr->title, valueBuf);
      }
      else if (hasValue)
      {
        if (curr == node)
        {
          (void)snprintf(lineBuf, sizeof(lineBuf), ">%s %s", curr->title, valueBuf);
        }
        else
        {
          (void)snprintf(lineBuf, sizeof(lineBuf), " %s %s", curr->title, valueBuf);
        }
      }
      else
      {
        const char *suffix = (curr->child != 0) ? " >" : "";
        if (curr == node)
        {
          (void)snprintf(lineBuf, sizeof(lineBuf), ">%s%s", curr->title, suffix);
        }
        else
        {
          (void)snprintf(lineBuf, sizeof(lineBuf), " %s%s", curr->title, suffix);
        }
      }

      BSP_Lcd_PrintLine((uint8_t)row, lineBuf);
    }

    curr = curr->sibling;
  }
}

static int MenuKeyHandler_OperationItem(MenuKey_t key)
{
  ParamId_t id;
  int32_t minVal = 0;
  int32_t maxVal = 0;
  bool isFreq = false;
  int32_t current = 0;
  int digit;

  if (!Menu_GetOperationConfig(s_current, &id, &minVal, &maxVal, &isFreq))
  {
    return 0;
  }

  (void)APP_ParamDict_GetValue(id, &current);
  if (isFreq)
  {
    current /= 1000;
  }

  if (!s_paramEditing)
  {
    if (key == MENU_KEY_STAR)
    {
      s_paramEditing = true;
      s_paramInputValue = current;
      s_paramInputDigits = 0u;
      return 1;
    }
    return 0;
  }

  digit = Menu_KeyToDigit(key);
  if (digit >= 0)
  {
    if (s_paramInputDigits < 10u)
    {
      s_paramInputValue = (s_paramInputValue * 10) + digit;
      s_paramInputDigits++;
    }
    return 1;
  }

  if (key == MENU_KEY_UP)
  {
    if (s_paramInputValue > minVal)
    {
      s_paramInputValue--;
    }
    return 1;
  }

  if (key == MENU_KEY_DOWN)
  {
    if (s_paramInputValue < maxVal)
    {
      s_paramInputValue++;
    }
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

  if (key == MENU_KEY_STAR)
  {
    s_paramInputValue = current;
    s_paramInputDigits = 0u;
    return 1;
  }

  if (key == MENU_KEY_BACK)
  {
    s_paramEditing = false;
    s_paramInputDigits = 0u;
    return 1;
  }

  if (key == MENU_KEY_OK)
  {
    int32_t applyVal = s_paramInputValue;

    if (applyVal < minVal)
    {
      applyVal = minVal;
    }
    else if (applyVal > maxVal)
    {
      applyVal = maxVal;
    }

    if (isFreq)
    {
      applyVal *= 1000;
    }

    if ((s_current != 0) &&
        ((s_current->id == MENU_ID_OPERATION_FREQ1) ||
         (s_current->id == MENU_ID_OPERATION_FREQ2) ||
         (s_current->id == MENU_ID_OPERATION_FREQ3) ||
         (s_current->id == MENU_ID_OPERATION_FREQ4)))
    {
      uint8_t bit = 0u;
      int32_t f1 = 0;
      int32_t f2 = 0;
      int32_t f3 = 0;
      int32_t f4 = 0;

      switch (s_current->id)
      {
        case MENU_ID_OPERATION_FREQ1:
          bit = 0x01u;
          (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ1, applyVal);
          break;
        case MENU_ID_OPERATION_FREQ2:
          bit = 0x02u;
          (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ2, applyVal);
          break;
        case MENU_ID_OPERATION_FREQ3:
          bit = 0x04u;
          (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ3, applyVal);
          break;
        case MENU_ID_OPERATION_FREQ4:
          bit = 0x08u;
          (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ4, applyVal);
          break;
        default: break;
      }

      s_adaptFreqEditMask |= bit;

      if (s_adaptFreqEditMask == 0x0Fu)
      {
        (void)APP_ParamDict_GetValue(PARAM_ID_ADAPT_FREQ1, &f1);
        (void)APP_ParamDict_GetValue(PARAM_ID_ADAPT_FREQ2, &f2);
        (void)APP_ParamDict_GetValue(PARAM_ID_ADAPT_FREQ3, &f3);
        (void)APP_ParamDict_GetValue(PARAM_ID_ADAPT_FREQ4, &f4);

        /* 先发送四个频点，再发送 HOP=ADPT，确保主控提交的是本轮新值。 */
        (void)APP_ParamUpdate_RequestValue(PARAM_ID_ADAPT_FREQ1, f1);
        (void)APP_ParamUpdate_RequestValue(PARAM_ID_ADAPT_FREQ2, f2);
        (void)APP_ParamUpdate_RequestValue(PARAM_ID_ADAPT_FREQ3, f3);
        (void)APP_ParamUpdate_RequestValue(PARAM_ID_ADAPT_FREQ4, f4);
        (void)APP_ParamUpdate_RequestValue(PARAM_ID_FREQ_HOP_MODE, 1);

        s_adaptFreqEditMask = 0u;
      }
    }
    else if ((s_current != 0) &&
             (s_current->id == MENU_ID_OPERATION_HOPMODE) &&
             (applyVal == 1))
    {
      /* 切到 ADPT 仅进入“待配置四频点”状态，不立即下发。 */
      (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_FREQ_HOP_MODE, applyVal);
      s_adaptFreqEditMask = 0u;
      s_adaptHopProtectActive = true;
    }
    else
    {
      if ((s_current != 0) && (s_current->id == MENU_ID_OPERATION_HOPMODE))
      {
        s_adaptFreqEditMask = 0u;
        s_adaptHopProtectActive = false;
      }
      (void)APP_ParamUpdate_RequestValue(id, applyVal);
    }

    s_paramEditing = false;
    s_paramInputDigits = 0u;
    return 1;
  }

  return 1;
}

static int MenuKeyHandler_TrafficOp(MenuKey_t key)
{
  int32_t current = 0;

  (void)APP_ParamDict_GetValue(PARAM_ID_TXRX_OPERATION, &current);

  if (!s_paramEditing)
  {
    if (key == MENU_KEY_STAR)
    {
      s_paramEditing = true;
      s_paramInputValue = current;
      s_paramInputDigits = 0u;
      return 1;
    }
    return 0;
  }

  {
    int digit = Menu_KeyToDigit(key);
    if (digit >= 0)
    {
      s_paramInputValue = digit;
      s_paramInputDigits = 1u;
      return 1;
    }
  }

  if (key == MENU_KEY_UP)
  {
    if (s_paramInputValue > 0)
    {
      s_paramInputValue--;
    }
    return 1;
  }

  if (key == MENU_KEY_DOWN)
  {
    if (s_paramInputValue < 2)
    {
      s_paramInputValue++;
    }
    return 1;
  }

  if (key == MENU_KEY_HASH)
  {
    s_paramInputValue = 0;
    s_paramInputDigits = 0u;
    return 1;
  }

  if (key == MENU_KEY_BACK)
  {
    s_paramEditing = false;
    s_paramInputDigits = 0u;
    return 1;
  }

  if (key == MENU_KEY_OK)
  {
    int32_t applyVal = s_paramInputValue;

    if (applyVal < 0)
    {
      applyVal = 0;
    }
    else if (applyVal > 2)
    {
      applyVal = 2;
    }

    (void)APP_ParamUpdate_RequestValue(PARAM_ID_TXRX_OPERATION, applyVal);

    s_paramEditing = false;
    s_paramInputDigits = 0u;
    return 1;
  }

  return 1;
}

static int MenuKeyHandler_MemberNodeList(MenuKey_t key)
{
  uint8_t count = APP_UartRx_GetNeighborCountCached();
  uint8_t memberId = 0u;
  uint8_t payload[1];
  int digit = Menu_KeyToDigit(key);

  if (count == 0u)
  {
    if ((key == MENU_KEY_OK) || (key == MENU_KEY_STAR))
    {
      return 1;
    }
    return 0;
  }

  if (s_memberNodeSelect < 1u)
  {
    s_memberNodeSelect = 1u;
  }
  if (s_memberNodeSelect > count)
  {
    s_memberNodeSelect = count;
  }

  if ((digit >= 1) && (digit <= 9))
  {
    if ((uint8_t)digit <= count)
    {
      s_memberNodeSelect = (uint8_t)digit;
    }
    return 1;
  }

  if (key == MENU_KEY_UP)
  {
    if (s_memberNodeSelect > 1u)
    {
      s_memberNodeSelect--;
    }
    return 1;
  }

  if (key == MENU_KEY_DOWN)
  {
    if (s_memberNodeSelect < count)
    {
      s_memberNodeSelect++;
    }
    return 1;
  }

  if ((key == MENU_KEY_OK) || (key == MENU_KEY_STAR))
  {
    (void)APP_UartRx_LoadNeighborByIndexToParams(s_memberNodeSelect);

    if (APP_UartRx_GetNeighborNodeIdByIndex(s_memberNodeSelect, &memberId))
    {
      payload[0] = memberId;
      (void)APP_UartProto_SendRaw(0x0Au, 0xFFu, payload, 1u);
    }

    if (key == MENU_KEY_STAR)
    {
      return 1;
    }

    return 0;
  }

  return 0;
}

static int MenuKeyHandler_DetailRequest(MenuKey_t key)
{
  if ((key == MENU_KEY_OK) || (key == MENU_KEY_STAR))
  {
    uint8_t memberId = 0u;
    uint8_t payload[1];

    if (!APP_UartRx_GetNeighborNodeIdByIndex(s_memberNodeSelect, &memberId))
    {
      return 1;
    }

    payload[0] = memberId;
    (void)APP_UartProto_SendRaw(0x0Au, 0xFFu, payload, 1u);

    if (key == MENU_KEY_STAR)
    {
      return 1;
    }

    return 0;
  }

  return 0;
}

/* 串口波特率编辑逻辑：与操作菜单保持一致
 * - '*' 进入编辑
 * - 编辑态下支持数字键/UP/DOWN 调整，OK 确认，BACK 取消
 */
static int MenuKeyHandler_UartBaud(MenuKey_t key)
{
  int digit;
  int32_t current = 0;
  int32_t currentIndex = 0;
  if (!APP_ParamDict_GetValue(PARAM_ID_UART_BAUDRATE, &current))
  {
    return 0;
  }

  /* 与菜单目录定义一致: 0~7 */
  static const int32_t baudList[] = {9600, 19200, 38400, 57600, 115200, 256000, 460800, 921600};
  const uint32_t baudCount = (uint32_t)(sizeof(baudList) / sizeof(baudList[0]));

  /* 找到当前所在索引 */
  for (uint32_t i = 0U; i < baudCount; ++i)
  {
    if (baudList[i] == current)
    {
      currentIndex = (int32_t)i;
      break;
    }
  }

  if (!s_paramEditing)
  {
    if (key == MENU_KEY_STAR)
    {
      s_paramEditing = true;
      s_paramInputValue = currentIndex;
      s_paramInputDigits = 0u;
      return 1;
    }
    return 0;
  }

  digit = Menu_KeyToDigit(key);
  if (digit >= 0)
  {
    if (s_paramInputDigits < 10u)
    {
      s_paramInputValue = (s_paramInputValue * 10) + digit;
      s_paramInputDigits++;
    }
    return 1;
  }

  if (key == MENU_KEY_UP)
  {
    if (s_paramInputValue > 0)
    {
      s_paramInputValue--;
    }
    return 1;
  }

  if (key == MENU_KEY_DOWN)
  {
    if (s_paramInputValue < ((int32_t)baudCount - 1))
    {
      s_paramInputValue++;
    }
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

  if (key == MENU_KEY_STAR)
  {
    s_paramInputValue = currentIndex;
    s_paramInputDigits = 0u;
    return 1;
  }

  if (key == MENU_KEY_BACK)
  {
    s_paramEditing = false;
    s_paramInputDigits = 0u;
    return 1;
  }

  if (key == MENU_KEY_OK)
  {
    int32_t applyIndex = s_paramInputValue;

    if (applyIndex < 0)
    {
      applyIndex = 0;
    }
    else if (applyIndex >= (int32_t)baudCount)
    {
      applyIndex = (int32_t)baudCount - 1;
    }

    (void)APP_ParamUpdate_RequestValue(PARAM_ID_UART_BAUD_IDX, applyIndex);
    s_paramEditing = false;
    s_paramInputDigits = 0u;
    return 1;
  }

  return 1;
}

static int MenuKeyHandler_UartFormat(MenuKey_t key)
{
  int32_t *value = 0;
  ParamId_t id = 0;
  int32_t maxVal = 0;
  int digit;

  if (s_current == 0)
  {
    return 0;
  }

  switch (s_current->id)
  {
    case MENU_ID_SETTING_UART_DATABITS:
      value = &s_uartDataBitsIdx;
      id = PARAM_ID_UART_DATABITS;
      maxVal = 3;
      break;

    case MENU_ID_SETTING_UART_STOPBITS:
      value = &s_uartStopBitsIdx;
      id = PARAM_ID_UART_STOPBITS;
      maxVal = 2;
      break;

    case MENU_ID_SETTING_UART_PARITY:
      value = &s_uartParityIdx;
      id = PARAM_ID_UART_PARITY;
      maxVal = 4;
      break;

    case MENU_ID_SETTING_UART_FLOW:
      value = &s_uartFlowIdx;
      id = PARAM_ID_UART_FLOW;
      maxVal = 5;
      break;

    default:
      return 0;
  }

  if (!s_paramEditing)
  {
    if (key == MENU_KEY_STAR)
    {
      int32_t curr = *value;
      (void)APP_ParamDict_GetValue(id, &curr);
      *value = curr;
      s_paramEditing = true;
      s_paramInputValue = curr;
      s_paramInputDigits = 0u;
      return 1;
    }
    return 0;
  }

  digit = Menu_KeyToDigit(key);
  if (digit >= 0)
  {
    if (s_paramInputDigits < 10u)
    {
      s_paramInputValue = (s_paramInputValue * 10) + digit;
      s_paramInputDigits++;
    }
    return 1;
  }

  if (key == MENU_KEY_UP)
  {
    if (s_paramInputValue > 0)
    {
      s_paramInputValue--;
    }
    return 1;
  }

  if (key == MENU_KEY_DOWN)
  {
    if (s_paramInputValue < maxVal)
    {
      s_paramInputValue++;
    }
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

  if (key == MENU_KEY_STAR)
  {
    s_paramInputValue = *value;
    s_paramInputDigits = 0u;
    return 1;
  }

  if (key == MENU_KEY_BACK)
  {
    s_paramEditing = false;
    s_paramInputDigits = 0u;
    return 1;
  }

  if (key == MENU_KEY_OK)
  {
    int32_t applyVal = s_paramInputValue;

    if (applyVal < 0)
    {
      applyVal = 0;
    }
    else if (applyVal > maxVal)
    {
      applyVal = maxVal;
    }

    (void)APP_ParamUpdate_RequestValue(id, applyVal);
    s_paramEditing = false;
    s_paramInputDigits = 0u;
    return 1;
  }

  return 1;
}

static int MenuKeyHandler_TimeManual(MenuKey_t key)
{
  ParamId_t id;
  int32_t minVal = 0;
  int32_t maxVal = 0;
  int32_t current = 0;
  int digit;

  if (!Menu_GetManualTimeConfig(s_current, &id, &minVal, &maxVal))
  {
    return 0;
  }

  (void)APP_ParamDict_GetValue(id, &current);

  if (!s_paramEditing)
  {
    if (key == MENU_KEY_STAR)
    {
      s_paramEditing = true;
      s_paramInputValue = current;
      s_paramInputDigits = 0u;
      return 1;
    }
    return 0;
  }

  digit = Menu_KeyToDigit(key);
  if (digit >= 0)
  {
    if (s_paramInputDigits < 10u)
    {
      s_paramInputValue = (s_paramInputValue * 10) + digit;
      s_paramInputDigits++;
    }
    return 1;
  }

  if (key == MENU_KEY_UP)
  {
    if (s_paramInputValue > minVal)
    {
      s_paramInputValue--;
    }
    return 1;
  }

  if (key == MENU_KEY_DOWN)
  {
    if (s_paramInputValue < maxVal)
    {
      s_paramInputValue++;
    }
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

  if (key == MENU_KEY_STAR)
  {
    s_paramInputValue = current;
    s_paramInputDigits = 0u;
    return 1;
  }

  if (key == MENU_KEY_BACK)
  {
    s_paramEditing = false;
    s_paramInputDigits = 0u;
    return 1;
  }

  if (key == MENU_KEY_OK)
  {
    int32_t applyVal = s_paramInputValue;

    if (applyVal < minVal)
    {
      applyVal = minVal;
    }
    else if (applyVal > maxVal)
    {
      applyVal = maxVal;
    }

    (void)APP_ParamUpdate_RequestValue(id, applyVal);

    s_paramEditing = false;
    s_paramInputDigits = 0u;
    return 1;
  }

  return 1;
}


