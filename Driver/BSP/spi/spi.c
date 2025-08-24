#include "spi.h"

/* CS引脚控制函数 */
static void cs_low() { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); }   
static void cs_high() { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); }  

SPI_HandleTypeDef hspi1;  /* SPI句柄定义 */

void SPI_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();  /* 启用GPIOA和SPI1时钟 */

    /* PA4片选，PA5时钟，PA6输入(MISO)，PA7输出(MOSI) */
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    hspi1.Instance = SPI1;                                      /* 设置SPI实例 */
    hspi1.Init.Mode = SPI_MODE_MASTER;                          /* 设置为主模式 */
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;                /* 双线全双工 */
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;                    /* 8位数据 */
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;                  /* 时钟极性低 */
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;                      /* 时钟相位在第一个边沿 */
    hspi1.Init.NSS = SPI_NSS_SOFT;                              /* 软件管理片选线 */
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;   /* 波特率预分频 */
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;                     /* MSB(高位)先行 */
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;                     /* 禁用TI模式 */
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;     /* 禁用CRC计算 */
    hspi1.Init.CRCPolynomial = 10;                              /* CRC多项式 */
    HAL_SPI_Init(&hspi1);                                       /* 初始化SPI */

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7; /* 输出模式下仍可正常输入 */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* 初始片选信号为高电平 */
    cs_high();  
}

void SPI_Start(void)
{
    cs_low();   /* 拉低片选信号，选中从设备 */
}

void SPI_Stop(void)
{
    cs_high();  /* 拉高片选信号，释放从设备 */
} 

void SPI_TransmitByte(uint8_t transmitData)
{
    HAL_SPI_Transmit(&hspi1, &transmitData, 1, 1000);  /* 发送数据 */
}

void SPI_TransmitBytes(uint8_t *transmitDataBuffer, uint16_t size)
{
    HAL_SPI_Transmit(&hspi1, transmitDataBuffer, size, 1000);  /* 发送数据 */
}

uint8_t SPI_ReceiveByte(void)
{
    uint8_t receiveData = 0;
    HAL_SPI_Receive(&hspi1, &receiveData, 1, 1000);
    return receiveData;
}

void SPI_ReceiveBytes(uint8_t *receiveDataBuffer, uint16_t size)
{
    HAL_SPI_Receive(&hspi1, receiveDataBuffer, size, 1000);
}

uint8_t SPI_SwapByte(uint8_t transmitData)
{
    uint8_t receiveData = 0;
    HAL_SPI_TransmitReceive(&hspi1, &transmitData, &receiveData, 1, 1000);
    return receiveData;
}

void SPI_SwapBytes(uint8_t *transmitDataBuffer, uint8_t *receiveDataBuffer, uint16_t size)
{
    HAL_SPI_TransmitReceive(&hspi1, transmitDataBuffer, receiveDataBuffer, size, 1000);
}