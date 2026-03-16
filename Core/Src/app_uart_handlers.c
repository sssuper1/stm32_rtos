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

/* 根据协议文档，0x08 用于“开机显示参数”等基础配置。
 * 这里先给出一个最小示例：假设 payload[0] 承载“当前工作模式”，
 * 用于刷新数据字典中的 PARAM_ID_WORK_MODE。
 *
 * 后续你可以根据实际的 struct 定义，扩展解析更多字段。
 */
static void handle_cmd_0x08(const uint8_t *payload, uint16_t payloadLen)
{
  if ((payload == NULL) || (payloadLen < 1u))
  {
    return;
  }

  int32_t mode = (int32_t)payload[0]; /* 假定第 0 字节为工作模式 */

  if (gParamMutexHandle != NULL)
  {
    (void)osMutexAcquire(gParamMutexHandle, osWaitForever);
  }

  (void)APP_ParamDict_SetValueUnsafe(PARAM_ID_WORK_MODE, mode);

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

