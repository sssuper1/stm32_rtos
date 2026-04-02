#include "app_param_uart_map.h"

#include <stddef.h>

typedef struct
{
  ParamId_t id;
  uint32_t  addr;
  uint8_t   valueLen;
} ParamUartWriteMap_t;

/*
 * Centralized write-address configuration for UART cmd 0x01.
 * If an address is not confirmed yet, keep it as 0x00000000 and it will be ignored.
 */
#define PARAM_ADDR_ROUTING_PROTOCOL 0x12310000u
#define PARAM_ADDR_ACCESS_PROTOCOL  0x12320000u
#define PARAM_ADDR_WORK_MODE        0x11100000u
#define PARAM_ADDR_SPATIAL_FILTER   0x11300000u
#define PARAM_ADDR_SYNC_MODE        0x11600000u
#define PARAM_ADDR_FREQ_HOP_MODE    0x12340000u
#define PARAM_ADDR_FIXED_FREQ       0x12351000u
#define PARAM_ADDR_ADAPT_FREQ1      0x12352000u
#define PARAM_ADDR_ADAPT_FREQ2      0x12353000u
#define PARAM_ADDR_ADAPT_FREQ3      0x12354000u
#define PARAM_ADDR_ADAPT_FREQ4      0x12355000u
#define PARAM_ADDR_SIGNAL_BW        0x12370000u
#define PARAM_ADDR_WAVEFORM_GEAR    0x12382000u
#define PARAM_ADDR_TX_POWER         0x123A2000u
#define PARAM_ADDR_TX_POWER_ATTEN   0x123B0000u
#define PARAM_ADDR_TXRX_OPERATION   0x23900000u

#define PARAM_ADDR_DEVICE_NAME      0x32000000u
#define PARAM_ADDR_NET_BIZ_IP       0x33110000u
#define PARAM_ADDR_NET_BIZ_PORT     0x33120000u
#define PARAM_ADDR_NET_MGMT_IP      0x33210000u
#define PARAM_ADDR_NET_MGMT_PORT    0x33220000u
#define PARAM_ADDR_NET_SENSE_IP     0x33310000u
#define PARAM_ADDR_NET_SENSE_PORT   0x33320000u
#define PARAM_ADDR_UART_BAUD_IDX    0x34100000u
#define PARAM_ADDR_UART_DATABITS    0x34200000u
#define PARAM_ADDR_UART_STOPBITS    0x34300000u
#define PARAM_ADDR_UART_PARITY      0x34400000u
#define PARAM_ADDR_UART_FLOW        0x34500000u
#define PARAM_ADDR_LOC_AUTO_SYNC    0x35000000u
#define PARAM_ADDR_LOC_LON          0x35100000u
#define PARAM_ADDR_LOC_LAT          0x35200000u
#define PARAM_ADDR_LOC_ALT          0x35300000u
#define PARAM_ADDR_TIME_AUTO_SYNC   0x36100000u
#define PARAM_ADDR_TIME_HOUR        0x36210000u
#define PARAM_ADDR_TIME_MINUTE      0x36220000u
#define PARAM_ADDR_TIME_SECOND      0x36230000u

static const ParamUartWriteMap_t s_writeMap[] =
{
  { PARAM_ID_WORK_MODE,         PARAM_ADDR_WORK_MODE, 1u },
  { PARAM_ID_SPATIAL_FILTER,    PARAM_ADDR_SPATIAL_FILTER, 1u },
  { PARAM_ID_SYNC_MODE,         PARAM_ADDR_SYNC_MODE, 1u },
  { PARAM_ID_FREQ_HOP_MODE,     PARAM_ADDR_FREQ_HOP_MODE, 1u },
  { PARAM_ID_ROUTING_PROTOCOL,  PARAM_ADDR_ROUTING_PROTOCOL, 1u },
  { PARAM_ID_ACCESS_PROTOCOL,   PARAM_ADDR_ACCESS_PROTOCOL, 1u },
  { PARAM_ID_SIGNAL_BANDWIDTH,  PARAM_ADDR_SIGNAL_BW, 1u },
  { PARAM_ID_WAVEFORM_GEAR,     PARAM_ADDR_WAVEFORM_GEAR, 1u },
  { PARAM_ID_FIXED_FREQ,        PARAM_ADDR_FIXED_FREQ, 4u },
  { PARAM_ID_ADAPT_FREQ1,       PARAM_ADDR_ADAPT_FREQ1, 4u },
  { PARAM_ID_ADAPT_FREQ2,       PARAM_ADDR_ADAPT_FREQ2, 4u },
  { PARAM_ID_ADAPT_FREQ3,       PARAM_ADDR_ADAPT_FREQ3, 4u },
  { PARAM_ID_ADAPT_FREQ4,       PARAM_ADDR_ADAPT_FREQ4, 4u },
  { PARAM_ID_TX_POWER,          PARAM_ADDR_TX_POWER, 1u },
  { PARAM_ID_TX_POWER_ATTEN,    PARAM_ADDR_TX_POWER_ATTEN, 1u },
  { PARAM_ID_TXRX_OPERATION,    PARAM_ADDR_TXRX_OPERATION, 1u },

  { PARAM_ID_DEVICE_NAME_TOKEN, PARAM_ADDR_DEVICE_NAME, 4u },
  { PARAM_ID_NET_IP_ADDR,       PARAM_ADDR_NET_BIZ_IP, 4u },
  { PARAM_ID_NET_BIZ_PORT,      PARAM_ADDR_NET_BIZ_PORT, 2u },
  { PARAM_ID_NET_MGMT_IP,       PARAM_ADDR_NET_MGMT_IP, 4u },
  { PARAM_ID_NET_MGMT_PORT,     PARAM_ADDR_NET_MGMT_PORT, 2u },
  { PARAM_ID_NET_SENSE_IP,      PARAM_ADDR_NET_SENSE_IP, 4u },
  { PARAM_ID_NET_SENSE_PORT,    PARAM_ADDR_NET_SENSE_PORT, 2u },
  { PARAM_ID_UART_BAUD_IDX,     PARAM_ADDR_UART_BAUD_IDX, 1u },
  { PARAM_ID_UART_DATABITS,     PARAM_ADDR_UART_DATABITS, 1u },
  { PARAM_ID_UART_STOPBITS,     PARAM_ADDR_UART_STOPBITS, 1u },
  { PARAM_ID_UART_PARITY,       PARAM_ADDR_UART_PARITY, 1u },
  { PARAM_ID_UART_FLOW,         PARAM_ADDR_UART_FLOW, 1u },
  { PARAM_ID_LOC_AUTO_SYNC,     PARAM_ADDR_LOC_AUTO_SYNC, 1u },
  { PARAM_ID_GPS_LONGITUDE,     PARAM_ADDR_LOC_LON, 4u },
  { PARAM_ID_GPS_LATITUDE,      PARAM_ADDR_LOC_LAT, 4u },
  { PARAM_ID_GPS_ALTITUDE,      PARAM_ADDR_LOC_ALT, 2u },
  { PARAM_ID_TIME_AUTO_SYNC,    PARAM_ADDR_TIME_AUTO_SYNC, 1u },
  { PARAM_ID_TIME_HOUR,         PARAM_ADDR_TIME_HOUR, 1u },
  { PARAM_ID_TIME_MINUTE,       PARAM_ADDR_TIME_MINUTE, 1u },
  { PARAM_ID_TIME_SECOND,       PARAM_ADDR_TIME_SECOND, 1u }
};

static bool map_find(ParamId_t id, uint32_t *outAddr, uint8_t *outValueLen)
{
  for (uint32_t i = 0u; i < (sizeof(s_writeMap) / sizeof(s_writeMap[0])); ++i)
  {
    if (s_writeMap[i].id == id)
    {
      if ((s_writeMap[i].addr == 0u) ||
          (s_writeMap[i].valueLen == 0u) ||
          (s_writeMap[i].valueLen > 4u))
      {
        return false;
      }

      *outAddr = s_writeMap[i].addr;
      *outValueLen = s_writeMap[i].valueLen;
      return true;
    }
  }

  return false;
}

bool APP_ParamUartMap_BuildWrite(ParamId_t id,
                                 int32_t value,
                                 uint32_t *outAddr,
                                 uint8_t outValueBytes[4],
                                 uint8_t *outValueLen)
{
  uint32_t addr = 0u;
  uint8_t valueLen = 0u;
  uint32_t uval;

  if ((outAddr == NULL) || (outValueBytes == NULL) || (outValueLen == NULL))
  {
    return false;
  }

  if (!map_find(id, &addr, &valueLen))
  {
    return false;
  }

  uval = (uint32_t)value;

  outValueBytes[0] = 0u;
  outValueBytes[1] = 0u;
  outValueBytes[2] = 0u;
  outValueBytes[3] = 0u;

  if (valueLen == 1u)
  {
    outValueBytes[0] = (uint8_t)(uval & 0xFFu);
  }
  else if (valueLen == 2u)
  {
    outValueBytes[0] = (uint8_t)((uval >> 8) & 0xFFu);
    outValueBytes[1] = (uint8_t)(uval & 0xFFu);
  }
  else
  {
    outValueBytes[0] = (uint8_t)((uval >> 24) & 0xFFu);
    outValueBytes[1] = (uint8_t)((uval >> 16) & 0xFFu);
    outValueBytes[2] = (uint8_t)((uval >> 8) & 0xFFu);
    outValueBytes[3] = (uint8_t)(uval & 0xFFu);
  }

  *outAddr = addr;
  *outValueLen = valueLen;
  return true;
}
