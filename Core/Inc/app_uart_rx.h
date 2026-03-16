/**
 * @file app_uart_rx.h
 * @brief UART receive finite state machine for protocol frames.
 *
 * 本模块只负责：
 *  - 从字节流中按协议“寻帧头 → 收完整帧 → CRC 校验 → 交给上层处理”
 *  - 不关心具体命令含义，仅输出 cmd/type/payload。
 *
 * 使用方式（示例）：
 *  1. 系统启动时调用 APP_UartRx_Init()
 *  2. 在串口中断、DMA 空闲中断中，将新收到的字节逐个或批量喂给 APP_UartRx_OnByte()/APP_UartRx_OnBytes()
 *  3. 在本模块内，当发现 CRC 正确且尾部匹配的完整帧时，调用回调 APP_UartRx_OnFrame()
 */

#ifndef APP_UART_RX_H
#define APP_UART_RX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

void APP_UartRx_Init(void);

/**
 * @brief 向接收状态机推送一个字节。
 */
void APP_UartRx_OnByte(uint8_t byte);

/**
 * @brief 向接收状态机推送一批字节（便利函数）。
 */
void APP_UartRx_OnBytes(const uint8_t *data, size_t len);

/**
 * @brief 在中断上下文缓存接收到的字节，不做协议解析。
 */
void APP_UartRx_PushFromISR(const uint8_t *data, size_t len);

/**
 * @brief 在任务上下文处理 ISR 缓冲字节并驱动协议 FSM。
 */
void APP_UartRx_ProcessPending(void);

/**
 * @brief 收到完整、CRC 校验通过的一帧时的回调。
 *
 * 默认提供一个弱实现，用户可在其他 C 文件中重新实现此函数，
 * 用于把 payload 映射到数据字典、菜单刷新等。
 *
 * @param cmd        命令字
 * @param msgType    消息类型
 * @param payload    数据域指针
 * @param payloadLen 数据域长度
 */
void APP_UartRx_OnFrame(uint8_t cmd,
                        uint8_t msgType,
                        const uint8_t *payload,
                        uint16_t payloadLen);

#ifdef __cplusplus
}
#endif

#endif /* APP_UART_RX_H */

