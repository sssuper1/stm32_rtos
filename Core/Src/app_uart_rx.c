#include "app_uart_rx.h"
#include "app_uart_proto.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdbool.h>

/* 协议固定头尾，与 app_uart_proto 保持一致 */
#define UART_FRAME_HEAD_H  0xD5u
#define UART_FRAME_HEAD_L  0x5Du
#define UART_FRAME_TAIL_H  0x5Du
#define UART_FRAME_TAIL_L  0xD5u

/* 收包缓冲区最大长度：足够容纳常见的状态/控制帧 */
#define UART_RX_BUFFER_MAX 256u

static uint8_t  s_rxBuf[UART_RX_BUFFER_MAX];
static uint16_t s_rxIndex = 0u;

#define UART_ISR_RING_SIZE 512u
static volatile uint16_t s_isrWrite = 0u;
static volatile uint16_t s_isrRead  = 0u;
static uint8_t s_isrRing[UART_ISR_RING_SIZE];

void APP_UartRx_Init(void)
{
  s_rxIndex = 0u;
  s_isrWrite = 0u;
  s_isrRead = 0u;
}

static void uart_rx_reset(void)
{
  s_rxIndex = 0u;
}

void APP_UartRx_OnByte(uint8_t byte)
{
  /* 简单状态机：始终维护一个缓冲区，以“帧头+任意内容+帧尾”的方式寻找完整帧 */

  if (s_rxIndex == 0u)
  {
    /* 只接受 0xD5 作为可能的帧头起始 */
    if (byte == UART_FRAME_HEAD_H)
    {
      s_rxBuf[s_rxIndex++] = byte;
    }
    return;
  }
  else if (s_rxIndex == 1u)
  {
    /* 第二个字节必须是 0x5D，否则视作新的起点尝试 */
    if (byte == UART_FRAME_HEAD_L)
    {
      s_rxBuf[s_rxIndex++] = byte;
    }
    else
    {
      /* 若此字节本身是 0xD5，则可以作为新的起点 */
      s_rxIndex = 0u;
      if (byte == UART_FRAME_HEAD_H)
      {
        s_rxBuf[s_rxIndex++] = byte;
      }
    }
    return;
  }

  /* 已有帧头，继续累积 */
  if (s_rxIndex >= UART_RX_BUFFER_MAX)
  {
    /* 缓冲区溢出，丢弃当前帧 */
    uart_rx_reset();
    return;
  }

  s_rxBuf[s_rxIndex++] = byte;

  /* 最小合法帧长度：2(head) + 1(cmd) + 1(type) + 0(payload) + 2(crc) + 2(tail) = 8 */
  if (s_rxIndex < 8u)
  {
    return;
  }

  /* 检查末尾是否为帧尾 */
  if ((s_rxBuf[s_rxIndex - 2u] == UART_FRAME_TAIL_H) &&
      (s_rxBuf[s_rxIndex - 1u] == UART_FRAME_TAIL_L))
  {
    /* 解析一帧 */
    const uint16_t totalLen = s_rxIndex;

    const uint8_t  cmd     = s_rxBuf[2u];
    const uint8_t  msgType = s_rxBuf[3u];

    /* payload 起始索引/长度 */
    const uint16_t headerLen  = 2u /* head */ + 1u /* cmd */ + 1u /* type */;
    const uint16_t tailLen    = 2u;
    const uint16_t crcLen     = 2u;
    const uint16_t payloadLen = (uint16_t)(totalLen - headerLen - crcLen - tailLen);

    if ((int16_t)payloadLen < 0)
    {
      uart_rx_reset();
      return;
    }

    const uint8_t *payload = &s_rxBuf[headerLen];

    /* CRC frame index: 紧跟在 payload 之后的两个字节 (高字节在前) */
    const uint16_t crcIndex = (uint16_t)(headerLen + payloadLen);
    uint16_t crcFrame = (uint16_t)(((uint16_t)s_rxBuf[crcIndex] << 8) |
                                   (uint16_t)s_rxBuf[crcIndex + 1u]);

    /* 计算 CRC */
    uint16_t crcCalc = APP_UartProto_CalcCrc(&s_rxBuf[2u],
                                             (uint16_t)(2u + payloadLen)); /* cmd+type+payload */

    if (crcCalc == crcFrame)
    {
      /* CRC 正确，调用上层回调 */
      APP_UartRx_OnFrame(cmd, msgType, payload, payloadLen);
    }

    /* 无论校验成功与否，重置，开始寻找下一帧 */
    uart_rx_reset();
  }
}

void APP_UartRx_OnBytes(const uint8_t *data, size_t len)
{
  if ((data == NULL) || (len == 0u))
  {
    return;
  }

  for (size_t i = 0u; i < len; ++i)
  {
    APP_UartRx_OnByte(data[i]);
  }
}

void APP_UartRx_PushFromISR(const uint8_t *data, size_t len)
{
  if ((data == NULL) || (len == 0u))
  {
    return;
  }

  for (size_t i = 0u; i < len; ++i)
  {
    uint16_t next = (uint16_t)((s_isrWrite + 1u) % UART_ISR_RING_SIZE);
    if (next == s_isrRead)
    {
      /* ring full: drop oldest byte */
      s_isrRead = (uint16_t)((s_isrRead + 1u) % UART_ISR_RING_SIZE);
    }
    s_isrRing[s_isrWrite] = data[i];
    s_isrWrite = next;
  }
}

void APP_UartRx_ProcessPending(void)
{
  for (;;)
  {
    uint8_t byte;

    taskENTER_CRITICAL();
    bool hasData = (s_isrRead != s_isrWrite);
    if (hasData)
    {
      byte = s_isrRing[s_isrRead];
      s_isrRead = (uint16_t)((s_isrRead + 1u) % UART_ISR_RING_SIZE);
    }
    taskEXIT_CRITICAL();

    if (!hasData)
    {
      break;
    }

    APP_UartRx_OnByte(byte);
  }
}

/* 默认的弱实现：用户可在其他模块中重新实现此函数 */
__attribute__((weak)) void APP_UartRx_OnFrame(uint8_t cmd,
                                              uint8_t msgType,
                                              const uint8_t *payload,
                                              uint16_t payloadLen)
{
  (void)cmd;
  (void)msgType;
  (void)payload;
  (void)payloadLen;
}

