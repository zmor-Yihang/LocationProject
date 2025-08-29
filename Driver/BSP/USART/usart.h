#ifndef __USART_H__
#define __USART_H__

#include "sys/sys.h"
#include "string.h"

/* 定义句柄 */
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;

/* 接收缓冲区 */
#define RX_BUFFER_SIZE 512

extern uint8_t rxBuffer[RX_BUFFER_SIZE];
extern uint16_t rxSize;
extern uint8_t rxCompleteFlag;

void USART2_Init(void);
void USART2_SendData(uint8_t *data, uint16_t size);
void USART2_SendString(char *str);

#endif
