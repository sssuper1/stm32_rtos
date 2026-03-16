#ifndef BSP_H
#define BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum
{
  BSP_LED_HEARTBEAT = 0
} BspLedId;

typedef enum
{
  KEY_NONE = 0,
  KEY_UP,
  KEY_DOWN,
  KEY_OK,
  KEY_BACK
} KeyCode;

typedef struct
{
  KeyCode code;
  bool    pressed;
} KeyEvent;

void BSP_Init(void);
void BSP_Led_Set(BspLedId led, bool on);

/* 为兼容现有菜单模块，保留 Lcd 命名；底层实现为 OLED。 */
void BSP_Lcd_Init(void);
void BSP_Lcd_PrintLine(uint8_t row, const char *text);

void BSP_Keys_Init(void);
bool BSP_Keys_GetEvent(KeyEvent *out_event);

void BSP_Uart_Init(uint32_t baudrate);
size_t BSP_Uart_Send(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* BSP_H */
