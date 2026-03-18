#ifndef BSP_KEYS_H
#define BSP_KEYS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "bsp.h"

void BSP_KeysDrv_Init(void);
bool BSP_KeysDrv_GetEvent(KeyEvent *out_event);

#ifdef __cplusplus
}
#endif

#endif /* BSP_KEYS_H */
