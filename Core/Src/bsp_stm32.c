#include "bsp.h"

#include "gpio.h"
#include "usart.h"
#include "spi.h"
#include "dma.h"
#include "main.h"
#include <string.h>
#include <stdio.h>

/* ===== LED：用 LCD_BLK 当心跳灯示例，你可以换成自己想用的引脚 ===== */

void BSP_Init(void) {
  /* GPIO/USART/SPI 时钟和模式已经在 MX_GPIO_Init/MX_USART1_UART_Init 等里配置好了 */
}

void BSP_Led_Set(BspLedId led, bool on) {
  (void)led;
  /* 这里临时用 LCD_BLK_Pin 作为心跳灯：高电平点亮 */
  HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* ===== LCD：这里先用串口打印占位，后面你可以换成真正的 LCD 驱动 ===== */

void BSP_Lcd_Init(void) {
  /* 如果你已经有基于 SPI1 的 LCD 驱动，可以在这里调用 LCD_Init()/LCD_Clear() 等 */
}

void BSP_Lcd_PrintLine(uint8_t row, const char* text) {
  if (!text) text = "";
  char buf[96];
  int n = snprintf(buf, sizeof(buf), "[ROW %u] %s\r\n", (unsigned)row, text);
  (void)n;
  HAL_UART_Transmit(&huart1, (uint8_t*)buf, (uint16_t)strlen(buf), 100);
}

/* ===== 键盘：利用 KEY_Rx / KEY_Cx 引脚做 4x4 矩阵扫描示例 ===== */

static bool s_last_pressed[4] = {false, false, false, false};

static KeyCode idx_to_keycode(int idx) {
  switch (idx) {
    case 0: return KEY_UP;
    case 1: return KEY_DOWN;
    case 2: return KEY_OK;
    case 3: return KEY_BACK;
    default: return KEY_NONE;
  }
}

void BSP_Keys_Init(void) {
  memset(s_last_pressed, 0, sizeof(s_last_pressed));
}

/* 非严格消抖，仅示意：每次调用最多返回一个按/松事件 */
bool BSP_Keys_GetEvent(KeyEvent* out_event) {
  if (!out_event) return false;

  /* 例：简单使用 4 行（KEY_R1~R4）代表 4 个按键，列全部拉低 */
  HAL_GPIO_WritePin(GPIOB, KEY_C1_Pin | KEY_C2_Pin | KEY_C3_Pin | KEY_C4_Pin, GPIO_PIN_RESET);

  GPIO_PinState row_state[4];
  row_state[0] = HAL_GPIO_ReadPin(GPIOB, KEY_R1_Pin);
  row_state[1] = HAL_GPIO_ReadPin(GPIOB, KEY_R2_Pin);
  row_state[2] = HAL_GPIO_ReadPin(GPIOB, KEY_R3_Pin);
  row_state[3] = HAL_GPIO_ReadPin(GPIOB, KEY_R4_Pin);

  for (int i = 0; i < 4; ++i) {
    bool pressed = (row_state[i] == GPIO_PIN_SET);  // 这里假设高电平为按下
    if (pressed != s_last_pressed[i]) {
      s_last_pressed[i] = pressed;
      out_event->code = idx_to_keycode(i);
      out_event->pressed = pressed;
      return true;
    }
  }
  return false;
}

/* ===== UART1：用 CubeMX 已生成的 huart1 ===== */

void BSP_Uart_Init(uint32_t baudrate) {
  if (huart1.Init.BaudRate != baudrate) {
    huart1.Init.BaudRate = baudrate;
    (void)HAL_UART_Init(&huart1);
  }
}

size_t BSP_Uart_Send(const uint8_t* data, size_t len) {
  if (!data || !len) return 0;
  if (HAL_UART_Transmit(&huart1, (uint8_t*)data, (uint16_t)len, 100) == HAL_OK) {
    return len;
  }
  return 0;
}