#include "bsp.h"

#include "gpio.h"
#include "usart.h"
#include "i2c.h"
#include "dma.h"
#include "main.h"
#include <string.h>
#include <stdio.h>

/* ===== LED：使用板载心跳灯 ===== */

void BSP_Init(void) {
  /* GPIO/USART/SPI 时钟和模式已经在 MX_GPIO_Init/MX_USART1_UART_Init 等里配置好了 */
}

void BSP_Led_Set(BspLedId led, bool on) {
  (void)led;
  HAL_GPIO_WritePin(LED_HEARTBEAT_GPIO_Port, LED_HEARTBEAT_Pin,
                    on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

/* ===== OLED(0.96")：I2C/SSD1306 + 6x8 字库渲染 ===== */

#define OLED_I2C_ADDR   (0x3Cu << 1)
#define OLED_CTRL_CMD   0x00u
#define OLED_CTRL_DATA  0x40u

#define OLED_WIDTH      128u
#define OLED_PAGES      8u
#define OLED_CHUNK_SIZE 16u

static uint8_t s_oledFront[OLED_PAGES][OLED_WIDTH];
static uint8_t s_oledBack[OLED_PAGES][OLED_WIDTH];
static uint8_t s_oledDirtyMask = 0u;

typedef struct
{
  char ch;
  uint8_t col[6];
} OledFont6x8_t;

static const OledFont6x8_t s_font6x8[] =
{
  {' ', {0x00,0x00,0x00,0x00,0x00,0x00}},
  {'-', {0x08,0x08,0x08,0x08,0x08,0x00}},
  {'_', {0x40,0x40,0x40,0x40,0x40,0x00}},
  {':', {0x00,0x36,0x36,0x00,0x00,0x00}},
  {'.', {0x00,0x60,0x60,0x00,0x00,0x00}},
  {'/', {0x20,0x10,0x08,0x04,0x02,0x00}},
  {'?', {0x02,0x01,0x51,0x09,0x06,0x00}},
  {'0', {0x3E,0x51,0x49,0x45,0x3E,0x00}},
  {'1', {0x00,0x42,0x7F,0x40,0x00,0x00}},
  {'2', {0x42,0x61,0x51,0x49,0x46,0x00}},
  {'3', {0x21,0x41,0x45,0x4B,0x31,0x00}},
  {'4', {0x18,0x14,0x12,0x7F,0x10,0x00}},
  {'5', {0x27,0x45,0x45,0x45,0x39,0x00}},
  {'6', {0x3C,0x4A,0x49,0x49,0x30,0x00}},
  {'7', {0x01,0x71,0x09,0x05,0x03,0x00}},
  {'8', {0x36,0x49,0x49,0x49,0x36,0x00}},
  {'9', {0x06,0x49,0x49,0x29,0x1E,0x00}},
  {'A', {0x7E,0x11,0x11,0x11,0x7E,0x00}},
  {'B', {0x7F,0x49,0x49,0x49,0x36,0x00}},
  {'C', {0x3E,0x41,0x41,0x41,0x22,0x00}},
  {'D', {0x7F,0x41,0x41,0x22,0x1C,0x00}},
  {'E', {0x7F,0x49,0x49,0x49,0x41,0x00}},
  {'F', {0x7F,0x09,0x09,0x09,0x01,0x00}},
  {'G', {0x3E,0x41,0x49,0x49,0x7A,0x00}},
  {'H', {0x7F,0x08,0x08,0x08,0x7F,0x00}},
  {'I', {0x00,0x41,0x7F,0x41,0x00,0x00}},
  {'J', {0x20,0x40,0x41,0x3F,0x01,0x00}},
  {'K', {0x7F,0x08,0x14,0x22,0x41,0x00}},
  {'L', {0x7F,0x40,0x40,0x40,0x40,0x00}},
  {'M', {0x7F,0x02,0x0C,0x02,0x7F,0x00}},
  {'N', {0x7F,0x04,0x08,0x10,0x7F,0x00}},
  {'O', {0x3E,0x41,0x41,0x41,0x3E,0x00}},
  {'P', {0x7F,0x09,0x09,0x09,0x06,0x00}},
  {'Q', {0x3E,0x41,0x51,0x21,0x5E,0x00}},
  {'R', {0x7F,0x09,0x19,0x29,0x46,0x00}},
  {'S', {0x46,0x49,0x49,0x49,0x31,0x00}},
  {'T', {0x01,0x01,0x7F,0x01,0x01,0x00}},
  {'U', {0x3F,0x40,0x40,0x40,0x3F,0x00}},
  {'V', {0x1F,0x20,0x40,0x20,0x1F,0x00}},
  {'W', {0x3F,0x40,0x38,0x40,0x3F,0x00}},
  {'X', {0x63,0x14,0x08,0x14,0x63,0x00}},
  {'Y', {0x07,0x08,0x70,0x08,0x07,0x00}},
  {'Z', {0x61,0x51,0x49,0x45,0x43,0x00}}
};

static void oled_write_cmd(uint8_t cmd)
{
  uint8_t tx[2] = {OLED_CTRL_CMD, cmd};
  (void)HAL_I2C_Master_Transmit(&hi2c2, OLED_I2C_ADDR, tx, 2U, 20U);
}

static void oled_set_cursor(uint8_t page, uint8_t column)
{
  oled_write_cmd((uint8_t)(0xB0u + (page & 0x07u)));
  oled_write_cmd((uint8_t)(0x00u + (column & 0x0Fu)));
  oled_write_cmd((uint8_t)(0x10u + ((column >> 4) & 0x0Fu)));
}

static void oled_write_data(const uint8_t *data, uint16_t len)
{
  uint8_t tx[1U + OLED_CHUNK_SIZE];
  tx[0] = OLED_CTRL_DATA;

  while (len > 0u)
  {
    uint16_t chunk = (len > OLED_CHUNK_SIZE) ? OLED_CHUNK_SIZE : len;
    memcpy(&tx[1], data, chunk);
    (void)HAL_I2C_Master_Transmit(&hi2c2, OLED_I2C_ADDR, tx, (uint16_t)(chunk + 1u), 20U);

    data += chunk;
    len = (uint16_t)(len - chunk);
  }
}

static void oled_flush_page(uint8_t page)
{
  if (page >= OLED_PAGES)
  {
    return;
  }

  oled_set_cursor(page, 0u);
  oled_write_data(s_oledBack[page], OLED_WIDTH);
  memcpy(s_oledFront[page], s_oledBack[page], OLED_WIDTH);
  s_oledDirtyMask = (uint8_t)(s_oledDirtyMask & (uint8_t)~(1u << page));
}

static void oled_clear_page(uint8_t page)
{
  if (page >= OLED_PAGES)
  {
    return;
  }
  memset(s_oledBack[page], 0, OLED_WIDTH);
}

static void oled_flush_all(void)
{
  for (uint8_t p = 0u; p < OLED_PAGES; ++p)
  {
    oled_flush_page(p);
  }
}

static void oled_mark_dirty_if_changed(uint8_t page)
{
  if (page >= OLED_PAGES)
  {
    return;
  }

  if (memcmp(s_oledBack[page], s_oledFront[page], OLED_WIDTH) != 0)
  {
    s_oledDirtyMask = (uint8_t)(s_oledDirtyMask | (uint8_t)(1u << page));
  }
}

static void oled_flush_dirty_pages(void)
{
  for (uint8_t p = 0u; p < OLED_PAGES; ++p)
  {
    if ((s_oledDirtyMask & (uint8_t)(1u << p)) != 0u)
    {
      oled_flush_page(p);
    }
  }
}

static const uint8_t *oled_font6x8_get(char ch)
{
  if ((ch >= 'a') && (ch <= 'z'))
  {
    ch = (char)(ch - 'a' + 'A');
  }

  for (uint32_t i = 0u; i < (sizeof(s_font6x8) / sizeof(s_font6x8[0])); ++i)
  {
    if (s_font6x8[i].ch == ch)
    {
      return s_font6x8[i].col;
    }
  }

  return s_font6x8[6].col; /* '?' */
}

static void oled_basic_init(void)
{
  /* SSD1306 常用初始化序列（128x64） */
  oled_write_cmd(0xAE); /* Display OFF */
  oled_write_cmd(0x20); /* Memory Addressing Mode */
  oled_write_cmd(0x00); /* Horizontal Addressing Mode */
  oled_write_cmd(0xB0); /* Page Start Address */
  oled_write_cmd(0xC8); /* COM Output Scan Direction remapped */
  oled_write_cmd(0x00); /* low column */
  oled_write_cmd(0x10); /* high column */
  oled_write_cmd(0x40); /* start line */
  oled_write_cmd(0x81); /* contrast */
  oled_write_cmd(0x7F);
  oled_write_cmd(0xA1); /* segment remap */
  oled_write_cmd(0xA6); /* normal display */
  oled_write_cmd(0xA8); /* multiplex */
  oled_write_cmd(0x3F);
  oled_write_cmd(0xA4); /* display follows RAM */
  oled_write_cmd(0xD3); /* display offset */
  oled_write_cmd(0x00);
  oled_write_cmd(0xD5); /* osc freq */
  oled_write_cmd(0x80);
  oled_write_cmd(0xD9); /* pre-charge */
  oled_write_cmd(0xF1);
  oled_write_cmd(0xDA); /* com pins */
  oled_write_cmd(0x12);
  oled_write_cmd(0xDB); /* vcomh */
  oled_write_cmd(0x40);
  oled_write_cmd(0x8D); /* charge pump */
  oled_write_cmd(0x14);
  oled_write_cmd(0xAF); /* Display ON */
}

void BSP_Lcd_Init(void) {
  oled_basic_init();
  memset(s_oledFront, 0, sizeof(s_oledFront));
  memset(s_oledBack, 0, sizeof(s_oledBack));
  s_oledDirtyMask = 0xFFu;
  oled_flush_all();
}

void BSP_Lcd_PrintLine(uint8_t row, const char* text) {
  if (row >= OLED_PAGES)
  {
    return;
  }

  if (text == NULL)
  {
    text = "";
  }

  oled_clear_page(row);

  uint8_t col = 0u;
  while ((*text != '\0') && (col <= (OLED_WIDTH - 6u)))
  {
    const uint8_t *glyph = oled_font6x8_get(*text);
    memcpy(&s_oledBack[row][col], glyph, 6u);
    col = (uint8_t)(col + 6u);
    text++;
  }

  oled_mark_dirty_if_changed(row);
  oled_flush_dirty_pages();
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

