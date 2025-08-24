#ifndef __I2C_H__
#define __I2C_H__

#include "sys/sys.h"

extern I2C_HandleTypeDef hi2c1;

void I2C1_Init(void);
void I2C1_SendByte(uint16_t adr, uint8_t data);
void I2C1_SendBytes(uint16_t adr, uint8_t *sendBuffre, uint8_t len);
void I2C1_ReceiveByte(uint16_t adr, uint8_t *recieveBuffer);
void I2C1_ReceiveBytes(uint16_t adr, uint8_t *recieveBuffer, uint8_t len);

#endif