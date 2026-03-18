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
#define PARAM_ADDR_FREQ_HOP_MODE    0x12340000u
#define PARAM_ADDR_FIXED_FREQ       0x12351000u
#define PARAM_ADDR_ADAPT_FREQ1      0x12352000u
#define PARAM_ADDR_ADAPT_FREQ2      0x12353000u
#define PARAM_ADDR_ADAPT_FREQ3      0x12354000u
#define PARAM_ADDR_ADAPT_FREQ4      0x12355000u
#define PARAM_ADDR_SIGNAL_BW        0x12370000u
#define PARAM_ADDR_WAVEFORM_GEAR    0x12382000u

/* TODO: confirm addresses from Linux-side enum before enabling these fields. */
#define PARAM_ADDR_SPATIAL_FILTER   0x00000000u
#define PARAM_ADDR_SYNC_MODE        0x00000000u
#define PARAM_ADDR_TX_POWER         0x00000000u
#define PARAM_ADDR_TX_POWER_ATTEN   0x00000000u

static const ParamUartWriteMap_t s_writeMap[] =
{
  { PARAM_ID_WORK_MODE,         PARAM_ADDR_FREQ_HOP_MODE, 1u },
  { PARAM_ID_FREQ_HOP_MODE,     PARAM_ADDR_FREQ_HOP_MODE, 1u },
  { PARAM_ID_ROUTING_PROTOCOL,  PARAM_ADDR_ROUTING_PROTOCOL, 1u },
  { PARAM_ID_SIGNAL_BANDWIDTH,  PARAM_ADDR_SIGNAL_BW, 1u },
  { PARAM_ID_WAVEFORM_GEAR,     PARAM_ADDR_WAVEFORM_GEAR, 1u },
  { PARAM_ID_FIXED_FREQ,        PARAM_ADDR_FIXED_FREQ, 4u },
  { PARAM_ID_ADAPT_FREQ1,       PARAM_ADDR_ADAPT_FREQ1, 4u },
  { PARAM_ID_ADAPT_FREQ2,       PARAM_ADDR_ADAPT_FREQ2, 4u },
  { PARAM_ID_ADAPT_FREQ3,       PARAM_ADDR_ADAPT_FREQ3, 4u },
  { PARAM_ID_ADAPT_FREQ4,       PARAM_ADDR_ADAPT_FREQ4, 4u },

  /* Placeholder items: currently disabled by addr=0. */
  { PARAM_ID_SPATIAL_FILTER,    PARAM_ADDR_SPATIAL_FILTER, 1u },
  { PARAM_ID_SYNC_MODE,         PARAM_ADDR_SYNC_MODE, 1u },
  { PARAM_ID_TX_POWER,          PARAM_ADDR_TX_POWER, 1u },
  { PARAM_ID_TX_POWER_ATTEN,    PARAM_ADDR_TX_POWER_ATTEN, 1u }
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
