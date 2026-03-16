/**
 * @file app_uart_proto.h
 * @brief UART protocol helper for framing (header/CRC/tail).
 *
 * 本模块只关注“帧格式”本身：
 * - 帧头 0xD5 0x5D
 * - 命令字 / 消息类型
 * - 数据域 (payload)
 * - CRC16(A001，多项式) 计算
 * - 帧尾 0x5D 0xD5
 *
 * 至于 payload 内部如何组织（寄存器地址 + value 等），交由上层按协议.md 构造。
 */

#ifndef APP_UART_PROTO_H
#define APP_UART_PROTO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief 计算 CRC16(A001) 校验。
 *
 * 校验范围：从“命令字”开始，到“数据域”结束（不含帧头/CRC/帧尾）。
 */
uint16_t APP_UartProto_CalcCrc(const uint8_t *data, uint16_t len);

/**
 * @brief 按协议封装一帧数据。
 *
 * @param cmd          命令字/功能码
 * @param msgType      消息类型（0xFF 上报 / 0x00 应答成功 / 0x01 应答失败）
 * @param payload      数据域指针
 * @param payloadLen   数据域长度
 * @param outBuf       输出缓冲区
 * @param outBufSize   输出缓冲区大小
 * @return             实际写入的字节数；为 0 表示 outBuf 不够或参数错误
 */
size_t APP_UartProto_BuildFrame(uint8_t cmd,
                                uint8_t msgType,
                                const uint8_t *payload,
                                uint16_t payloadLen,
                                uint8_t *outBuf,
                                uint16_t outBufSize);

/**
 * @brief 直接通过底层 UART 发送一帧（内部调用 BuildFrame+发送）。
 *
 * 注意：该函数不做重试与队列，只是一个便捷封装。
 */
bool APP_UartProto_SendRaw(uint8_t cmd,
                           uint8_t msgType,
                           const uint8_t *payload,
                           uint16_t payloadLen);

#ifdef __cplusplus
}
#endif

#endif /* APP_UART_PROTO_H */

