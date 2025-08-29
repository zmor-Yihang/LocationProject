/**
 * @file qs100.c
 * @brief QS100 NB-IoT模块驱动程序实现
 * @details 本文件实现了QS100 NB-IoT模块的驱动程序，提供了初始化、
 *          通信、数据发送等功能。QS100是一款支持NB-IoT通信的模块，
 *          可以实现物联网设备的数据传输功能。
 * @section Dependencies 依赖项
 * - USART3: 用于与QS100模块的串口通信
 * - GPIO: 用于控制模块的唤醒引脚
 * - Debug: 用于调试信息输出
 */
#include "qs100/qs100.h"

/**
 * @brief 临时缓冲区，用于存储AT命令响应数据
 * @details 64字节的缓冲区，用于接收QS100模块返回的AT命令响应。
 *          该缓冲区被多个函数共享使用，在使用前应该清零。
 * @warning 该缓冲区为全局共享，在多线程环境下可能存在竞态条件
 * @note 缓冲区大小限制为64字节，超长响应可能被截断
 */
uint8_t tempBuffer[64] = {0};

/**
 * @brief 检查tempBuffer中是否包含有效的AT命令响应
 * @details 扫描tempBuffer缓冲区，查找"OK"或"ERROR"字符串，
 *          用于判断AT命令是否得到有效响应。
 * @return uint8_t 响应检查结果
 * @retval 1 包含"OK"或"ERROR"字符串，表示收到有效响应
 * @retval 0 不包含有效响应字符串
 * @note 该函数为静态函数，仅供内部使用
 * @warning 函数依赖于tempBuffer的内容，调用前应确保缓冲区包含最新的响应数据
 * @see tempBuffer
 */
static uint8_t QS100_CheckResponse(void)
{
    char *buffer_str = (char *)tempBuffer;

    // 检查是否包含"OK"或"ERROR"响应字符串
    // "OK"表示命令执行成功，"ERROR"表示命令执行失败
    if (strstr(buffer_str, "OK") != NULL || strstr(buffer_str, "ERROR") != NULL)
    {
        return 1;  // 找到有效响应
    }
    return 0;  // 未找到有效响应
}

/**
 * @brief 获取网络附着状态
 * @details 发送AT+CGATT?命令查询模块的网络附着状态。
 *          该命令用于检查模块是否已成功附着到移动网络。
 * @note 该函数为内部使用的静态函数
 * @note 响应格式: +CGATT: <state>，其中state为0表示未附着，1表示已附着
 * @see QS100_SendCommand()
 * @warning 调用后需要检查tempBuffer中的响应内容
 */
static void QS100_GetIP(void)
{
    QS100_SendCommand((uint8_t *)"AT+CGATT?\r\n");
}

/**
 * @brief 创建网络套接字客户端
 * @param socket 指向套接字号的指针，用于返回创建的套接字号
 * @details 发送AT+NSOCR命令创建STREAM类型的套接字，并解析返回的套接字号。
 *          命令格式: AT+NSOCR=STREAM,6,0,1
 *          - STREAM: 协议类型，表示TCP流套接字
 *          - 6: 协议号，表示TCP协议
 *          - 0: 本地端口（0表示自动分配）
 *          - 1: 接收控制标志
 * @param[out] socket 返回创建的套接字号(0或1)，如果创建失败则保持原值
 * @note 该函数为内部使用的静态函数，套接字号通过指针参数返回
 * @note 目前只支持套接字号0和1的解析，其他值不会被处理
 * @warning 如果响应中没有找到NSOCR:0或NSOCR:1，socket值将保持不变
 * @see QS100_SendCommand()
 */
static void QS100_CreateClient(uint8_t *socket)
{
    // 发送创建套接字命令
    QS100_SendCommand("AT+NSOCR=STREAM,6,0,1\r\n");
    char *buffer_str = (char *)tempBuffer;

    // 解析响应中的套接字号
    // 响应格式: NSOCR:<socket_id>
    if (strstr(buffer_str, "NSOCR:0") != NULL)
    {
        *socket = 0;  // 套接字号为0
    }
    else if (strstr(buffer_str, "NSOCR:1") != NULL)
    {
        *socket = 1;  // 套接字号为1
    }
    // 注意: 如果响应中没有找到预期的套接字号，socket值将保持不变
}

/**
 * @brief 关闭网络套接字客户端
 * @param socket 要关闭的套接字号
 * @details 发送AT+NSOCL命令关闭指定的网络套接字连接。
 *          命令格式: AT+NSOCL=<socket>
 * @param[in] socket 要关闭的套接字号(0-6)
 * @note 该函数为内部使用的静态函数
 * @note 成功关闭后模块将返回"OK"响应
 * @see QS100_SendCommand()
 * @warning 传入无效的套接字号可能导致命令执行失败
 */
static void QS100_CloseClient(uint8_t socket)
{
    char cmd[32] = {0};  // 命令缓冲区，32字节足够存储关闭命令
    
    // 组装关闭套接字命令
    sprintf(cmd, "AT+NSOCL=%d\r\n", socket);
    
    // 发送关闭命令
    QS100_SendCommand((uint8_t *)cmd);
}

/**
 * @brief 连接到远程服务器
 * @param socket 用于连接的套接字号
 * @param ip 目标服务器的IP地址字符串
 * @param port 目标服务器的端口号
 * @details 发送AT+NSOCO命令建立到远程服务器的TCP连接。
 *          命令格式: AT+NSOCO=<socket>,<remote_addr>,<remote_port>
 * @param[in] socket 已创建的套接字号
 * @param[in] ip 服务器IP地址字符串，如"112.125.89.8"
 * @param[in] port 服务器端口号，范围1-65535
 * @note 该函数为内部使用的静态函数
 * @note 连接成功后模块将返回"OK"响应
 * @see QS100_SendCommand()
 * @warning 确保在调用前已成功创建套接字
 * @warning IP地址格式必须正确，否则连接将失败
 */
static void QS100_ConnectServer(uint8_t socket, char *ip, uint16_t port)
{
    char cmd[64] = {0};  // 命令缓冲区，64字节足够存储连接命令
    
    // 组装连接服务器命令
    // 格式: AT+NSOCO=<socket>,<remote_addr>,<remote_port>
    sprintf(cmd, "AT+NSOCO=%d,%s,%d\r\n", socket, ip, port);
    
    // 发送连接命令
    QS100_SendCommand((uint8_t *)cmd);
}

/**
 * @brief 通过套接字发送数据到服务器
 * @param socket 用于发送数据的套接字号
 * @param data 指向要发送数据的指针
 * @param len 要发送的数据长度
 * @details 将二进制数据转换为十六进制字符串格式，然后通过AT+NSOSD命令发送。
 *          命令格式: AT+NSOSD=<socket>,<length>,<data>,<rai>,<sequence>
 *          - socket: 套接字号
 *          - length: 数据长度
 *          - data: 十六进制格式的数据
 *          - rai: RAI标志，0x200表示释放辅助信息
 *          - sequence: 序列号，用于标识发送序列
 * @param[in] socket 已连接的套接字号
 * @param[in] data 要发送的二进制数据缓冲区
 * @param[in] len 数据长度，单位为字节
 * @note 该函数为内部使用的静态函数
 * @note 数据会被转换为大写十六进制字符串格式发送
 * @warning 确保cmd缓冲区(512字节)足够存储完整的命令
 * @warning SEQUENCE必须在调用前定义，否则会编译错误
 * @see QS100_SendCommand()
 */
static void QS100_SendTo(uint8_t socket, uint8_t *data, uint16_t len)
{
    // 计算十六进制字符串所需长度（每字节需2个字符+结束符）
    uint16_t hex_len = len * 2 + 1;
    uint8_t hex_data[hex_len];  // 存储十六进制字符串的缓冲区
    
    // 清空缓冲区
    memset(hex_data, 0, hex_len);
    
    // 将二进制数据转换为十六进制字符串
    for (uint16_t i = 0; i < len; i++)
    {
        // 将每个字节转换为2位大写十六进制字符
        sprintf((char *)hex_data + i * 2, "%02X", data[i]);
    }
    
    // 命令缓冲区，512字节应足够存储大部分发送命令
    uint8_t cmd[512] = {0};
    
    // 组装发送数据命令
    // AT+NSOSD=<socket>,<length>,<data>,<rai>,<sequence>
    sprintf((char *)cmd, "AT+NSOSD=%d,%d,%s,0x200,%d\r\n", socket, len, hex_data, SEQUENCE);
    
    // 发送命令到模块
    QS100_SendCommand(cmd);
}

/**
 * @brief QS100模块初始化
 * @details 初始化QS100模块，包括以下步骤：
 *          1. 初始化GPIOB13（用于模块控制）
 *          2. 初始化USART3（用于串口通信）
 *          3. 唤醒模块
 *          4. 复位模块
 *          5. 开启AT命令回显
 * @note 该函数必须在使用QS100模块前调用
 */
void QS100_Init(void)
{
    GPIOB13_Init();
    USART3_Init();
    QS100_SendCommand((uint8_t *)"ATE1\r\n"); /* 打开命令回显 */
}

/**
 * @brief 复位QS100模块
 * @details 发送AT+RB命令复位QS100模块，并等待模块响应
 * @note 复位成功后会通过调试接口打印复位信息
 */
void QS100_Reset(void)
{
    USART3_SendData((uint8_t *)"AT+RB\r\n", strlen("AT+RB\r\n"));
    memset(tempBuffer, 0, sizeof(tempBuffer));
    USART3_ReceiveData(tempBuffer, sizeof(tempBuffer));
    if (strlen((char *)tempBuffer) > 0)
    {
        DEBUG_Printf("------------ QS100 Reset Response -----------\r\n");
        DEBUG_Printf("%s\r\n", tempBuffer);
        DEBUG_Printf("QS100 Reset Successful!\r\n");
        DEBUG_Printf("---------------------------------------------\r\n");
    }
}

/**
 * @brief QS100芯片进入低功耗模式
 * @details 发送AT+FASTOFF=0命令使QS100芯片进入低功耗模式。
 *          在此模式下，芯片通过禁用非必要功能来降低功耗。
 * @note 调用此函数前必须正确初始化芯片
 * @note 从低功耗模式恢复可能需要特定的唤醒命令
 */
void QS100_EnterLowPowerMode(void)
{
    QS100_SendCommand((uint8_t *)"AT+FASTOFF=0\r\n"); // 进入低功耗模式
}

/**
 * @brief 唤醒QS100模块
 * @details 通过控制GPIOB13引脚产生唤醒脉冲信号来唤醒处于休眠状态的QS100模块
 * @note 唤醒序列：高电平5ms → 低电平5ms
 */
void QS100_Wakeup(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_Delay(5);
}

/**
 * @brief 打印QS100模块的详细信息
 * @details 通过发送一系列AT命令获取并打印QS100模块的详细信息，包括：
 *          - AT+CIMI: 获取国际移动用户识别码(IMSI)
 *          - AT+CGSN=0: 获取产品序列号
 *          - AT+CGSN=1: 获取国际移动设备识别码(IMEI)  
 *          - AT+CGSN=2: 获取国际移动设备识别码和软件版本(IMEISV)
 *          - AT+CGSN=3: 获取集成电路卡识别码(ICCID)
 *          - AT+CGMR: 获取固件版本信息
 *          - AT+CGMI: 获取制造商信息
 *          - AT+CGMM: 获取模块型号
 *          - AT+NV=GET,HARDVER: 获取硬件版本
 *          - AT+NV=GET,EXTVER: 获取扩展版本信息
 *          - AT+SWVER: 获取软件版本
 *          - AT+HVER: 获取硬件版本
 *          - AT+NV=GET,PRODUCTVER: 获取产品版本
 *          - AT+NV=GET,VER: 获取综合版本信息
 * @note 该函数主要用于调试和系统信息查询
 * @note 所有查询结果会通过DEBUG_Printf输出到调试接口
 * @see QS100_SendCommand()
 * @see DEBUG_Printf()
 */
void QS100_PrintInfo(void)
{
    DEBUG_Printf("------------------- QS100 -------------------\r\n");

    // 获取SIM卡的国际移动用户识别码(IMSI)
    QS100_SendCommand((uint8_t *)"AT+CIMI\r\n");
    
    // 获取各种设备识别码
    QS100_SendCommand((uint8_t *)"AT+CGSN=0\r\n");  // 产品序列号
    QS100_SendCommand((uint8_t *)"AT+CGSN=1\r\n");  // 国际移动设备识别码(IMEI)
    QS100_SendCommand((uint8_t *)"AT+CGSN=2\r\n");  // IMEI和软件版本(IMEISV)
    QS100_SendCommand((uint8_t *)"AT+CGSN=3\r\n");  // 集成电路卡识别码(ICCID)
    
    // 获取设备基本信息
    QS100_SendCommand((uint8_t *)"AT+CGMR\r\n");    // 固件版本信息
    QS100_SendCommand((uint8_t *)"AT+CGMI\r\n");    // 制造商信息
    QS100_SendCommand((uint8_t *)"AT+CGMM\r\n");    // 模块型号
    
    // 获取非易失性存储器中的版本信息
    QS100_SendCommand((uint8_t *)"AT+NV=GET,HARDVER\r\n");     // 硬件版本
    QS100_SendCommand((uint8_t *)"AT+NV=GET,EXTVER\r\n");      // 扩展版本信息
    
    // 获取软件版本信息（包括设置和查询）
    QS100_SendCommand((uint8_t *)"AT+SWVER=<verval>\r\n");     // 设置软件版本值
    QS100_SendCommand((uint8_t *)"AT+SWVER\r\n");              // 查询软件版本
    
    // 获取硬件版本信息（包括设置和查询）
    QS100_SendCommand((uint8_t *)"AT+HVER=<verval>\r\n");      // 设置硬件版本值
    QS100_SendCommand((uint8_t *)"AT+HVER\r\n");               // 查询硬件版本
    
    // 获取产品相关版本信息
    QS100_SendCommand((uint8_t *)"AT+NV=GET,PRODUCTVER\r\n");  // 产品版本
    QS100_SendCommand((uint8_t *)"AT+NV=GET,VER\r\n");         // 综合版本信息

    DEBUG_Printf("---------------------------------------------\r\n");
}

/**
 * @brief 向QS100模块发送AT命令并处理响应
 * @param cmd 指向要发送的AT命令字符串的指针
 * @details 发送AT命令并等待模块响应，具有重试机制的完整处理流程：
 *          1. 通过USART3发送命令到模块
 *          2. 清空接收缓冲区准备接收响应
 *          3. 使用空闲中断方式接收响应数据到临时缓冲区
 *          4. 检查响应是否包含"OK"或"ERROR"关键字
 *          5. 如果未收到有效响应，使用普通接收方式重试，最多重试5次
 *          6. 收到有效响应后通过调试接口打印响应内容
 * @param[in] cmd 指向null结尾的AT命令字符串，必须包含\r\n结束符
 * @note 函数会自动处理命令的发送和响应接收过程
 * @note 响应内容会被存储在全局变量tempBuffer中
 * @note 函数具有容错机制，会自动重试失败的接收操作
 * @warning 确保传入的命令字符串格式正确且以\r\n结尾
 * @warning 长响应可能被tempBuffer的64字节限制截断
 */
void QS100_SendCommand(uint8_t *cmd)
{
    uint8_t i = 0;  // 重试计数器
    
    // 通过USART3发送AT命令到QS100模块
    USART3_SendData(cmd, strlen((char *)cmd));
    
    // 清空临时缓冲区，准备接收新的响应数据
    memset(tempBuffer, 0, sizeof(tempBuffer));
    
    // 使用空闲中断方式接收数据，这种方式更适合接收不定长的AT响应
    USART3_ReceiveToIdle(tempBuffer, sizeof(tempBuffer));
    
    // 重试机制：最多尝试5次接收有效响应
    while (i < 5)
    {
        // 检查接收到的响应是否包含"OK"或"ERROR"
        if (QS100_CheckResponse() == 1)
        {
            // 收到有效响应，打印到调试接口并退出循环
            DEBUG_Printf("%s\r\n", tempBuffer);
            break;
        }
        else
        {
            // 未收到有效响应，使用普通接收方式重试
            // 这里使用阻塞式接收作为备用方案
            USART3_ReceiveData(tempBuffer, sizeof(tempBuffer));
            i++;  // 增加重试计数
        }
    }
}

/**
 * @brief 通过QS100模块发送数据到远程服务器
 * @param data 指向要发送数据的指针
 * @param len 要发送的数据长度（字节数）
 * @details 通过NB-IoT网络发送数据到预定义服务器的完整流程，包括：
 *          1. 检查网络连接状态（CGATT查询，最多重试10次）
 *          2. 创建网络套接字客户端（NSOCR命令，最多重试10次）
 *          3. 连接到远程服务器（NSOCO命令，使用IP和PORT宏定义）
 *          4. 发送数据（NSOSD命令，转换为十六进制格式）
 *          5. 检查发送状态（SEQUENCE命令）
 *          6. 关闭客户端连接（NSOCL命令）
 * @param[in] data 指向要发送的二进制数据缓冲区
 * @param[in] len 数据长度，单位为字节，建议不超过1KB
 * @note 每个步骤都具有重试机制，最多重试10次
 * @note 函数使用预定义的IP和PORT连接服务器
 * @note 发送状态通过硬编码的缓冲区位置检查（存在风险）
 * @warning 函数中存在多处逻辑错误和潜在风险：
 *          - tempBuffer状态管理混乱
 *          - 硬编码的状态检查位置(tempBuffer[19])
 *          - 缺少错误处理和状态验证
 * @warning 如果任何步骤重试10次后仍失败，函数会继续执行后续步骤
 * @todo 修复tempBuffer状态管理问题
 * @todo 改进错误处理机制
 * @todo 使用更可靠的状态检查方法
 */
void QS100_SendData(uint8_t *data, uint16_t len)
{
    //==================== 第一步：检查网络连接状态 ====================
    // 检查模块是否已连接到移动网络，这是数据传输的前提条件
    uint8_t i = 0;  // 重试计数器，用于各个步骤的重试控制
    
    // 发送网络附着状态查询命令
    QS100_GetIP();
    
    // 重试循环：最多尝试10次检查网络连接状态
    while (i < 10)
    {
        char *pbuffer = (char *)tempBuffer;
        
        // 检查响应中是否包含"1"，表示已附着到网络
        // 注意：这里的检查方式较为简单，可能匹配到其他含"1"的内容
        if (strstr((char *)pbuffer, "1") != NULL)
        {
            DEBUG_Printf("Internet Connected\r\n");
            break;  // 网络连接正常，退出重试循环
        }
        else
        {
            HAL_Delay(1000);  // 等待1秒后重试
            QS100_GetIP();     // 重新查询网络状态
            i++;               // 增加重试计数
        }
        // 注意：如果10次重试后仍未连接网络，函数会继续执行后续步骤（存在风险）
    }

    //==================== 第二步：创建网络套接字 ====================
    // 创建TCP套接字用于与服务器通信
    i = 0;  // 重置重试计数器
    uint8_t socket = 0xff;  // 套接字号，初始化为无效值0xff
    
    // 尝试创建套接字客户端
    QS100_CreateClient(&socket);
    
    // 重试循环：最多尝试10次创建套接字
    while (i < 10)
    {
        // 检查套接字是否创建成功（socket不等于初始值0xff表示成功）
        if (socket == 0xff)
        {
            // 套接字创建失败，等待后重试
            HAL_Delay(1000);
            QS100_CreateClient(&socket);
            i++;
        }
        else
        {
            // 套接字创建成功，打印套接字号
            DEBUG_Printf("Socket is socket %d\r\n", socket);
            break;  // 退出重试循环
        }
        // 注意：如果10次重试后仍未成功创建套接字，socket仍为0xff，
        // 后续操作可能会因无效套接字号而失败
    }
    //==================== 第三步：连接到远程服务器 ====================
    // 使用创建的套接字连接到预定义的服务器地址和端口
    i = 0;  // 重置重试计数器
    
    // 发送连接服务器命令，使用头文件中定义的IP和PORT
    QS100_ConnectServer(socket, IP, PORT);
    
    // 重试循环：最多尝试10次连接服务器
    while (i < 10)
    {
        char *buffer_str = (char *)tempBuffer;

        // 检查连接响应是否包含"OK"，表示连接成功
        if (strstr(buffer_str, "OK") != NULL)
        {
            DEBUG_Printf("Connect Server Successful\r\n");
            break;  // 连接成功，退出重试循环
        }
        else
        {
            // 连接失败，等待后重试
            HAL_Delay(1000);
            QS100_ConnectServer(socket, IP, PORT);
            i++;
        }
        // 注意：如果10次重试后仍连接失败，函数会继续执行数据发送步骤
        // 这可能导致发送失败，应该在此处添加错误处理
    }

    //==================== 第四步：发送数据并检查状态 ====================
    // 将用户数据发送到服务器，并检查发送状态
    i = 0;  // 重置重试计数器
    uint8_t cmd[64] = {0};  // 用于存储状态查询命令的缓冲区
    
    // 发送数据到服务器（数据会被转换为十六进制格式）
    QS100_SendTo(socket, data, len);
    
    // 等待5秒让模块处理发送命令
    // 注意：这个延时可能需要根据实际情况调整
    HAL_Delay(5000);
    
    // 组装序列号查询命令，用于检查数据发送状态
    // 警告：SEQUENCE宏必须在头文件中定义，否则编译错误
    sprintf((char *)cmd, "AT+SEQUENCE=%d,%d\r\n", socket, SEQUENCE);
    QS100_SendCommand(cmd);
    
    // 重试循环：最多尝试10次检查发送状态
    while (i < 10)
    {
        char *buffer_str = (char *)tempBuffer;

        // 检查连接响应是否包含"1"，表示连接成功
        if (strstr(buffer_str, "1") != NULL)
        {
            DEBUG_Printf("Send Data Successful!\r\n");
            break;  // 连接成功，退出重试循环
        }
        
        else
        {
            // 发送可能失败，等待后重新发送
            HAL_Delay(1000);
            QS100_SendTo(socket, data, len);  // 重新发送数据
            HAL_Delay(10000);  // 更长的等待时间，等待数据处理完成
            
            // 重新查询发送状态
            sprintf((char *)cmd, "AT+SEQUENCE=%d,%d\r\n", socket, SEQUENCE);
            QS100_SendCommand(cmd);
            i++;  // 增加重试计数
        }
        // 注意：如果10次重试后仍显示发送失败，函数仍会继续执行关闭操作
    }

    //==================== 第五步：关闭网络套接字连接 ====================
    // 数据发送完成后，关闭套接字连接以释放网络资源
    i = 0;  // 重置重试计数器
    
    // 发送关闭套接字命令
    QS100_CloseClient(socket);
    
    // 重试循环：最多尝试10次关闭套接字
    while (i < 10)
    {
        char *buffer_str = (char *)tempBuffer;

        // 检查关闭响应是否包含"OK"，表示关闭成功
        if (strstr((char *)buffer_str, "OK") != NULL)
        {
            DEBUG_Printf("Close Client Successful\r\n");
            break;  // 关闭成功，退出重试循环
        }
        else
        {
            // 关闭失败，等待后重试
            HAL_Delay(1000);
            QS100_CloseClient(socket);
            i++;
        }
        // 注意：即使关闭失败，函数也会结束执行
        // 未关闭的套接字可能会占用系统资源
    }
    
    HAL_Delay(1000);  // 最后等待1秒，确保关闭客户端完成
}



