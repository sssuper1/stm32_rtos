#include "bsp.h"

#include "gpio.h"
#include "usart.h"
#include "bsp_oled.h"
#include "bsp_keys.h"
#include "main.h"

void BSP_Init(void)
{
  /* GPIO/USART clocks and modes are configured by CubeMX init functions. */
}

void BSP_Led_Set(BspLedId led, bool on)
{
  (void)led;
  HAL_GPIO_WritePin(LED_HEARTBEAT_GPIO_Port, LED_HEARTBEAT_Pin,
                    on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void BSP_Lcd_Init(void)
{
  BSP_Oled_Init();
}

void BSP_Lcd_PrintLine(uint8_t row, const char *text)
{
  BSP_Oled_PrintLine(row, text);
}

void BSP_Keys_Init(void)
{
  BSP_KeysDrv_Init();
}

bool BSP_Keys_GetEvent(KeyEvent *out_event)
{
  return BSP_KeysDrv_GetEvent(out_event);
}

void BSP_Uart_Init(uint32_t baudrate)
{
  if (huart1.Init.BaudRate != baudrate)
  {
    huart1.Init.BaudRate = baudrate;
    (void)HAL_UART_Init(&huart1);
  }
}

size_t BSP_Uart_Send(const uint8_t *data, size_t len)
{
  if ((data == NULL) || (len == 0u))
  {
    return 0u;
  }

  if (HAL_UART_Transmit(&huart1, (uint8_t *)data, (uint16_t)len, 100u) == HAL_OK)
  {
    return len;
  }

  return 0u;
}
