/**
 * @file qs100.c
 * @brief QS100 NB-IoT模块驱动程序实现
 * @details 本文件实现了QS100 NB-IoT模块的驱动程序，提供了初始化、
 * 通信、数据发送等功能。QS100是一款支持NB-IoT通信的模块，
 * 可以实现物联网设备的数据传输功能。
 */

#include "qs100/qs100.h"

/**
 * @brief 临时缓冲区，用于存储AT命令响应数据
 * @details 64字节的缓冲区，用于接收QS100模块返回的AT命令响应
 */
uint8_t tempBuffer[64] = {0};

/**
 * @brief 检查tempBuffer中是否含有"ok"或"error"字符串
 * @return 1: 含有, 0: 都不含有
 */
static uint8_t QS100_CheckResponse(void)
{
    char *buffer_str = (char *)tempBuffer;

    // 检查是否包含"ok"或"error"
    if (strstr(buffer_str, "OK") != NULL || strstr(buffer_str, "ERROR") != NULL)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief 获取网络附着状态
 * @details 发送AT+CGATT?命令查询模块的网络附着状态
 * @note 该函数为内部使用的静态函数
 */
static void QS100_GetIP(void)
{
    QS100_SendCommand((uint8_t *)"AT+CGATT?\r\n");
}

/**
 * @brief 创建网络套接字客户端
 * @param socket 指向套接字号的指针，用于返回创建的套接字号
 * @details 发送AT+NSOCR命令创建STREAM类型的套接字，并解析返回的套接字号
 * @note 该函数为内部使用的静态函数，套接字号通过指针参数返回
 */
static void QS100_CreateClient(uint8_t *socket)
{
    QS100_SendCommand("AT+NSOCR=STREAM,6,0,1\r\n");
    char *buffer_str = (char *)tempBuffer;

    if (strstr(buffer_str, "NSOCR:0") != NULL)
    {
        *socket = 0;
    }
    else if (strstr(buffer_str, "NSOCR:1") != NULL)
    {
        *socket = 1;
    }
}

static void QS100_CloseClient(uint8_t socket)
{
    char cmd[32] = {0};
    sprintf(cmd, "AT+NSOCL=%d\r\n", socket);
    QS100_SendCommand((uint8_t *)cmd);
}

static void QS100_ConnectServer(uint8_t socket, char *ip, uint16_t port)
{
    char cmd[64] = {0};
    sprintf(cmd, "AT+NSOCO=%d,%s,%d\r\n", socket, ip, port);
    QS100_SendCommand((uint8_t *)cmd);
}

static void QS100_SendTo(uint8_t socket, uint8_t *data, uint16_t len)
{
    uint16_t hex_len = len * 2 + 1;
    uint8_t hex_data[hex_len];
    memset(hex_data, 0, hex_len);
    for (uint16_t i = 0; i < len; i++)
    {
        sprintf((char *)hex_data + i * 2, "%02X", data[i]);
    }
    uint8_t cmd[512] = {0};
    sprintf((char *)cmd, "AT+NSOSD=%d,%d,%s,0x200,%d\r\n", socket, len, hex_data, SEQUENCE);
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
    QS100_Wakeup();
    QS100_Reset();
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
 * @brief 打印QS100模块信息
 * @details 通过发送一系列AT命令获取并打印QS100模块的详细信息，包括：
 * @note 该函数主要用于调试和系统信息查询
 */
void QS100_PrintInfo(void)
{
    DEBUG_Printf("------------------- QS100 -------------------\r\n");

    QS100_SendCommand((uint8_t *)"AT+CIMI\r\n");
    QS100_SendCommand((uint8_t *)"AT+CGSN=0\r\n");
    QS100_SendCommand((uint8_t *)"AT+CGSN=1\r\n");
    QS100_SendCommand((uint8_t *)"AT+CGSN=2\r\n");
    QS100_SendCommand((uint8_t *)"AT+CGSN=3\r\n");
    QS100_SendCommand((uint8_t *)"AT+CGMR\r\n");
    QS100_SendCommand((uint8_t *)"AT+CGMI\r\n");
    QS100_SendCommand((uint8_t *)"AT+CGMM\r\n");
    QS100_SendCommand((uint8_t *)"AT+NV=GET,HARDVER\r\n");
    QS100_SendCommand((uint8_t *)"AT+NV=GET,EXTVER\r\n");
    QS100_SendCommand((uint8_t *)"AT+SWVER=<verval>\r\n");
    QS100_SendCommand((uint8_t *)"AT+SWVER\r\n");
    QS100_SendCommand((uint8_t *)"AT+HVER=<verval>\r\n");
    QS100_SendCommand((uint8_t *)"AT+HVER\r\n");
    QS100_SendCommand((uint8_t *)"AT+NV=GET,PRODUCTVER\r\n");
    QS100_SendCommand((uint8_t *)"AT+NV=GET,VER\r\n");

    DEBUG_Printf("---------------------------------------------\r\n");
}

/**
 * @brief 向QS100模块发送AT命令
 * @param cmd 指向要发送的AT命令字符串的指针
 * @details 发送AT命令并等待模块响应，会多次尝试接收有效响应：
 *          1. 发送命令到模块
 *          2. 接收响应数据到临时缓冲区
 *          3. 检查响应是否包含"OK"或"ERROR"
 *          4. 如果未收到有效响应，最多重试5次
 *          5. 收到有效响应后打印到调试接口
 * @note 函数会自动处理命令的发送和响应接收过程
 */
void QS100_SendCommand(uint8_t *cmd)
{
    uint8_t i = 0;
    USART3_SendData(cmd, strlen((char *)cmd));
    memset(tempBuffer, 0, sizeof(tempBuffer));
    USART3_ReceiveToIdle(tempBuffer, sizeof(tempBuffer));
    while (i < 5)
    {
        if (QS100_CheckResponse() == 1)
        {
            DEBUG_Printf("%s\r\n", tempBuffer);
            break;
        }
        else
        {
            USART3_ReceiveData(tempBuffer, sizeof(tempBuffer));
            i++;
        }
    }
}

/**
 * @brief 通过QS100模块发送数据
 * @param data 指向要发送数据的指针
 * @param len 要发送的数据长度
 * @details 通过NB-IoT网络发送数据，执行以下步骤：
 *          1. 检查网络连接状态（最多重试10次）
 *          2. 创建网络套接字客户端（最多重试10次）
 *          3. 连接到服务端（待实现）
 *          4. 进入ES状态（待实现）
 *          5. 发送数据（待实现）
 *          6. 关闭客户端连接（待实现）
 * @note 该函数目前只实现了网络检查和套接字创建部分，
 *       其他功能需要根据具体应用场景实现
 * @warning 函数中的部分功能（连接服务端、发送数据等）尚未实现
 */
void QS100_SendData(uint8_t *data, uint16_t len)
{
    /* 检查是否联网 */
    uint8_t i = 0;
    QS100_GetIP();
    while (i < 10)
    {
        char *pbuffer = (char *)tempBuffer;
        if (strstr((char *)pbuffer, "1") != NULL)
        {
            DEBUG_Printf("Internet Connected\r\n");
            break;
        }
        else
        {
            HAL_Delay(1000);
            QS100_GetIP();
            i++;
        }
    }

    /* 创建客户端 */
    i = 0;
    uint8_t socket = 0xff;
    QS100_CreateClient(&socket);
    while (i < 10)
    {
        if (socket == 0xff)
        {
            HAL_Delay(1000);
            QS100_CreateClient(&socket);
            i++;
        }
        else
        {
            DEBUG_Printf("Socket is socket %d\r\n", socket);
            break;
        }
    }
    /* 连接服务端 */
    i = 0;
    QS100_ConnectServer(socket, IP, PORT);
    while (i < 10)
    {
        char *buffer_str = (char *)tempBuffer;

        if (strstr(buffer_str, "OK") != NULL)
        {
            DEBUG_Printf("Connect Server Successful\r\n");
            break;
        }
        else
        {
            HAL_Delay(1000);
            QS100_ConnectServer(socket, IP, PORT);
            i++;
        }
    }

    /* 发送数据 */
    i = 0;
    uint8_t cmd[64] = {0};
    QS100_SendTo(socket, data, len);
    HAL_Delay(5000);
    sprintf((char *)cmd, "AT+SEQUENCE=%d,%d\r\n", socket, SEQUENCE);
    QS100_SendCommand(cmd);
    while (i < 10)
    {
        char status_char = tempBuffer[19];
        if(status_char == '1')
        {
            DEBUG_Printf("Send Data Successful\r\n");
            break;
        }
        else
        {
            HAL_Delay(1000);
            QS100_SendTo(socket, data, len);
            HAL_Delay(10000);
            sprintf((char *)cmd, "AT+SEQUENCE=%d,%d\r\n", socket, SEQUENCE);
            QS100_SendCommand(cmd);
            i++;
        }
    }

    /* 关闭客户端 */
    i = 0;
    QS100_CloseClient(socket);
    while (i < 10)
    {
        char *buffer_str = (char *)tempBuffer;

        if (strstr((char *)buffer_str, "OK") != NULL)
        {
            DEBUG_Printf("Close Client Successful\r\n");
            break;
        }
        else
        {
            HAL_Delay(1000);
            QS100_CloseClient(socket);
            i++;
        }
    }
}