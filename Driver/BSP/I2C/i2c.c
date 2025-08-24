/* I2C总线通信驱动文件 */
#include "i2c.h"

/* I2C1句柄结构体 */
I2C_HandleTypeDef hi2c1;

void I2C1_Init(void)
{ 
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能I2C1和GPIOB时钟 */
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 配置I2C1基本参数 */
    hi2c1.Instance = I2C1;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;  /* 7位地址模式 */
    hi2c1.Init.ClockSpeed = 100000;                       /* 通信速率，最大400k */
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE; /* 单地址模式 */
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;               /* SCL高低电平持续时间之比  */
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE; /* 禁用广播模式 */
    /* I2C主设备发送通用呼叫地址（0x00）时，I2C 总线上的所有从设备都会响应这个地址，
        这个模式通常用于对总线上的所有从设备进行广播消息或重置操作。*/
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_ENABLE;     /* 允许时钟拉伸 */
    /* 允许I2C从设备在数据传输过程中暂停（“拉伸”）时钟信号。当从设备忙于处理
       数据或等待其他操作时，它可以通过拉低时钟线来延迟主设备的时钟信号，以便有更
       多的时间来处理请求。主设备会在时钟信号被恢复后继续通信。 */
    hi2c1.Init.OwnAddress1 = 0;     /* 自身地址1 */
    hi2c1.Init.OwnAddress2 = 0;     /* 自身地址2 */
    HAL_I2C_Init(&hi2c1);           /* 初始化I2C1 */

    /* 配置I2C引脚为复用开漏输出模式 */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;      /* 复用开漏输出 */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7; /* PB6(SCL) PB7(SDA) */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;  /* 高速输出 */
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);      /* 初始化GPIO */
}

/*
 * 函数名称: I2C_SendByte
 * 函数功能: 向I2C从设备发送一个字节数据
 * 输入参数: adr - 从设备7位地址(需要左移1位)
 *          data - 要发送的字节数据
 * 返回值:   无
 * 说明:     使用阻塞方式发送，超时时间1000ms
 */
/* 向从设备发送一个字节数据 */
void I2C1_SendByte(uint16_t adr, uint8_t data)
{
    HAL_I2C_Master_Transmit(&hi2c1, adr, &data, 1, 1000);
}

/*
 * 函数名称: I2C_SendBytes
 * 函数功能: 向I2C从设备发送多个字节数据
 * 输入参数: adr - 从设备7位地址(需要左移1位)
 *          sendBuffre - 指向发送数据缓冲区的指针
 *          len - 要发送的数据长度
 * 返回值:   无
 * 说明:     使用阻塞方式发送，超时时间1000ms
 */
/* 向从设备发送多个字节数据 */
void I2C1_SendBytes(uint16_t adr, uint8_t *sendBuffre, uint8_t len)
{
    HAL_I2C_Master_Transmit(&hi2c1, adr, sendBuffre, len, 1000);
}

/**
 * @brief 从I2C从设备接收一个字节数据
 * @param adr I2C从设备地址
 * @param recieveBuffer 接收数据缓冲区指针
 * @retval None
 * @note 超时时间设置为1000ms
 */
void I2C1_ReceiveByte(uint16_t adr, uint8_t *recieveBuffer)
{
    HAL_I2C_Master_Receive(&hi2c1, adr, recieveBuffer, 1, 1000);
}

/**
 * @brief 从I2C从设备接收多个字节数据
 * @param adr I2C从设备地址
 * @param recieveBuffer 接收数据缓冲区指针
 * @param len 要接收的数据长度
 * @retval None
 * @note 超时时间设置为1000ms
 */
void I2C1_ReceiveBytes(uint16_t adr, uint8_t *recieveBuffer, uint8_t len)
{
    HAL_I2C_Master_Receive(&hi2c1, adr, recieveBuffer, len, 1000);
}