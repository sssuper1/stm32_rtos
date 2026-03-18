#include "bsp_keys.h"

#include "main.h"
#include <stdint.h>

#define KEY_ROWS 4u
#define KEY_COLS 4u
#define KEY_DEBOUNCE_MS 20u

typedef struct
{
	GPIO_TypeDef *port;
	uint16_t pin;
} KeyPin;

static const KeyPin s_rowPins[KEY_ROWS] = {
	{KEY_R1_GPIO_Port, KEY_R1_Pin},
	{KEY_R2_GPIO_Port, KEY_R2_Pin},
	{KEY_R3_GPIO_Port, KEY_R3_Pin},
	{KEY_R4_GPIO_Port, KEY_R4_Pin}
};

static const KeyPin s_colPins[KEY_COLS] = {
	{KEY_C1_GPIO_Port, KEY_C1_Pin},
	{KEY_C2_GPIO_Port, KEY_C2_Pin},
	{KEY_C3_GPIO_Port, KEY_C3_Pin},
	{KEY_C4_GPIO_Port, KEY_C4_Pin}
};

/*
 * 4x4 键盘映射：
 * - 左 3 列：数字键区（1~9,*,0,#）
 * - 第 4 列：导航键（UP,DOWN,OK,BACK）
 */
static const KeyCode s_keyMap[KEY_ROWS][KEY_COLS] = {
	{KEY_NUM_1, KEY_NUM_2, KEY_NUM_3, KEY_UP},
	{KEY_NUM_4, KEY_NUM_5, KEY_NUM_6, KEY_DOWN},
	{KEY_NUM_7, KEY_NUM_8, KEY_NUM_9, KEY_OK},
	{KEY_STAR,  KEY_NUM_0, KEY_HASH,  KEY_BACK}
};

static uint16_t s_lastRawMask = 0u;
static uint16_t s_stableMask = 0u;
static uint32_t s_lastChangeTick = 0u;

static void Keys_ConfigGpio(void)
{
	GPIO_InitTypeDef init = {0};

	init.Pin = KEY_R1_Pin | KEY_R2_Pin | KEY_R3_Pin | KEY_R4_Pin;
	init.Mode = GPIO_MODE_INPUT;
	init.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &init);

	init.Pin = KEY_C1_Pin | KEY_C2_Pin | KEY_C3_Pin | KEY_C4_Pin;
	init.Mode = GPIO_MODE_OUTPUT_PP;
	init.Pull = GPIO_NOPULL;
	init.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &init);

	HAL_GPIO_WritePin(GPIOB,
										KEY_C1_Pin | KEY_C2_Pin | KEY_C3_Pin | KEY_C4_Pin,
										GPIO_PIN_RESET);
}

static inline uint8_t key_index(uint8_t row, uint8_t col)
{
	return (uint8_t)(row * KEY_COLS + col);
}

static void Keys_AllColsLow(void)
{
	for (uint8_t col = 0u; col < KEY_COLS; ++col)
	{
		HAL_GPIO_WritePin(s_colPins[col].port, s_colPins[col].pin, GPIO_PIN_RESET);
	}
}

static uint16_t Keys_ScanRawMask(void)
{
	uint16_t mask = 0u;

	for (uint8_t col = 0u; col < KEY_COLS; ++col)
	{
		Keys_AllColsLow();
		HAL_GPIO_WritePin(s_colPins[col].port, s_colPins[col].pin, GPIO_PIN_SET);

		for (volatile uint32_t i = 0u; i < 200u; ++i)
		{
			__NOP();
		}

		for (uint8_t row = 0u; row < KEY_ROWS; ++row)
		{
			if (HAL_GPIO_ReadPin(s_rowPins[row].port, s_rowPins[row].pin) == GPIO_PIN_SET)
			{
				mask |= (uint16_t)(1u << key_index(row, col));
			}
		}
	}

	Keys_AllColsLow();
	return mask;
}

void BSP_KeysDrv_Init(void)
{
	Keys_ConfigGpio();
	s_lastRawMask = 0u;
	s_stableMask = 0u;
	s_lastChangeTick = HAL_GetTick();
}

bool BSP_KeysDrv_GetEvent(KeyEvent *out_event)
{
	if (out_event == NULL)
	{
		return false;
	}

	uint16_t rawMask = Keys_ScanRawMask();
	uint32_t now = HAL_GetTick();

	if (rawMask != s_lastRawMask)
	{
		s_lastRawMask = rawMask;
		s_lastChangeTick = now;
		return false;
	}

	if ((now - s_lastChangeTick) < KEY_DEBOUNCE_MS)
	{
		return false;
	}

	if (rawMask == s_stableMask)
	{
		return false;
	}

	uint16_t changed = (uint16_t)(rawMask ^ s_stableMask);
	s_stableMask = rawMask;

	for (uint8_t row = 0u; row < KEY_ROWS; ++row)
	{
		for (uint8_t col = 0u; col < KEY_COLS; ++col)
		{
			uint16_t bit = (uint16_t)(1u << key_index(row, col));
			if ((changed & bit) != 0u)
			{
				KeyCode code = s_keyMap[row][col];
				if (code == KEY_NONE)
				{
					continue;
				}

				out_event->code = code;
				out_event->pressed = ((rawMask & bit) != 0u);
				return true;
			}
		}
	}

	return false;
}
