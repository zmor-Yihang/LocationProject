#ifndef __USART3_H__
#define __USART3_H__

#include "sys/sys.h"
#include "string.h"

extern UART_HandleTypeDef huart3;

void USART3_Init(void);
void USART3_SendData(uint8_t *buf, uint16_t len);
void USART3_ReceiveData(uint8_t *buf, uint16_t len);
uint16_t USART3_ReceiveToIdle(uint8_t *buf, uint16_t len);

#endif
