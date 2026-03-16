/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_menu.h"
#include "app_param_dict.h"
#include "app_uart_proto.h"
#include "app_uart_rx.h"
#include "bsp.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* Mutex to protect global parameter dictionary */
osMutexId_t gParamMutexHandle;
const osMutexAttr_t gParamMutex_attributes = {
  .name = "ParamDictMutex"
};

/* Message queue for parameter update notifications: HMI -> UART task */
typedef struct
{
  ParamId_t id;
  int32_t   value;
} ParamUpdateMsg_t;

osMessageQueueId_t gParamUpdateQueueHandle;
const osMessageQueueAttr_t gParamUpdateQueue_attributes = {
  .name = "ParamUpdateQueue"
};

/* USER CODE END Variables */
/* Definitions for HMI_Task */
osThreadId_t HMI_TaskHandle;
const osThreadAttr_t HMI_Task_attributes = {
  .name = "HMI_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UART_Task */
osThreadId_t UART_TaskHandle;
const osThreadAttr_t UART_Task_attributes = {
  .name = "UART_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for Logic_Task */
osThreadId_t Logic_TaskHandle;
const osThreadAttr_t Logic_Task_attributes = {
  .name = "Logic_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static MenuKey_t HMI_ReadKey(void);

/* Simple helpers for locking/unlocking parameter dictionary */
static inline void ParamDict_Lock(void)
{
  (void)osMutexAcquire(gParamMutexHandle, osWaitForever);
}

static inline void ParamDict_Unlock(void)
{
  (void)osMutexRelease(gParamMutexHandle);
}

/* Map ParamUpdateMsg to UART write-frame payload and send it (example). */
static void UART_SendParamWrite(const ParamUpdateMsg_t *msg);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  gParamMutexHandle = osMutexNew(&gParamMutex_attributes);
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  gParamUpdateQueueHandle = osMessageQueueNew(
      8,                               /* queue length: can be tuned */
      sizeof(ParamUpdateMsg_t),        /* each item size */
      &gParamUpdateQueue_attributes);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of HMI_Task */
  HMI_TaskHandle = osThreadNew(StartDefaultTask, NULL, &HMI_Task_attributes);

  /* creation of UART_Task */
  UART_TaskHandle = osThreadNew(StartTask02, NULL, &UART_Task_attributes);

  /* creation of Logic_Task */
  Logic_TaskHandle = osThreadNew(StartTask03, NULL, &Logic_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the HMI_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Initialize BSP (LED / LCD / Keys / UART helpers) and menu state machine */
  BSP_Init();
  BSP_Lcd_Init();
  BSP_Keys_Init();
  (void)APP_Menu_Init();

  /* Infinite loop */
  for(;;)
  {
    int32_t workModeBefore = 0;
    (void)APP_ParamDict_GetValue(PARAM_ID_WORK_MODE, &workModeBefore);

    /* 1. Read key input (polling or from buffer) */
    MenuKey_t key = HMI_ReadKey();

    /* 2. Drive menu state machine if there is a key event */
    if (key != MENU_KEY_NONE)
    {
      (void)APP_Menu_HandleKey(key);

      int32_t workModeAfter = workModeBefore;
      if (APP_ParamDict_GetValue(PARAM_ID_WORK_MODE, &workModeAfter) &&
          (workModeAfter != workModeBefore) &&
          (gParamUpdateQueueHandle != NULL))
      {
        ParamUpdateMsg_t txMsg;
        txMsg.id = PARAM_ID_WORK_MODE;
        txMsg.value = workModeAfter;
        (void)osMessageQueuePut(gParamUpdateQueueHandle, &txMsg, 0U, 0U);
      }
    }

    /* 3. Delay to control polling period (e.g. 10ms) */
    osDelay(10);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the UART_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* UART_Comm_Task: wait for parameter update messages and send via UART (skeleton) */
  ParamUpdateMsg_t msg;

  /* Infinite loop */
  for(;;)
  {
    /* Block until there is a new parameter update request from HMI */
    if (osMessageQueueGet(gParamUpdateQueueHandle, &msg, NULL, osWaitForever) == osOK)
    {
      /* Assemble UART frame and transmit according to protocol. */
      UART_SendParamWrite(&msg);
    }
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the Logic_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  for(;;)
  {
    APP_UartRx_ProcessPending();
    osDelay(5);
  }
  /* USER CODE END StartTask03 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/**
  * @brief Simple placeholder for HMI key reading.
  *
  * 后续可在此处实现矩阵键盘扫描或从按键中断队列读取事件，
  * 目前默认不产生按键（返回 MENU_KEY_NONE），仅保证 HMI_Task
  * 已经正确调用菜单状态机接口。
  */
static MenuKey_t HMI_ReadKey(void)
{
  KeyEvent ev;
  if (!BSP_Keys_GetEvent(&ev))
  {
    return MENU_KEY_NONE;
  }

  /* 仅对“按下”事件做处理，松开忽略 */
  if (!ev.pressed)
  {
    return MENU_KEY_NONE;
  }

  switch (ev.code)
  {
    case KEY_UP:   return MENU_KEY_UP;
    case KEY_DOWN: return MENU_KEY_DOWN;
    case KEY_OK:   return MENU_KEY_OK;
    case KEY_BACK: return MENU_KEY_BACK;
    default:       return MENU_KEY_NONE;
  }

  return MENU_KEY_NONE;
}

/* Example: build a "write parameter" command payload based on ParamId,
 * then send it using the generic UART protocol framing helper.
 *
 * Payload layout (see 协议.md):
 *  - length (1B): 1(cmd) + 4(addr) + value_len
 *  - value_addr (4B, big-endian)
 *  - value (1/2/4B)
 *
 * 这里以“路由协议”参数为示例，对应地址 0x12310000。
 */
static void UART_SendParamWrite(const ParamUpdateMsg_t *msg)
{
  if (msg == NULL)
  {
    return;
  }

  uint8_t payload[1U + 4U + 4U]; /* 最大预留 4 字节 value */
  uint16_t payloadLen = 0U;

  uint32_t addr = 0U;
  uint8_t  valueBytes[4] = {0};
  uint8_t  valueLen = 0U;

  switch (msg->id)
  {
    case PARAM_ID_WORK_MODE:
      /* 地址 0x12340000 (PARAM_CH1_FREQ_HOPPING_MODE) */
      addr = 0x12340000u;
      valueBytes[0] = (uint8_t)(msg->value & 0xFF);
      valueLen = 1U;
      break;

    case PARAM_ID_ROUTING_PROTOCOL:
      /* 地址 0x12310000 (PARAM_CH1_ROUTING_PROTOCOL) */
      addr = 0x12310000u;
      valueBytes[0] = (uint8_t)(msg->value & 0xFF);
      valueLen = 1U;
      break;

    default:
      /* 未映射的参数暂不下发 */
      return;
  }

  /* 长度字段: 命令字(1) + 地址(4) + valueLen */
  const uint8_t lengthField = (uint8_t)(1U + 4U + valueLen);

  uint16_t idx = 0U;
  payload[idx++] = lengthField;

  /* 写入 4 字节地址（大端） */
  payload[idx++] = (uint8_t)((addr >> 24) & 0xFFu);
  payload[idx++] = (uint8_t)((addr >> 16) & 0xFFu);
  payload[idx++] = (uint8_t)((addr >> 8)  & 0xFFu);
  payload[idx++] = (uint8_t)(addr & 0xFFu);

  /* 写入 value */
  for (uint8_t i = 0U; i < valueLen; ++i)
  {
    payload[idx++] = valueBytes[i];
  }

  payloadLen = idx;

  /* 命令字选择：根据协议设计，这里假设 0x04 为“写寄存器”命令，
   * 消息类型使用 0xFF 表示主动请求。具体值可根据实际 enum 定义调整。
   */
  (void)APP_UartProto_SendRaw(0x04u, 0xFFu, payload, payloadLen);
}

/* USER CODE END Application */

