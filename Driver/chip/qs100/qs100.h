#ifndef __QS100_H__
#define __QS100_H__

#include "stdlib.h"
#include "USART/usart3.h"
#include "GPIO/gpio.h"
#include "Debug/debug.h"

#define IP "112.125.89.8"
#define PORT 47379
#define SEQUENCE 5

void QS100_Init(void);

void QS100_Reset(void);

void QS100_EnterLowPowerMode(void);

void QS100_Wakeup(void);

void QS100_PrintInfo(void);

void QS100_SendCommand(uint8_t *cmd);

void QS100_SendData(uint8_t *data, uint16_t len);


#endif
