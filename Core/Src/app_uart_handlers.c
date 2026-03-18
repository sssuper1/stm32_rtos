#include "app_uart_rx.h"
#include "app_param_dict.h"
#include "app_menu.h"
#include "cmsis_os.h"

/* 由 freertos.c 中定义 */
extern osMutexId_t gParamMutexHandle;

static uint32_t read_be_u32(const uint8_t *p)
{
  return ((uint32_t)p[0] << 24) |
         ((uint32_t)p[1] << 16) |
         ((uint32_t)p[2] << 8)  |
         ((uint32_t)p[3]);
}

static int16_t read_be_i16(const uint8_t *p)
{
  return (int16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

static int32_t read_be_i32(const uint8_t *p)
{
  return (int32_t)read_be_u32(p);
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
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_FREQ_HOP_MODE, (int32_t)payload[6]);
  }
  if (payloadLen >= 11u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_FIXED_FREQ, (int32_t)read_be_u32(&payload[7]));
  }
  if (payloadLen >= 15u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ1, (int32_t)read_be_u32(&payload[11]));
  }
  if (payloadLen >= 19u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ2, (int32_t)read_be_u32(&payload[15]));
  }
  if (payloadLen >= 23u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ3, (int32_t)read_be_u32(&payload[19]));
  }
  if (payloadLen >= 27u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ADAPT_FREQ4, (int32_t)read_be_u32(&payload[23]));
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

/* 0x05: 设备基础信息（简化映射）
 * payload[0..3]   : IPv4 (u32 big-endian)
 * payload[4..7]   : 经度 (i32)
 * payload[8..11]  : 纬度 (i32)
 * payload[12..13] : 高度 (i16)
 * payload[14]     : 卫星锁定状态
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

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexRelease(gParamMutexHandle);
  }

  APP_Menu_RefreshCurrent();
}

/* 0x06: 流量统计
 * payload[0..3]   : ETH_TX
 * payload[4..7]   : ETH_RX
 * payload[8..11]  : VOICE_TX
 * payload[12..15] : VOICE_RX
 */
static void handle_cmd_0x06(const uint8_t *payload, uint16_t payloadLen)
{
  if ((payload == NULL) || (payloadLen < 16u))
  {
    return;
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexAcquire(gParamMutexHandle, osWaitForever);
  }

  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ETH_TX_CNT, (int32_t)read_be_u32(&payload[0]));
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_ETH_RX_CNT, (int32_t)read_be_u32(&payload[4]));
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_VOICE_TX_CNT, (int32_t)read_be_u32(&payload[8]));
  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_VOICE_RX_CNT, (int32_t)read_be_u32(&payload[12]));

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexRelease(gParamMutexHandle);
  }

  APP_Menu_RefreshCurrent();
}

/* 0x07: 自检状态
 * payload[0]      : battery
 * payload[1..2]   : temperature(0.1C, i16)
 * payload[3]      : fan state
 * payload[4]      : sat lock
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

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexRelease(gParamMutexHandle);
  }

  APP_Menu_RefreshCurrent();
}

/* 0x09: 邻居信息（简化映射）
 * payload[0] : neighbor count
 * payload[1] : first/strongest RSSI (i8)
 */
static void handle_cmd_0x09(const uint8_t *payload, uint16_t payloadLen)
{
  if ((payload == NULL) || (payloadLen < 1u))
  {
    return;
  }

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexAcquire(gParamMutexHandle, osWaitForever);
  }

  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NEIGHBOR_COUNT, (int32_t)payload[0]);
  if (payloadLen >= 2u)
  {
    (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_NEIGHBOR_RSSI, (int32_t)(int8_t)payload[1]);
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

    default:
      /* 其他命令暂不处理，后续可按协议文档逐步扩展解析逻辑 */
      break;
  }
}

