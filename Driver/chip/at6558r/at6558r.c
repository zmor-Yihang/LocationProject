/**
 * @file    at6558r.c
 * @brief   AT6558R GNSS定位芯片驱动程序
 * @details AT6558R是一款高性能、低功耗的多模GNSS定位芯片，支持多种卫星导航系统：
 *          - GPS (美国全球定位系统)
 *          - BDS (中国北斗卫星导航系统)
 *          - GLONASS (俄罗斯格洛纳斯系统)
 * @note    使用前需要初始化USART2接口和GPIOB3控制引脚
 */

#include "at6558r.h"

/* AT命令临时缓冲区 */
uint8_t tempBufferOfCmd[32];

/**
 * @brief   计算AT命令的NMEA校验和并格式化命令
 * @details 按照NMEA 0183协议标准计算XOR校验和，并将命令格式化为标准格式。
 *          NMEA命令格式：$命令*校验和\r\n
 *          校验和计算方法：对'$'和'*'之间的所有字符进行XOR运算
 * @param   cmd 指向需要计算校验和的命令字符串的指针
 * @retval  None
 * @note    该函数为静态函数，仅在本文件内部使用
 * @warning 确保cmd指向有效的字符串，且tempBufferOfCmd有足够空间
 */
static void AT6558R_CalculateChecksum(char *cmd)
{
    char *ptr = cmd;          /* 字符串遍历指针 */
    uint8_t checksum = 0;     /* XOR校验和累加器 */
    
    /* 遍历命令字符串中的每个字符，计算XOR校验和 */
    while (*ptr)
    {
        checksum ^= *ptr++;   /* 异或运算并移动指针 */
    }
    
    /* 按照NMEA 0183标准格式化命令：$命令*校验和\r\n */
    sprintf((char *)tempBufferOfCmd, "$%s*%02X\r\n", cmd, checksum);
}

/**
 * @brief   发送AT命令到AT6558R GNSS芯片
 * @details 将AT命令按照NMEA协议格式化并通过UART接口发送给芯片。
 *          发送流程：计算校验和 -> 格式化命令 -> UART发送
 * @param   cmd 指向需要发送的AT命令字符串的指针
 * @retval  None
 * @note    该函数为静态函数，仅在本文件内部使用
 * @warning 确保USART2已正确初始化，且cmd指向有效的命令字符串
 */
static void AT6558R_SendCmd(char *cmd)
{
    /* 计算命令的NMEA校验和并格式化为标准格式 */
    AT6558R_CalculateChecksum(cmd);
    
    /* 通过USART2接口发送格式化后的AT命令 */
    USART2_SendString((char *)tempBufferOfCmd);
}

/**
 * @brief   打印AT6558R GNSS芯片的详细信息
 * @details 通过发送一系列AT查询命令来获取并显示芯片的完整信息，
 *          包括固件版本、硬件序列号、接收器模式、客户编号等。
 *          所有查询结果会通过UART接收并存储在rxBuffer中。
 * @param   None
 * @retval  None
 * @note    执行后需要一定时间等待芯片响应，建议调用后延时500ms以上
 * @warning 确保rxBuffer有足够空间存储所有响应数据
 */
void AT6558R_PrintInfo(void)
{
    /* 发送固件版本查询命令，获取芯片软件版本信息 */
    AT6558R_SendCmd(AT6558R_Info_FirmwareVersion);
    
    /* 发送序列号查询命令，获取芯片唯一标识 */
    AT6558R_SendCmd(AT6558R_Info_SerialNumber);
    
    /* 发送多模接收器模式查询命令，获取支持的卫星系统 */
    AT6558R_SendCmd(AT6558R_Info_MultimodeReceiverMode);
    
    /* 发送客户编号查询命令，获取厂商定制信息 */
    AT6558R_SendCmd(AT6558R_Info_CustomerNumber);
    
    /* 发送升级代码查询命令，获取固件升级相关信息 */
    AT6558R_SendCmd(AT6558R_Info_UpgradeCode);
    
    /* 延时等待AT6558R芯片处理所有查询命令并发送响应数据 */
    /* 延时时间必须足够长，否则会导致数据接收不完整 */
    HAL_Delay(500);
    
    /* 打印接收缓冲区中的所有响应信息 */
    printf("-------------------AT6558R-------------------\r\n");
    printf("%s", rxBuffer);
    printf("---------------------------------------------\r\n");
}

/**
 * @brief   初始化AT6558R GNSS定位模块
 * @details 完成AT6558R模块的完整初始化流程，包括硬件初始化和参数配置：
 *          1. 初始化GPIO控制引脚（启动芯片）
 *          2. 初始化UART通信接口  
 *          3. 配置GNSS数据输出频率
 *          4. 设置多模卫星工作模式
 * @param   None
 * @retval  None
 * @note    此函数必须在使用其他AT6558R相关功能前调用
 * @warning 确保相关的时钟已使能，GPIO和USART2外设可正常工作
 */
void AT6558R_Init(void)
{
    /* 初始化GPIOB3引脚并设置为高电平，启动AT6558R芯片 */
    /* 该引脚控制芯片的电源使能，高电平有效 */
    GPIOB3_Init();
    
    /* 初始化USART2串口通信接口 */
    /* USART2用于发送AT命令和接收GNSS数据 */
    USART2_Init();
    
    /* 配置GNSS数据输出频率为1Hz */
    /* 1Hz表示每秒输出一次完整的定位信息 */
    AT6558R_SendCmd(AT6558R_FREQUENCY_1Hz);
    
    /* 设置GNSS工作模式为双模（GPS + 北斗BDS） */
    /* 双模可以同时接收GPS和北斗卫星信号，提高定位精度和可靠性 */
    AT6558R_SendCmd(AT6558R_MODE_Dual);
}

/**
 * @brief   验证GNSS数据的完整性
 * @details 通过检查接收缓冲区中的关键NMEA句子来验证数据完整性。
 *          该函数主要检查GGA和TXT句子是否都存在：
 *          - GGA句子：包含定位质量、经纬度、高度等核心定位信息
 *          - TXT句子：包含芯片状态、卫星数量等文本信息
 * @param   None
 * @retval  uint8_t 数据完整性状态
 *          @arg 1: 数据完整，包含所需的关键句子
 *          @arg 0: 数据不完整，缺少关键信息
 * @note    建议在处理GNSS数据前调用此函数进行验证
 * @warning 需要确保rxBuffer中包含完整的NMEA数据流
 */
uint8_t AT6558R_VerifyIntegrityOfGPSData(void)
{
    /* 检查接收缓冲区是否同时包含GGA和TXT关键句子 */
    /* GGA句子包含核心定位信息，TXT句子包含状态信息 */
    if(strstr((char *)rxBuffer, "GGA") != NULL && strstr((char *)rxBuffer, "TXT") != NULL)
    {
        return 1;     /* 数据完整，包含必要的NMEA句子 */
    }
    
    return 0;         /* 数据不完整，缺少关键的NMEA句子 */
}
