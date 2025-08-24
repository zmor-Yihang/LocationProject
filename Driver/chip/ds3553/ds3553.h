#ifndef __DS3553_H__
#define __DS3553_H__

#include "i2c/i2c.h"
#include "gpio/gpio.h"
#include "debug/debug.h"

extern I2C_HandleTypeDef hi2c1;

/* DS3553 I2C地址定义 */
#define DS3553_ADDR 0x4F
#define DS3553_ADDW 0x4E

/* DS3553寄存器地址定义 */
#define CHIP_ID 0x01
#define USER_SET 0xC3
#define STEP_CNT_L 0xC4
#define STEP_CNT_M 0xC5
#define STEP_CNT_H 0xC6

extern uint32_t countOfStep;

void DS3553_Init(void);
void DS3553_PrintInfo(void);
void DS3553_WriteData(uint8_t addr, uint8_t *bufferOfSend, uint8_t len);
void DS3553_ReadData(uint8_t addr, uint8_t *bufferOfRead, uint8_t len);
uint32_t DS3553_GetStepCount(void);
void DS3553_Reset(void);

#endif