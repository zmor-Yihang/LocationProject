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

/* ===== 函数声明 ===== */

/**
 * @brief   初始化LoRa通信模块
 * @details 完成LLCC68芯片的完整初始化流程，包括：
 *          1. 初始化SPI通信接口和GPIO控制引脚
 *          2. 设置芯片工作模式和调制参数
 *          3. 配置发射功率和接收参数
 *          4. 设置中断和同步字
 *          5. 进入接收模式
 * @param   None
 * @retval  None
 * @note    此函数必须在使用其他LoRa相关函数前调用
 * @warning 确保SPI和GPIO引脚的时钟已使能
 */
void LORA_Init(void);

/**
 * @brief   进入LoRa发送模式
 * @details 配置芯片进入发送状态，设置相关GPIO控制引脚：
 *          - 拉低GPIOB1引脚（接收指示）
 *          - 拉高GPIOB2引脚（发送指示）
 *          - 配置发送相关的中断
 * @param   None
 * @retval  uint8_t 操作结果
 *          - 0: 成功进入发送模式
 *          - 1: 设置失败
 * @note    发送完成后建议调用LORA_EnterReceiveMode()恢复接收
 * @warning 发送前确保芯片已正确初始化
 */
uint8_t LORA_EnterSendMode(void);

/**
 * @brief   进入LoRa接收模式
 * @details 配置芯片进入连续接收状态，设置相关参数：
 *          - 拉低GPIOB2引脚（发送指示）
 *          - 拉高GPIOB1引脚（接收指示）
 *          - 配置接收数据包参数和IQ极性
 *          - 启动连续接收
 * @param   None
 * @retval  uint8_t 操作结果
 *          - 0: 成功进入接收模式
 *          - 1: 设置失败
 * @note    这是默认的工作模式，芯片初始化后会自动进入此模式
 * @warning 接收过程中避免频繁切换模式影响性能
 */
uint8_t LORA_EnterReceiveMode(void);

/**
 * @brief   发送LoRa数据
 * @details 通过LoRa芯片发送指定长度的数据：
 *          1. 切换到发送模式
 *          2. 配置数据包参数并发送数据
 *          3. 发送完成后返回接收模式
 * @param   sendDataBuffer 指向要发送数据的缓冲区指针
 * @param   length 要发送的数据字节数 (1-255字节)
 * @retval  uint8_t 发送结果
 *          - 0: 发送成功
 *          - 1: 发送失败
 * @note    发送完成后会自动恢复到接收模式
 * @warning 确保sendDataBuffer指向的内存区域至少有length字节的有效数据
 */
uint8_t LORA_SendData(uint8_t *sendDataBuffer, uint16_t length);

/**
 * @brief   接收LoRa数据
 * @details 处理LoRa芯片的接收中断并获取接收到的数据：
 *          1. 处理中断状态
 *          2. 检查是否有新数据
 *          3. 复制数据到用户缓冲区
 *          4. 清空内部接收缓冲区
 * @param   receiveDataBuffer 指向存储接收数据的缓冲区指针
 * @param   length 指向存储接收数据长度的变量指针
 * @retval  uint8_t 接收结果
 *          - 0: 处理成功（有无数据都返回0）
 *          - 1: 处理失败
 * @note    需要定期调用此函数以处理接收到的数据
 *         接收到的数据会自动添加字符串结束符'\0'
 * @warning 确保receiveDataBuffer有足够空间存储接收的数据（最大255字节）
 */
uint8_t LORA_ReceiveData(uint8_t *receiveDataBuffer, uint16_t *length);



#endif
