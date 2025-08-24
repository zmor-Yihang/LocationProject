#ifndef __SPI_H
#define __SPI_H

#include "sys/sys.h"

extern SPI_HandleTypeDef hspi1;  /* SPI句柄声明 */

void SPI_Init(void);
void SPI_Start(void);
void SPI_Stop(void);
void SPI_TransmitByte(uint8_t transmitData);
void SPI_TransmitBytes(uint8_t *transmitDataBuffer, uint16_t size);
uint8_t SPI_ReceiveByte(void);
void SPI_ReceiveBytes(uint8_t *receiveDataBuffer, uint16_t size);
uint8_t SPI_SwapByte(uint8_t transmitData);
void SPI_SwapBytes(uint8_t *transmitDataBuffer, uint8_t *receiveDataBuffer, uint16_t size);

#endif /* __SPI_H */
