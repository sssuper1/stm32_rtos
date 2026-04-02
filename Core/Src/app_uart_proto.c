#include "app_uart_proto.h"
#include "bsp.h"

/* 协议固定头尾 */
#define UART_FRAME_HEAD_H  0xD5u
#define UART_FRAME_HEAD_L  0x5Du
#define UART_FRAME_TAIL_H  0x5Du
#define UART_FRAME_TAIL_L  0xD5u

uint16_t APP_UartProto_CalcCrc(const uint8_t *data, uint16_t len)
{
  uint16_t crc = 0xFFFFu;

  if ((data == NULL) || (len == 0U))
  {
    return crc;
  }

  for (uint16_t i = 0U; i < len; ++i)
  {
    crc ^= (uint16_t)data[i];
    for (uint8_t j = 0U; j < 8U; ++j)
    {
      if ((crc & 0x0001u) != 0U)
      {
        crc = (uint16_t)((crc >> 1) ^ 0xA001u);
      }
      else
      {
        crc >>= 1;
      }
    }
  }

  /* 与主控板 CRC_Check 保持一致：返回前交换高低字节。 */
  crc = (uint16_t)((crc >> 8) | (crc << 8));

  return crc;
}

size_t APP_UartProto_BuildFrame(uint8_t cmd,
                                uint8_t msgType,
                                const uint8_t *payload,
                                uint16_t payloadLen,
                                uint8_t *outBuf,
                                uint16_t outBufSize)
{
  if (outBuf == NULL)
  {
    return 0U;
  }

  /* 2(head) + 1(cmd) + 1(type) + N(payload) + 2(crc) + 2(tail) */
  const uint16_t totalLen = (uint16_t)(2U + 1U + 1U + payloadLen + 2U + 2U);
  if (outBufSize < totalLen)
  {
    return 0U;
  }

  uint16_t idx = 0U;

  /* 帧头 */
  outBuf[idx++] = UART_FRAME_HEAD_H;
  outBuf[idx++] = UART_FRAME_HEAD_L;

  /* 命令字 + 消息类型 */
  outBuf[idx++] = cmd;
  outBuf[idx++] = msgType;

  /* 数据域 */
  if ((payload != NULL) && (payloadLen > 0U))
  {
    for (uint16_t i = 0U; i < payloadLen; ++i)
    {
      outBuf[idx++] = payload[i];
    }
  }

  /* CRC 计算范围：从 cmd 到数据域最后一个字节 */
  uint16_t crc = APP_UartProto_CalcCrc(&outBuf[2], (uint16_t)(2U + payloadLen));

  /* 协议采用大端: 高字节在前 */
  outBuf[idx++] = (uint8_t)((crc >> 8) & 0xFFu);
  outBuf[idx++] = (uint8_t)(crc & 0xFFu);

  /* 帧尾 */
  outBuf[idx++] = UART_FRAME_TAIL_H;
  outBuf[idx++] = UART_FRAME_TAIL_L;

  return idx;
}

bool APP_UartProto_SendRaw(uint8_t cmd,
                           uint8_t msgType,
                           const uint8_t *payload,
                           uint16_t payloadLen)
{
  /* 临时栈缓冲区：根据协议，典型控制帧长度一般较短 */
  uint8_t buf[128];
  size_t len = APP_UartProto_BuildFrame(cmd, msgType, payload, payloadLen, buf, (uint16_t)sizeof(buf));
  if (len == 0U)
  {
    return false;
  }

  return (BSP_Uart_Send(buf, len) == len);
}

