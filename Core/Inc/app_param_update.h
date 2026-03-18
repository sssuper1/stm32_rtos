#ifndef APP_PARAM_UPDATE_H
#define APP_PARAM_UPDATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "app_param_dict.h"

/* Queue one parameter update request using current value in dictionary. */
bool APP_ParamUpdate_Request(ParamId_t id);

/* Queue one parameter update request with an explicit value. */
bool APP_ParamUpdate_RequestValue(ParamId_t id, int32_t value);

#ifdef __cplusplus
}
#endif

#endif /* APP_PARAM_UPDATE_H */
