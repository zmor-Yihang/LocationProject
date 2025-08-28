/**
 * @file    ds3553.c
 * @brief   DS3553加速度计步器芯片驱动程序
 * @details DS3553是一款超低功耗的加速度计步器芯片，内置计步算法，
 * @note    使用前需要初始化I2C接口和GPIOB5引脚
 */

#include "ds3553.h"

/* 全局变量：当前步数计数值,存储从DS3553芯片读取的步数计数，范围0~16777215 */
uint32_t countOfStep = 0;

/**
 * @brief   启动DS3553芯片通信序列
 * @details 通过拉低GPIOB5引脚来启动与DS3553芯片的通信，
 *          该引脚作为片选信号，低电平有效
 * @param   None
 * @retval  None
 * @note    该函数为静态函数，仅在本文件内部使用
 * @warning 调用此函数前必须确保GPIOB5已正确初始化
 */
static void DS3553_Start(void)
{
    /* 拉低GPIOB5引脚，启动通信 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief   停止DS3553芯片通信序列
 * @details 通过拉高GPIOB5引脚来停止与DS3553芯片的通信，
 *          释放片选信号，使芯片进入空闲状态
 * @param   None
 * @retval  None
 * @note    该函数为静态函数，仅在本文件内部使用
 * @warning 每次通信结束后必须调用此函数以释放总线
 */
static void DS3553_Stop(void)
{
    /* 拉高GPIOB5引脚，停止通信 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
}

/**
 * @brief   初始化DS3553步数计芯片
 * @details 完成DS3553芯片的完整初始化流程，包括：
 *          1. 初始化I2C通信接口
 *          2. 初始化GPIO控制引脚
 *          3. 配置芯片工作模式和参数
 *          4. 验证配置结果
 * @param   None
 * @retval  None
 * @note    此函数必须在使用其他DS3553相关函数前调用
 * @warning 确保I2C1和GPIOB5的时钟已使能
 */
void DS3553_Init(void)
{
    uint8_t set = 0x18;      /* 初始配置值：0x18 = 0001 1000b */

    /* 初始化I2C通信接口 */
    I2C1_Init();
    
    /* 初始化GPIOB5作为片选控制引脚 */
    GPIOB5_Init();

    /* 配置用户设置寄存器 */
    set = 0x18;              /* 恢复默认值 */
    set &= ~(0x01 << 4);     
    set |= (0x01 << 1);      
    
    /* 将配置写入芯片的USER_SET寄存器 */
    DS3553_WriteData(USER_SET, &set, 1);  
}

/**
 * @brief   打印DS3553芯片的基本信息
 * @details 读取并通过调试接口打印芯片的基本信息，用于：
 *          1. 验证芯片通信是否正常
 *          2. 确认芯片型号
 *          3. 检查当前配置状态
 * @param   None
 * @retval  None
 * @note    需要调试输出接口支持，确保DEBUG_Printf函数可用
 * 
 * @par     输出信息包括:
 *          - CHIP_ID: 芯片标识符
 *          - USER_SET: 用户设置寄存器当前值
 */
void DS3553_PrintInfo(void)
{
    uint8_t tempbuffer = 0;

    DEBUG_Printf("--------------------DS3553-------------------\r\n");
    /* 读取并打印芯片ID */
    DS3553_ReadData(CHIP_ID, &tempbuffer, 1);
    DEBUG_Printf("DS3553 CHIP_ID: 0x%02X\r\n", tempbuffer);
    
    /* 读取并打印用户设置寄存器值 */
    DS3553_ReadData(USER_SET, &tempbuffer, 1);
    DEBUG_Printf("DS3553 Init USER_SET: 0x%02X\r\n", tempbuffer);
    DEBUG_Printf("---------------------------------------------\r\n");
}

/**
 * @brief   向DS3553指定寄存器写入数据
 * @details 通过I2C接口向DS3553芯片的指定寄存器地址写入指定长度的数据。
 *          该函数实现完整的I2C写操作序列，包括起始、数据传输和停止。
 * @param   addr 目标寄存器地址 (8位地址)
 * @param   bufferOfSend 指向要写入数据的缓冲区指针
 * @param   len 要写入的数据字节数 (1-255字节)
 * @retval  None
 * @note    写操作完成后会自动添加延时确保数据写入成功
 * @warning 确保bufferOfSend指向的内存区域至少有len字节的有效数据
 */
void DS3553_WriteData(uint8_t addr, uint8_t *bufferOfSend, uint8_t len)
{
    uint8_t tempBuffer[1 + len];  /* 临时缓冲区：寄存器地址+数据 */
    
    /* 组装I2C发送数据包：第一字节为寄存器地址，后续为数据 */
    tempBuffer[0] = addr;
    for (uint8_t i = 0; i < len; i++) 
    {
        tempBuffer[1 + i] = *(bufferOfSend + i);
    }

    /* 启动DS3553通信序列 */
    DS3553_Start();

    /* 等待通信线路稳定 */
    HAL_Delay(10);

    /* 通过I2C发送完整数据包 */
    I2C1_SendBytes(DS3553_ADDW, tempBuffer, 1 + len);

    /* 停止DS3553通信序列 */
    DS3553_Stop();

    /* 等待写操作完成，确保数据已写入芯片内部寄存器 */
    HAL_Delay(15);
}

/**
 * @brief   从DS3553指定寄存器读取数据
 * @details 通过I2C接口从DS3553芯片的指定寄存器地址读取指定长度的数据。
 *          该函数实现完整的I2C读操作序列，包括写寄存器地址和读取数据。
 * @param   addr 目标寄存器地址 (8位地址)
 * @param   bufferOfRead 指向存储读取数据的缓冲区指针
 * @param   len 要读取的数据字节数 (1-255字节)
 * @retval  None
 * @note    读操作完成后会自动添加延时确保操作时序正确
 * @warning 确保bufferOfRead指向的内存区域至少有len字节的可用空间
 */
void DS3553_ReadData(uint8_t addr, uint8_t *bufferOfRead, uint8_t len)
{
    /* 启动DS3553通信序列 */
    DS3553_Start();

    /* 等待通信线路稳定 */
    HAL_Delay(10);

    /* 发送寄存器地址，指定要读取的寄存器 */
    I2C1_SendByte(DS3553_ADDW, addr);  

    /* 从指定寄存器读取数据 */
    I2C1_ReceiveBytes(DS3553_ADDR, bufferOfRead, len);

    /* 停止DS3553通信序列 */
    DS3553_Stop();

    /* 等待读操作完成，确保时序正确 */
    HAL_Delay(15);
}

/**
 * @brief   获取DS3553芯片当前的步数计数值
 * @details 从DS3553芯片读取24位(3字节)的步数计数值，并将其转换为32位无符号整数。
 *          DS3553芯片将步数存储在连续的3个寄存器中：
 *          - STEP_CNT_L (0xC4): 步数低字节
 *          - STEP_CNT_M (0xC5): 步数中字节  
 *          - STEP_CNT_H (0xC6): 步数高字节
 * @param   None
 * @retval  uint32_t 当前步数计数值 (范围: 0 ~ 16777215)
 * @note    读取的步数值会同时更新全局变量countOfStep
 * @warning 芯片复位或掉电后步数计数会清零
 * 
 * @par     数据格式说明:
 *          芯片采用小端序存储，即：
 *          - tempbuffer[0] = 低字节 (STEP_CNT_L)
 *          - tempbuffer[1] = 中字节 (STEP_CNT_M) 
 *          - tempbuffer[2] = 高字节 (STEP_CNT_H)
 *          最终步数 = (高字节<<16) | (中字节<<8) | 低字节
 */
uint32_t DS3553_GetStepCount(void)
{
    uint8_t tempbuffer[3] = {0};  /* 存储3字节步数数据的缓冲区 */

    /* 从STEP_CNT_L开始连续读取3字节步数数据 */
    DS3553_ReadData(STEP_CNT_L, tempbuffer, 3);

    /* 将3字节数据组合成32位步数值（小端序转大端序） */
    countOfStep = (tempbuffer[2] << 16) | (tempbuffer[1] << 8) | (tempbuffer[0]);

    /* 返回当前步数计数值 */
    return countOfStep;
}

/**
 * @brief   复位DS3553芯片的步数计数器
 * @details 通过设置USER_SET寄存器的RST_STEP位(bit2)来清零步数计数器。
 *          复位操作不会影响芯片的其他配置，只是将步数计数器清零。
 * @param   None
 * @retval  None
 * @note    复位后步数计数器将从0开始重新计数
 * @warning 复位操作不可逆，执行后当前步数信息将丢失
 * 
 * @par     复位机制说明:
 *          1. 读取当前USER_SET寄存器的值
 *          2. 保持其他位不变，仅设置bit2为1
 *          3. 写回寄存器，触发步数复位
 *          4. 芯片自动清除bit2，完成复位操作
 */
void DS3553_Reset(void)
{
    uint8_t data = 0;
    
    /* 读取当前用户设置寄存器的值，保持其他配置不变 */
    DS3553_ReadData(USER_SET, &data, 1);
    
    /* 设置RST_STEP位(bit2)为1，触发步数计数器复位 */
    data |= 0x04;  /* 0x04 = 0000 0100b，对应bit2 */
    
    /* 将修改后的配置写回寄存器，执行复位操作 */
    DS3553_WriteData(USER_SET, &data, 1);
}