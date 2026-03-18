#ifndef BSP_OLED_H
#define BSP_OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void BSP_Oled_Init(void);
void BSP_Oled_PrintLine(uint8_t row, const char *text);

#ifdef __cplusplus
}
#endif

#endif /* BSP_OLED_H */
