#ifndef APP_PARAM_UART_MAP_H
#define APP_PARAM_UART_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "app_param_dict.h"

/* Build one writable parameter mapping item for UART 0x04 write command. */
bool APP_ParamUartMap_BuildWrite(ParamId_t id,
                                 int32_t value,
                                 uint32_t *outAddr,
                                 uint8_t outValueBytes[4],
                                 uint8_t *outValueLen);

#ifdef __cplusplus
}
#endif

#endif /* APP_PARAM_UART_MAP_H */
