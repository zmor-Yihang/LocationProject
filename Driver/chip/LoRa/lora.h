/**
 * @file    lora.h
 * @brief   LLCC68 LoRa无线通信芯片驱动程序头文件
 * @details LLCC68是一款低功耗长距离LoRa无线通信芯片，支持LoRa调制
 *          本文件定义了LoRa通信相关的默认参数、函数声明和接口
 * @note    使用前需要初始化SPI接口和相关GPIO引脚
 */

#ifndef __LORA_H__
#define __LORA_H__

#include "driver_llcc68_interface.h"

#include "spi/spi.h"
#include "gpio/gpio.h"

/* ===== LoRa芯片默认配置参数定义 ===== */

/** @brief 停止前导码计时器配置 - 禁用前导码检测时停止计时器功能 */
#define LLCC68_LORA_DEFAULT_STOP_TIMER_ON_PREAMBLE      LLCC68_BOOL_FALSE                 

/** @brief 电源调节器模式配置 - 使用DC-DC和LDO混合模式提高效率 */
#define LLCC68_LORA_DEFAULT_REGULATOR_MODE              LLCC68_REGULATOR_MODE_DC_DC_LDO   

/** @brief PA配置占空比 - 设置功率放大器占空比以实现+17dBm输出功率 */
#define LLCC68_LORA_DEFAULT_PA_CONFIG_DUTY_CYCLE        0x02                              

/** @brief PA配置高功率最大值 - 设置功率放大器最大输出为+17dBm */
#define LLCC68_LORA_DEFAULT_PA_CONFIG_HP_MAX            0x03                              

/** @brief 默认发送功率 - 设置为+17dBm，提供较强的信号覆盖范围 */
#define LLCC68_LORA_DEFAULT_TX_DBM                      17                                

/** @brief 功率爬升时间 - 设置为10微秒，平衡速度和EMC性能 */
#define LLCC68_LORA_DEFAULT_RAMP_TIME                   LLCC68_RAMP_TIME_10US             

/** @brief 扩频因子 - SF9提供良好的距离和数据速率平衡 */
#define LLCC68_LORA_DEFAULT_SF                          LLCC68_LORA_SF_9                  

/** @brief 信号带宽 - 125kHz标准带宽，兼容性好 */
#define LLCC68_LORA_DEFAULT_BANDWIDTH                   LLCC68_LORA_BANDWIDTH_125_KHZ     

/** @brief 编码率 - 4/5编码率提供适当的纠错能力 */
#define LLCC68_LORA_DEFAULT_CR                          LLCC68_LORA_CR_4_5                

/** @brief 低数据率优化 - 禁用，适用于当前配置的数据速率 */
#define LLCC68_LORA_DEFAULT_LOW_DATA_RATE_OPTIMIZE      LLCC68_BOOL_FALSE                 

/** @brief 射频频率 - 480MHz，根据地区法规设置 */
#define LLCC68_LORA_DEFAULT_RF_FREQUENCY                480000000U                        

/** @brief 符号数超时 - 0表示禁用超时，持续接收 */
#define LLCC68_LORA_DEFAULT_SYMB_NUM_TIMEOUT            0                                 

/** @brief 同步字 - 0x3444用于公共网络通信 */
#define LLCC68_LORA_DEFAULT_SYNC_WORD                   0x3444U                           

/** @brief 接收增益 - 0x94提供标准接收灵敏度 */
#define LLCC68_LORA_DEFAULT_RX_GAIN                     0x94                              

/** @brief 过流保护 - 140mA，保护功率放大器 */
#define LLCC68_LORA_DEFAULT_OCP                         0x38                              

/** @brief 前导码长度 - 12个符号，标准配置 */
#define LLCC68_LORA_DEFAULT_PREAMBLE_LENGTH             12                                

/** @brief 数据包头部类型 - 显式头部，包含数据包信息 */
#define LLCC68_LORA_DEFAULT_HEADER                      LLCC68_LORA_HEADER_EXPLICIT       

/** @brief 缓冲区大小 - 255字节最大数据包大小 */
#define LLCC68_LORA_DEFAULT_BUFFER_SIZE                 255                               

/** @brief CRC校验 - 开启CRC校验确保数据完整性 */
#define LLCC68_LORA_DEFAULT_CRC_TYPE                    LLCC68_LORA_CRC_TYPE_ON           

/** @brief IQ极性反转 - 禁用，保持标准极性 */
#define LLCC68_LORA_DEFAULT_INVERT_IQ                   LLCC68_BOOL_FALSE                 

/** @brief CAD检测符号数 - 2个符号用于信道活动检测 */
#define LLCC68_LORA_DEFAULT_CAD_SYMBOL_NUM              LLCC68_LORA_CAD_SYMBOL_NUM_2      

/** @brief CAD检测峰值阈值 - 24，用于判断信道是否忙碌 */
#define LLCC68_LORA_DEFAULT_CAD_DET_PEAK                24                                

/** @brief CAD检测最小值 - 10，噪声门限设置 */
#define LLCC68_LORA_DEFAULT_CAD_DET_MIN                 10                                

/** @brief 启动模式 - 热启动模式，快速响应 */
#define LLCC68_LORA_DEFAULT_START_MODE                  LLCC68_START_MODE_WARM            

/** @brief RTC唤醒功能 - 启用RTC唤醒以降低功耗 */
#define LLCC68_LORA_DEFAULT_RTC_WAKE_UP                 LLCC68_BOOL_TRUE                  


void LORA_Init(void);

uint8_t LORA_EnterSendMode(void);

uint8_t LORA_EnterReceiveMode(void);

uint8_t LORA_SendData(uint8_t *sendDataBuffer, uint16_t length);

uint8_t LORA_ReceiveData(uint8_t *receiveDataBuffer, uint16_t *length);

#endif
