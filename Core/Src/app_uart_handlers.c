#include "app_uart_rx.h"
#include "app_param_dict.h"
#include "app_menu.h"
#include "cmsis_os.h"

#include <stdint.h>

/* 由 freertos.c 中定义 */
extern osMutexId_t gParamMutexHandle;

#define NEIGHBOR_NODE_CACHE_MAX 64u

typedef struct
{
  uint8_t id;
  uint32_t ip;
  uint8_t hops;
  int8_t rssi;
  uint16_t delay;
  int32_t lon;
  int32_t lat;
  uint16_t alt;
} NeighborNodeCache_t;

static NeighborNodeCache_t s_neighborNodeCache[NEIGHBOR_NODE_CACHE_MAX];
static uint8_t s_neighborNodeCount = 0u;

static void cache_write_neighbor_to_params(const NeighborNodeCache_t *node)
{
  if (node == NULL)
  {
    return;
  }

  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NEIGHBOR_NODE_ID, (int32_t)node->id);
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NEIGHBOR_NODE_IP, (int32_t)node->ip);
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NEIGHBOR_NODE_HOPS, (int32_t)node->hops);
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NEIGHBOR_RSSI, (int32_t)node->rssi);
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NEIGHBOR_NODE_DELAY, (int32_t)node->delay);
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NEIGHBOR_NODE_LON, node->lon);
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NEIGHBOR_NODE_LAT, node->lat);
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NEIGHBOR_NODE_ALT, (int32_t)node->alt);
}

uint8_t APP_UartRx_GetNeighborCountCached(void)
{
  return s_neighborNodeCount;
}

bool APP_UartRx_GetNeighborNodeIdByIndex(uint8_t oneBasedIndex, uint8_t *outMemberId)
{
  uint8_t idx;

  if ((oneBasedIndex == 0u) || (outMemberId == NULL))
  {
    return false;
  }

  idx = (uint8_t)(oneBasedIndex - 1u);
  if (idx >= s_neighborNodeCount)
  {
    return false;
  }

  *outMemberId = s_neighborNodeCache[idx].id;
  return true;
}

bool APP_UartRx_LoadNeighborByIndexToParams(uint8_t oneBasedIndex)
{
  uint8_t idx;

  if (oneBasedIndex == 0u)
  {
    return false;
  }

  idx = (uint8_t)(oneBasedIndex - 1u);
  if (idx >= s_neighborNodeCount)
  {
    return false;
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexAcquire(gParamMutexHandle, osWaitForever);
  }

  cache_write_neighbor_to_params(&s_neighborNodeCache[idx]);

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexRelease(gParamMutexHandle);
  }

  return true;
}

static uint32_t read_be_u32(const uint8_t *p)
{
  return ((uint32_t)p[0] << 24) |
         ((uint32_t)p[1] << 16) |
         ((uint32_t)p[2] << 8)  |
         ((uint32_t)p[3]);
}

static uint32_t read_le_u32(const uint8_t *p)
{
  return ((uint32_t)p[0]) |
         ((uint32_t)p[1] << 8) |
         ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}

static int16_t read_be_i16(const uint8_t *p)
{
  return (int16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

static uint16_t read_le_u16(const uint8_t *p)
{
  return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static int32_t read_be_i32(const uint8_t *p)
{
  return (int32_t)read_be_u32(p);
}

static bool parse_ascii_coord_1e6(const uint8_t *buf, uint16_t len, int32_t *outVal)
{
  bool neg = false;
  bool seenDot = false;
  bool hasDigit = false;
  int64_t intPart = 0;
  int64_t fracPart = 0;
  uint8_t fracDigits = 0;

  if ((buf == NULL) || (outVal == NULL) || (len == 0u))
  {
    return false;
  }

  uint16_t i = 0u;
  while (i < len)
  {
    uint8_t c = buf[i];
    if ((c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'))
    {
      ++i;
      continue;
    }
    break;
  }

  if ((i < len) && (buf[i] == '-'))
  {
    neg = true;
    ++i;
  }
  else if ((i < len) && (buf[i] == '+'))
  {
    ++i;
  }

  for (; i < len; ++i)
  {
    uint8_t c = buf[i];

    if ((c == '\0') || (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'))
    {
      break;
    }

    if ((c == '.') && (!seenDot))
    {
      seenDot = true;
      continue;
    }

    if ((c < '0') || (c > '9'))
    {
      break;
    }

    hasDigit = true;
    if (!seenDot)
    {
      intPart = (intPart * 10) + (int64_t)(c - '0');
    }
    else if (fracDigits < 6u)
    {
      fracPart = (fracPart * 10) + (int64_t)(c - '0');
      ++fracDigits;
    }
  }

  if (!hasDigit)
  {
    return false;
  }

  while (fracDigits < 6u)
  {
    fracPart *= 10;
    ++fracDigits;
  }

  int64_t scaled = (intPart * 1000000LL) + fracPart;
  if (neg)
  {
    scaled = -scaled;
  }

  if (scaled > 2147483647LL)
  {
    scaled = 2147483647LL;
  }
  else if (scaled < (-2147483647LL - 1LL))
  {
    scaled = (-2147483647LL - 1LL);
  }

  *outVal = (int32_t)scaled;
  return true;
}

static int32_t uart_baud_from_idx(uint8_t idx)
{
  static const int32_t baudMap[] = {9600, 19200, 38400, 57600, 115200, 256000, 460800, 921600};
  if (idx < (uint8_t)(sizeof(baudMap) / sizeof(baudMap[0])))
  {
    return baudMap[idx];
  }
  return 115200;
}

static int32_t parse_name_token(const uint8_t *name, uint16_t len, uint8_t fallback)
{
  uint32_t token = 0u;
  bool hasDigit = false;

  if ((name == NULL) || (len == 0u))
  {
    return (int32_t)fallback;
  }

  for (uint16_t i = 0u; i < len; ++i)
  {
    uint8_t c = name[i];
    if ((c >= '0') && (c <= '9'))
    {
      hasDigit = true;
      token = (token * 10u) + (uint32_t)(c - '0');
      if (token > 2147483647u)
      {
        token = 2147483647u;
        break;
      }
    }
  }

  if (!hasDigit)
  {
    token = (uint32_t)fallback;
  }

  return (int32_t)token;
}

/* 0x02: 参数写入应答
 * - msgType: 0x00=成功, 0x01=失败
 * - payload[0] (可选): 被应答命令字
 */
static void handle_cmd_0x02(uint8_t msgType, const uint8_t *payload, uint16_t payloadLen)
{
  int32_t ackState = 0;
  int32_t ackCmd = 0x01;

  if (msgType == 0x00u)
  {
    ackState = 1;
  }
  else if (msgType == 0x01u)
  {
    ackState = 2;
  }

  if ((payload != NULL) && (payloadLen >= 1u))
  {
    ackCmd = (int32_t)payload[0];
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexAcquire(gParamMutexHandle, osWaitForever);
  }

  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_UART_ACK_STATE, ackState);
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_UART_ACK_CMD, ackCmd);

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexRelease(gParamMutexHandle);
  }

  APP_Menu_RefreshCurrent();
}

/* 0x08: 开机显示参数
 * payload 固定顺序（按图片/协议表定义）：
 * [0..2]   当前时间: hour/min/sec (3 * uint1)
 * [3]      入网状态
 * [4]      波形档位
 * [5]      信号带宽
 * [6]      跳频方式
 * [7..10]  点频工作频点 (1000uint4)
 * [11..14] 自适应选频1 (1000uint4)
 * [15..18] 自适应选频2 (1000uint4)
 * [19..22] 自适应选频3 (1000uint4)
 * [23..26] 自适应选频4 (1000uint4)
 * [27]     空域滤波
 * [28]     内外同步模式
 * [29]     发射功率
 * [30]     发射功率衰减量
 */
static void handle_cmd_0x08(const uint8_t *payload, uint16_t payloadLen)
{
  bool protectAdapt;

  if ((payload == NULL) || (payloadLen == 0u))
  {
    return;
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexAcquire(gParamMutexHandle, osWaitForever);
  }

  protectAdapt = (APP_Menu_IsAdaptHopProtectActive() != 0u);

  if (payloadLen >= 1u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_TIME_HOUR, (int32_t)payload[0]);
  }
  if (payloadLen >= 2u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_TIME_MINUTE, (int32_t)payload[1]);
  }
  if (payloadLen >= 3u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_TIME_SECOND, (int32_t)payload[2]);
  }
  if (payloadLen >= 4u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NET_JOIN_STATE, (int32_t)payload[3]);
  }
  if (payloadLen >= 5u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_WAVEFORM_GEAR, (int32_t)payload[4]);
  }
  if (payloadLen >= 6u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_SIGNAL_BANDWIDTH, (int32_t)payload[5]);
  }
  if (payloadLen >= 7u)
  {
    uint8_t reportedHop = payload[6];
    if ((!protectAdapt) || (reportedHop == 1u))
    {
      (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_FREQ_HOP_MODE, (int32_t)reportedHop);
      if (protectAdapt && (reportedHop == 1u))
      {
        APP_Menu_ClearAdaptHopProtect();
        protectAdapt = false;
      }
    }
  }
  if (payloadLen >= 11u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_FIXED_FREQ, (int32_t)read_le_u32(&payload[7]));
  }
  if ((payloadLen >= 15u) && (!protectAdapt))
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ1, (int32_t)read_le_u32(&payload[11]));
  }
  if ((payloadLen >= 19u) && (!protectAdapt))
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ2, (int32_t)read_le_u32(&payload[15]));
  }
  if ((payloadLen >= 23u) && (!protectAdapt))
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ3, (int32_t)read_le_u32(&payload[19]));
  }
  if ((payloadLen >= 27u) && (!protectAdapt))
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ4, (int32_t)read_le_u32(&payload[23]));
  }
  if (payloadLen >= 28u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_SPATIAL_FILTER, (int32_t)payload[27]);
  }
  if (payloadLen >= 29u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_SYNC_MODE, (int32_t)payload[28]);
  }
  if (payloadLen >= 30u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_TX_POWER, (int32_t)payload[29]);
  }
  if (payloadLen >= 31u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_TX_POWER_ATTEN, (int32_t)payload[30]);
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexRelease(gParamMutexHandle);
  }

  APP_Menu_RefreshCurrent();
}

/* 0x05: 设备基础信息
 * 兼容两种来源：
 * 1) 新版（ui_practice_test::Send_0x05）完整 DeviceConfig（61B）
 * 2) 旧版简化映射（IP+i32经纬度+i16高度+sat）
 */
static void handle_cmd_0x05(const uint8_t *payload, uint16_t payloadLen)
{
  if ((payload == NULL) || (payloadLen < 4u))
  {
    return;
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexAcquire(gParamMutexHandle, osWaitForever);
  }

  if (payloadLen >= 61u)
  {
    int32_t lon = 0;
    int32_t lat = 0;
    int32_t altitude = (int32_t)read_le_u16(&payload[55]);
    uint32_t ip = ((uint32_t)payload[11] << 24) |
                  ((uint32_t)payload[12] << 16) |
                  ((uint32_t)payload[13] << 8) |
                  ((uint32_t)payload[14]);

    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_DEVICE_ID, (int32_t)payload[0]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_DEVICE_NAME_TOKEN, parse_name_token(&payload[1], 10u, payload[0]));
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NET_IP_ADDR, (int32_t)ip);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NET_BIZ_PORT, (int32_t)read_le_u16(&payload[15]));
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NET_MGMT_IP, (int32_t)(((uint32_t)payload[17] << 24) |
                                                                        ((uint32_t)payload[18] << 16) |
                                                                        ((uint32_t)payload[19] << 8) |
                                                                        ((uint32_t)payload[20])));
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NET_MGMT_PORT, (int32_t)read_le_u16(&payload[21]));
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NET_SENSE_IP, (int32_t)(((uint32_t)payload[23] << 24) |
                                                                         ((uint32_t)payload[24] << 16) |
                                                                         ((uint32_t)payload[25] << 8) |
                                                                         ((uint32_t)payload[26])));
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NET_SENSE_PORT, (int32_t)read_le_u16(&payload[27]));
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_UART_BAUD_IDX, (int32_t)payload[29]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_UART_DATABITS, (int32_t)payload[30]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_UART_STOPBITS, (int32_t)payload[31]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_UART_PARITY, (int32_t)payload[32]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_UART_FLOW, (int32_t)payload[33]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_LOC_AUTO_SYNC, (int32_t)payload[34]);

    if (parse_ascii_coord_1e6(&payload[35], 10u, &lon))
    {
      (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_GPS_LONGITUDE, lon);
    }
    if (parse_ascii_coord_1e6(&payload[45], 10u, &lat))
    {
      (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_GPS_LATITUDE, lat);
    }

    if (altitude > 32767)
    {
      altitude = 32767;
    }
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_GPS_ALTITUDE, altitude);

    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_UART_BAUDRATE, uart_baud_from_idx(payload[29]));
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_TIME_AUTO_SYNC, (int32_t)payload[57]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_TIME_HOUR, (int32_t)payload[58]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_TIME_MINUTE, (int32_t)payload[59]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_TIME_SECOND, (int32_t)payload[60]);
  }
  else
  {
    /* 兼容旧版简化帧。 */
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NET_IP_ADDR, (int32_t)read_be_u32(payload));

    if (payloadLen >= 8u)
    {
      (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_GPS_LONGITUDE, read_be_i32(&payload[4]));
    }
    if (payloadLen >= 12u)
    {
      (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_GPS_LATITUDE, read_be_i32(&payload[8]));
    }
    if (payloadLen >= 14u)
    {
      (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_GPS_ALTITUDE, (int32_t)read_be_i16(&payload[12]));
    }
    if (payloadLen >= 15u)
    {
      (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_SAT_LOCK, (int32_t)payload[14]);
    }
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexRelease(gParamMutexHandle);
  }

  APP_Menu_RefreshCurrent();
}

/* 0x06: 流量统计
 * 与 ui_practice_test::Send_0x06 对齐（MsgStats，packed）：
 * payload[0..1]   : TOTAL_TX   (u16, little-endian)
 * payload[2..3]   : TOTAL_RX   (u16, little-endian)
 * payload[4..5]   : VOICE_TX   (u16, little-endian)
 * payload[6..7]   : VOICE_RX   (u16, little-endian)
 * payload[8..9]   : ETH_TX     (u16, little-endian)
 * payload[10..11] : ETH_RX     (u16, little-endian)
 *
 * 兼容旧格式：若收到 >=16 字节，按旧版 4xu32(big-endian) 解析。
 */
static void handle_cmd_0x06(const uint8_t *payload, uint16_t payloadLen)
{
  if ((payload == NULL) || (payloadLen == 0u))
  {
    return;
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexAcquire(gParamMutexHandle, osWaitForever);
  }

  if (payloadLen == 12u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_VOICE_TX_CNT, (int32_t)read_le_u16(&payload[4]));
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_VOICE_RX_CNT, (int32_t)read_le_u16(&payload[6]));
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ETH_TX_CNT, (int32_t)read_le_u16(&payload[8]));
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ETH_RX_CNT, (int32_t)read_le_u16(&payload[10]));
  }
  else if (payloadLen >= 16u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ETH_TX_CNT, (int32_t)read_be_u32(&payload[0]));
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ETH_RX_CNT, (int32_t)read_be_u32(&payload[4]));
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_VOICE_TX_CNT, (int32_t)read_be_u32(&payload[8]));
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_VOICE_RX_CNT, (int32_t)read_be_u32(&payload[12]));
  }
  else
  {
    if (gParamMutexHandle != NULL)
    {
      (void)osMutexRelease(gParamMutexHandle);
    }
    return;
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexRelease(gParamMutexHandle);
  }

  APP_Menu_RefreshCurrent();
}

/* 0x07: 自检状态
 * 与 ui_practice_test::Send_0x07 对齐（Frame_07_byte，packed）：
 * SystemSelfTestStatus 固定 38 字节，后续 4 组 RFChannelStatus（每组 9 字节）。
 * 当前屏显使用字段：
 * [0]      self_test_status
 * [1]      battery_remaining_capacity
 * [5]      info_processor_temp
 * [6]      fan_speed_status
 * [7]      nav_lock_status
 * [8]      clock_selection
 * [9]      adc_status
 * [10]     clock_source_temp
 * [11..12] freq_word_send_count (u16, little-endian)
 * [13]     comm_sensing_status
 *
 * 兼容旧版简化帧：payload<38 时按旧格式解析。
 */
static void handle_cmd_0x07(const uint8_t *payload, uint16_t payloadLen)
{
  if ((payload == NULL) || (payloadLen < 1u))
  {
    return;
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexAcquire(gParamMutexHandle, osWaitForever);
  }

  if (payloadLen >= 38u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_SELFTEST_STATE, (int32_t)payload[0]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_BATTERY_CAP, (int32_t)payload[1]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ENV_TEMPERATURE, (int32_t)payload[5]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_FAN_STATE, (int32_t)payload[6]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_SAT_LOCK, (int32_t)payload[7]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_CLOCK_SELECTION, (int32_t)payload[8]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADC_STATUS, (int32_t)payload[9]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_CLOCK_SRC_TEMP, (int32_t)payload[10]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_FREQ_WORD_CNT, (int32_t)read_le_u16(&payload[11]));
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_COMM_SENSE_ST, (int32_t)payload[13]);
  }
  else
  {
    /* 兼容旧版简化帧 */
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_BATTERY_CAP, (int32_t)payload[0]);

    if (payloadLen >= 3u)
    {
      (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ENV_TEMPERATURE, (int32_t)read_be_i16(&payload[1]));
    }
    if (payloadLen >= 4u)
    {
      (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_FAN_STATE, (int32_t)payload[3]);
    }
    if (payloadLen >= 5u)
    {
      (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_SAT_LOCK, (int32_t)payload[4]);
    }
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexRelease(gParamMutexHandle);
  }

  APP_Menu_RefreshCurrent();
}

/* 0x09: 邻居信息
 * 与 ui_practice_test::Send_0x09 对齐：
 * payload[0] = neighbor count
 * payload[1..] = NetworkNodeStatus[] (packed, each 31 bytes)
 *   [0]      member_id
 *   [1..4]   ip_address[4]
 *   [5]      hop_count
 *   [6]      signal_strength (i8)
 *   [7..8]   transmission_delay (u16, little-endian)
 *   [9..18]  longitude[10] (ASCII)
 *   [19..28] latitude[10]  (ASCII)
 *   [29..30] altitude (u16, little-endian)
 *
 * 当前屏显先落库第 1 个节点信息。
 */
static void handle_cmd_0x09(const uint8_t *payload, uint16_t payloadLen)
{
  enum { NODE_STATUS_SIZE = 31 };

  if ((payload == NULL) || (payloadLen < 1u))
  {
    return;
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexAcquire(gParamMutexHandle, osWaitForever);
  }

  {
    uint8_t declaredCount = payload[0];
    uint16_t bytesAvail = (payloadLen > 1u) ? (uint16_t)(payloadLen - 1u) : 0u;
    uint8_t parseCount = (uint8_t)(bytesAvail / NODE_STATUS_SIZE);

    if (parseCount > declaredCount)
    {
      parseCount = declaredCount;
    }
    if (parseCount > NEIGHBOR_NODE_CACHE_MAX)
    {
      parseCount = NEIGHBOR_NODE_CACHE_MAX;
    }

    s_neighborNodeCount = parseCount;
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NEIGHBOR_COUNT, (int32_t)declaredCount);

    for (uint8_t i = 0u; i < parseCount; ++i)
    {
      uint16_t base = (uint16_t)(1u + ((uint16_t)i * NODE_STATUS_SIZE));
      int32_t lon = 0;
      int32_t lat = 0;

      s_neighborNodeCache[i].id = payload[base + 0u];
      s_neighborNodeCache[i].ip = ((uint32_t)payload[base + 1u] << 24) |
                                  ((uint32_t)payload[base + 2u] << 16) |
                                  ((uint32_t)payload[base + 3u] << 8) |
                                  ((uint32_t)payload[base + 4u]);
      s_neighborNodeCache[i].hops = payload[base + 5u];
      s_neighborNodeCache[i].rssi = (int8_t)payload[base + 6u];
      s_neighborNodeCache[i].delay = read_le_u16(&payload[base + 7u]);
      s_neighborNodeCache[i].alt = read_le_u16(&payload[base + 29u]);

      if (parse_ascii_coord_1e6(&payload[base + 9u], 10u, &lon))
      {
        s_neighborNodeCache[i].lon = lon;
      }
      else
      {
        s_neighborNodeCache[i].lon = 0;
      }

      if (parse_ascii_coord_1e6(&payload[base + 19u], 10u, &lat))
      {
        s_neighborNodeCache[i].lat = lat;
      }
      else
      {
        s_neighborNodeCache[i].lat = 0;
      }
    }

    if (parseCount > 0u)
    {
      cache_write_neighbor_to_params(&s_neighborNodeCache[0]);
    }
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexRelease(gParamMutexHandle);
  }

  APP_Menu_RefreshCurrent();
}

/* 0x0A: 节点详细信息（请求后回包）
 * 与 ui_practice_test::Send_0x0A 对齐：
 * payload = NodeBasicInfo (17 bytes, packed)
 * [0]      member_id
 * [1]      spatial_filter_status
 * [2..5]   ip_address[4]
 * [6]      ch_frequency_hopping
 * [7..10]  ch_working_freq (u32, little-endian)
 * [11]     ch_signal_bandwidth
 * [12]     ch_waveform
 * [13]     ch_power_level
 * [14]     ch_power_attenuation
 * [15]     ch_routing_protocol
 * [16]     ch_access_protocol
 */
static void handle_cmd_0x0A(const uint8_t *payload, uint16_t payloadLen)
{
  if ((payload == NULL) || (payloadLen == 0u))
  {
    return;
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexAcquire(gParamMutexHandle, osWaitForever);
  }

  if (payloadLen >= 1u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_DETAIL_MEMBER_ID, (int32_t)payload[0]);
  }
  if (payloadLen >= 2u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_DETAIL_SPATIAL, (int32_t)payload[1]);
  }
  if (payloadLen >= 6u)
  {
    uint32_t ip = ((uint32_t)payload[2] << 24) |
                  ((uint32_t)payload[3] << 16) |
                  ((uint32_t)payload[4] << 8) |
                  ((uint32_t)payload[5]);
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_DETAIL_IP, (int32_t)ip);
  }
  if (payloadLen >= 7u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_DETAIL_CH1_HOP, (int32_t)payload[6]);
  }
  if (payloadLen >= 11u)
  {
    uint32_t freqVal = read_le_u32(&payload[7]);

    /* 兼容主控口径差异：若收到Hz（明显大于kHz范围）则自动折算为kHz。 */
    if (freqVal > 3000000u)
    {
      freqVal /= 1000u;
    }

    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_DETAIL_CH1_FREQ, (int32_t)freqVal);
  }
  if (payloadLen >= 12u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_DETAIL_CH1_BW, (int32_t)payload[11]);
  }
  if (payloadLen >= 13u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_DETAIL_CH1_WAVE, (int32_t)payload[12]);
  }
  if (payloadLen >= 14u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_DETAIL_CH1_TXPWR, (int32_t)payload[13]);
  }
  if (payloadLen >= 15u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_DETAIL_CH1_TXATTEN, (int32_t)payload[14]);
  }
  if (payloadLen >= 16u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_DETAIL_CH1_ROUTE, (int32_t)payload[15]);
  }
  if (payloadLen >= 17u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_DETAIL_CH1_ACCESS, (int32_t)payload[16]);
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexRelease(gParamMutexHandle);
  }

  APP_Menu_RefreshCurrent();
}

static void handle_cmd_0x04(const uint8_t *payload, uint16_t payloadLen)
{
  bool protectAdapt;

  /* 0x04 payload（兼容两种长度）：
   * [0]      work mode
   * [1]      spatial filter
   * [2]      sync mode
   * [3]      routing protocol
   * [4]      access protocol
   * [5]      hop mode
   * 可选通道1参数扩展：
   * [6..9]   fixed freq   (u32, little-endian)
   * [10..13] adapt freq1  (u32, little-endian)
   * [14..17] adapt freq2  (u32, little-endian)
   * [18..21] adapt freq3  (u32, little-endian)
   * [22..25] adapt freq4  (u32, little-endian)
   * [26]     signal bandwidth
   * [27]     waveform/mcs
   * [28]     tx power
   * [29]     tx attenuation
   */
  if ((payload == NULL) || (payloadLen < 6u))
  {
    return;
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexAcquire(gParamMutexHandle, osWaitForever);
  }

  protectAdapt = (APP_Menu_IsAdaptHopProtectActive() != 0u);

  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_WORK_MODE, (int32_t)payload[0]);
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_SPATIAL_FILTER, (int32_t)payload[1]);
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_SYNC_MODE, (int32_t)payload[2]);
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ROUTING_PROTOCOL, (int32_t)payload[3]);
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ACCESS_PROTOCOL, (int32_t)payload[4]);
  {
    uint8_t reportedHop = payload[5];
    if ((!protectAdapt) || (reportedHop == 1u))
    {
      (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_FREQ_HOP_MODE, (int32_t)reportedHop);
      if (protectAdapt && (reportedHop == 1u))
      {
        APP_Menu_ClearAdaptHopProtect();
        protectAdapt = false;
      }
    }
  }

  if (payloadLen >= 10u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_FIXED_FREQ, (int32_t)read_le_u32(&payload[6]));
  }
  if ((payloadLen >= 14u) && (!protectAdapt))
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ1, (int32_t)read_le_u32(&payload[10]));
  }
  if ((payloadLen >= 18u) && (!protectAdapt))
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ2, (int32_t)read_le_u32(&payload[14]));
  }
  if ((payloadLen >= 22u) && (!protectAdapt))
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ3, (int32_t)read_le_u32(&payload[18]));
  }
  if ((payloadLen >= 26u) && (!protectAdapt))
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ4, (int32_t)read_le_u32(&payload[22]));
  }
  if (payloadLen >= 27u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_SIGNAL_BANDWIDTH, (int32_t)payload[26]);
  }
  if (payloadLen >= 28u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_WAVEFORM_GEAR, (int32_t)payload[27]);
  }
  if (payloadLen >= 29u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_TX_POWER, (int32_t)payload[28]);
  }
  if (payloadLen >= 30u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_TX_POWER_ATTEN, (int32_t)payload[29]);
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexRelease(gParamMutexHandle);
  }

  APP_Menu_RefreshCurrent();
}

void APP_UartRx_OnFrame(uint8_t cmd,
                        uint8_t msgType,
                        const uint8_t *payload,
                        uint16_t payloadLen)
{
  (void)msgType;

  switch (cmd)
  {
    case 0x02u:
      handle_cmd_0x02(msgType, payload, payloadLen);
      break;

    case 0x04u:
      handle_cmd_0x04(payload, payloadLen);
      break;

    case 0x05u:
      handle_cmd_0x05(payload, payloadLen);
      break;

    case 0x06u:
      handle_cmd_0x06(payload, payloadLen);
      break;

    case 0x07u:
      handle_cmd_0x07(payload, payloadLen);
      break;

    case 0x08u:
      handle_cmd_0x08(payload, payloadLen);
      break;

    case 0x09u:
      handle_cmd_0x09(payload, payloadLen);
      break;

    case 0x0Au:
      handle_cmd_0x0A(payload, payloadLen);
      break;

    default:
      /* 其他命令暂不处理，后续可按协议文档逐步扩展解析逻辑 */
      break;
  }
}

