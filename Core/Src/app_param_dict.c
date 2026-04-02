#include "app_param_dict.h"
#include "cmsis_os2.h"
extern osMutexId_t gParamMutexHandle;
#include <stddef.h>

/**
 * @brief Internal global parameter dictionary.
 *
 * 目前仅填入少量示例参数，证明结构与接口工作正常。
 * 后续可在此表中逐步追加项目文档中的所有参数。
 */
static ParamDef_t s_paramDict[] =
{
  /* 当前工作模式:
   * - 示例取值范围 0~3 (例如：待机/单点/组网/测试)
   * - 1 位整数，无小数
   */
  {
    .id        = PARAM_ID_WORK_MODE,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 1,
    .scale     = 0,
    .value     = 0
  },

  /* 网口 IP 地址（示例）：
   * 这里先用一个 32 位整型存储 IPv4 地址的整体数值，
   * 以后也可以改成四个独立参数：IP1, IP2, IP3, IP4。
   */
  {
    .id        = PARAM_ID_NET_IP_ADDR,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RW,
    .min_value = (-2147483647 - 1),
    .max_value = 2147483647,
    .scale     = 0,
    .value     = 0xC0A80201  /* 默认 192.168.2.1 */
  },

  /* 串口波特率：
   * 支持的范围 1200 ~ 921600，仅作示例。
   */
  {
    .id        = PARAM_ID_UART_BAUDRATE,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RW,
    .min_value = 1200,
    .max_value = 921600,
    .scale     = 0,
    .value     = 115200
  },

  /* 环境温度（只读状态量示例）：
   * 使用 int16 存储，单位 0.1℃，即 251 表示 25.1℃。
   * type 使用 SCALED_1000_I32 也可以，这里示例 int16 语义。
   */
  {
    .id        = PARAM_ID_ENV_TEMPERATURE,
    .type      = PARAM_TYPE_INT16,
    .access    = PARAM_ACCESS_RO,
    .min_value = -400,  /* -40.0 ℃ */
    .max_value =  850,  /*  85.0 ℃ */
    .scale     = 1,
    .value     = 250    /* 25.0 ℃ 示例 */
  },

  /* 路由协议（屏端协议值）：
   * 0 = OLSR, 1 = AODV, 2 = BATMAN。
   */
  {
    .id        = PARAM_ID_ROUTING_PROTOCOL,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 2,
    .scale     = 0,
    .value     = 0
  },

  /* 接入协议：当前仅支持 0=动态TDMA */
  {
    .id        = PARAM_ID_ACCESS_PROTOCOL,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 0,
    .scale     = 0,
    .value     = 0
  },

  {
    .id        = PARAM_ID_DEVICE_ID,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 255,
    .scale     = 0,
    .value     = 0
  },

  /* 0x05 基础信息（状态量，来自主控回包） */
  {
    .id        = PARAM_ID_GPS_LONGITUDE,
    .type      = PARAM_TYPE_INT32,
    .access    = PARAM_ACCESS_RW,
    .min_value = (-2147483647 - 1),
    .max_value = 2147483647,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_GPS_LATITUDE,
    .type      = PARAM_TYPE_INT32,
    .access    = PARAM_ACCESS_RW,
    .min_value = (-2147483647 - 1),
    .max_value = 2147483647,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_GPS_ALTITUDE,
    .type      = PARAM_TYPE_INT16,
    .access    = PARAM_ACCESS_RW,
    .min_value = -32768,
    .max_value = 32767,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_SAT_LOCK,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 1,
    .scale     = 0,
    .value     = 0
  },

  /* 0x06 统计信息 */
  {
    .id        = PARAM_ID_ETH_TX_CNT,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 2147483647,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_ETH_RX_CNT,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 2147483647,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_VOICE_TX_CNT,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 2147483647,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_VOICE_RX_CNT,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 2147483647,
    .scale     = 0,
    .value     = 0
  },

  /* 0x07 自检状态 */
  {
    .id        = PARAM_ID_BATTERY_CAP,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 100,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_FAN_STATE,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 255,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_SELFTEST_STATE,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 255,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_CLOCK_SELECTION,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 255,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_ADC_STATUS,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 255,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_CLOCK_SRC_TEMP,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 255,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_FREQ_WORD_CNT,
    .type      = PARAM_TYPE_UINT16,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 65535,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_COMM_SENSE_ST,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 255,
    .scale     = 0,
    .value     = 0
  },

  /* 0x08 开机显示参数 */
  {
    .id        = PARAM_ID_TIME_HOUR,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 23,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_TIME_MINUTE,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 59,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_TIME_SECOND,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 59,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_NET_JOIN_STATE,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 1,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_WAVEFORM_GEAR,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 7,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_SIGNAL_BANDWIDTH,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 3,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_FREQ_HOP_MODE,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 1,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_FIXED_FREQ,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RW,
    .min_value = 225000,
    .max_value = 2500000,
    .scale     = 3,
    .value     = 225000
  },
  {
    .id        = PARAM_ID_ADAPT_FREQ1,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RW,
    .min_value = 225000,
    .max_value = 2500000,
    .scale     = 3,
    .value     = 225000
  },
  {
    .id        = PARAM_ID_ADAPT_FREQ2,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RW,
    .min_value = 225000,
    .max_value = 2500000,
    .scale     = 3,
    .value     = 225000
  },
  {
    .id        = PARAM_ID_ADAPT_FREQ3,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RW,
    .min_value = 225000,
    .max_value = 2500000,
    .scale     = 3,
    .value     = 225000
  },
  {
    .id        = PARAM_ID_ADAPT_FREQ4,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RW,
    .min_value = 225000,
    .max_value = 2500000,
    .scale     = 3,
    .value     = 225000
  },
  {
    .id        = PARAM_ID_SPATIAL_FILTER,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 1,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_SYNC_MODE,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 1,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_TX_POWER,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 2,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_TX_POWER_ATTEN,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 90,
    .scale     = 0,
    .value     = 0
  },

    {
    .id        = PARAM_ID_MCS,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 7,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_SLOTLEN,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 3,
    .scale     = 0,
    .value     = 3
  },
  /* 0x09 邻居信息 */
  {
    .id        = PARAM_ID_NEIGHBOR_COUNT,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 255,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_NEIGHBOR_RSSI,
    .type      = PARAM_TYPE_INT16,
    .access    = PARAM_ACCESS_RO,
    .min_value = -127,
    .max_value = 127,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_NEIGHBOR_NODE_ID,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 255,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_NEIGHBOR_NODE_IP,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RO,
    .min_value = (-2147483647 - 1),
    .max_value = 2147483647,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_NEIGHBOR_NODE_HOPS,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 255,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_NEIGHBOR_NODE_DELAY,
    .type      = PARAM_TYPE_UINT16,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 65535,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_NEIGHBOR_NODE_LON,
    .type      = PARAM_TYPE_INT32,
    .access    = PARAM_ACCESS_RO,
    .min_value = (-2147483647 - 1),
    .max_value = 2147483647,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_NEIGHBOR_NODE_LAT,
    .type      = PARAM_TYPE_INT32,
    .access    = PARAM_ACCESS_RO,
    .min_value = (-2147483647 - 1),
    .max_value = 2147483647,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_NEIGHBOR_NODE_ALT,
    .type      = PARAM_TYPE_UINT16,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 65535,
    .scale     = 0,
    .value     = 0
  },

  /* 0x0A 节点详细信息 */
  {
    .id        = PARAM_ID_DETAIL_MEMBER_ID,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 255,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_DETAIL_SPATIAL,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 1,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_DETAIL_IP,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RO,
    .min_value = (-2147483647 - 1),
    .max_value = 2147483647,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_DETAIL_CH1_HOP,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 1,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_DETAIL_CH1_FREQ,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 3000000,
    .scale     = 3,
    .value     = 0
  },
  {
    .id        = PARAM_ID_DETAIL_CH1_BW,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 4,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_DETAIL_CH1_WAVE,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 7,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_DETAIL_CH1_TXPWR,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 2,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_DETAIL_CH1_TXATTEN,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 90,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_DETAIL_CH1_ROUTE,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 2,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_DETAIL_CH1_ACCESS,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 0,
    .scale     = 0,
    .value     = 0
  },

  {
    .id        = PARAM_ID_UART_ACK_STATE,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 3,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_UART_ACK_CMD,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RO,
    .min_value = 0,
    .max_value = 255,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_TXRX_OPERATION,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 2,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_DEVICE_NAME_TOKEN,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 2147483647,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_NET_BIZ_PORT,
    .type      = PARAM_TYPE_UINT16,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 65535,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_NET_MGMT_IP,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RW,
    .min_value = (-2147483647 - 1),
    .max_value = 2147483647,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_NET_MGMT_PORT,
    .type      = PARAM_TYPE_UINT16,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 65535,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_NET_SENSE_IP,
    .type      = PARAM_TYPE_UINT32,
    .access    = PARAM_ACCESS_RW,
    .min_value = (-2147483647 - 1),
    .max_value = 2147483647,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_NET_SENSE_PORT,
    .type      = PARAM_TYPE_UINT16,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 65535,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_UART_BAUD_IDX,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 7,
    .scale     = 0,
    .value     = 4
  },
  {
    .id        = PARAM_ID_UART_DATABITS,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 3,
    .scale     = 0,
    .value     = 3
  },
  {
    .id        = PARAM_ID_UART_STOPBITS,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 2,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_UART_PARITY,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 4,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_UART_FLOW,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 5,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_LOC_AUTO_SYNC,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 1,
    .scale     = 0,
    .value     = 0
  },
  {
    .id        = PARAM_ID_TIME_AUTO_SYNC,
    .type      = PARAM_TYPE_UINT8,
    .access    = PARAM_ACCESS_RW,
    .min_value = 0,
    .max_value = 1,
    .scale     = 0,
    .value     = 0
  }
};

static const uint16_t s_paramCount = (uint16_t)(sizeof(s_paramDict) / sizeof(s_paramDict[0]));

const ParamDef_t *APP_ParamDict_GetTable(uint16_t *outCount)
{
  if (outCount != NULL)
  {
    *outCount = s_paramCount;
  }
  return s_paramDict;
}

ParamDef_t *APP_ParamDict_FindById(ParamId_t id)
{
  for (uint16_t i = 0; i < s_paramCount; ++i)
  {
    if (s_paramDict[i].id == id)
    {
      return &s_paramDict[i];
    }
  }
  return NULL;
}

bool APP_ParamDict_GetValue(ParamId_t id, int32_t *outVal)
{
  if (gParamMutexHandle != NULL) osMutexAcquire(gParamMutexHandle, osWaitForever);
  ParamDef_t *param = APP_ParamDict_FindById(id);
  if ((param == NULL) || (outVal == NULL))
  {
    if (gParamMutexHandle != NULL) osMutexRelease(gParamMutexHandle);
    return false;
  }

  *outVal = param->value;
  if (gParamMutexHandle != NULL) osMutexRelease(gParamMutexHandle);
  return true;
}

bool APP_ParamDict_TrySetValue(ParamId_t id, int32_t newVal)
{
  ParamDef_t *param = APP_ParamDict_FindById(id);
  if (param == NULL)
  {
    return false;
  }

  /* 只读参数不允许修改 */
  if (param->access != PARAM_ACCESS_RW)
  {
    return false;
  }

  /* 边界检查 */
  if ((newVal < param->min_value) || (newVal > param->max_value))
  {
    return false;
  }

  param->value = newVal;
  return true;
}

bool APP_ParamDict_SetValueUnsafe(ParamId_t id, int32_t newVal)
{
  ParamDef_t *param = APP_ParamDict_FindById(id);
  if (param == NULL)
  {
    return false;
  }

  if ((newVal < param->min_value) || (newVal > param->max_value))
  {
    return false;
  }

  param->value = newVal;
  return true;
}

