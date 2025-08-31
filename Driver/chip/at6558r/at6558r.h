#ifndef __AT6558R_H__
#define __AT6558R_H__

#include "user_config.h"
#include "debug/debug.h"
#include "usart/usart.h"
#include "gpio/gpio.h"
#include <stdlib.h>

#define AT6558R_FREQUENCY_1Hz "PCAS02,1000"           /* 频率设置：1Hz */
#define AT6558R_FREQUENCY_2Hz "PCAS02,500"            /* 频率设置：2Hz */
#define AT6558R_FREQUENCY_4Hz "PCAS02,250"            /* 频率设置：4Hz */
#define AT6558R_FREQUENCY_5Hz "PCAS02,200"            /* 频率设置：5Hz */
#define AT6558R_FREQUENCY_10Hz "PCAS02,100"           /* 频率设置：10Hz */
#define AT6558R_MODE_Dual "PCAS04,3"                  /* 模式设置:GPS、北斗双模 */
#define AT6558R_Info_FirmwareVersion "PCAS06,0"       /* 固件版本信息 */
#define AT6558R_Info_SerialNumber "PCAS06,1"          /* 序列号信息 */
#define AT6558R_Info_MultimodeReceiverMode "PCAS06,2" /* 多模接收机模式信息 */
#define AT6558R_Info_CustomerNumber "PCAS06,3"        /* 客户编号信息 */
#define AT6558R_Info_UpgradeCode "PCAS06,5"           /* 升级码信息 */

void AT6558R_Init(void);

void AT6558R_PrintInfo(void);

uint8_t AT6558R_VerifyIntegrityOfGPSData(void);

uint8_t  AT6558R_VerifyValidityOfGPSData(void);

void AT6558R_ExtractGNRMCData(void);

void AT6558R_EnterLowPowerMode(void);

void AT6558R_Wakeup(void);

#endif
