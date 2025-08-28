#include "USART/usart3.h"

/* 定义句柄 */
UART_HandleTypeDef huart3;

void USART3_Init(void)
{
    /* 定义结构体 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* 使能时钟 */
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    /* 配置GPIO引脚 */
    /* USART3 TX - PB10 */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;		/* 推挽输出模式 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* USART3 RX - PB11 */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; 	/* 输入模式 */
    GPIO_InitStruct.Pull = GPIO_NOPULL;			/* 无上下拉 */
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* 配置UART参数 */
    huart3.Instance = USART3;
    huart3.Init.BaudRate = 9600;					 /* 设置波特率 */
    huart3.Init.WordLength = UART_WORDLENGTH_8B;	 /* 设置传输数据为长度 */
    huart3.Init.StopBits = UART_STOPBITS_1;		  	 /* 设置停止位长度 */
    huart3.Init.Parity = UART_PARITY_NONE;			 /* 设置奇偶校验 */
    huart3.Init.Mode = UART_MODE_TX_RX;				 /* 设置传输模式 */	
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;	 /* 硬件流控制 */
    huart3.Init.OverSampling = UART_OVERSAMPLING_16; /* 设置过采样 */
    
    /* 初始化UART */
    HAL_UART_Init(&huart3);
}

void USART3_SendData(uint8_t *buf, uint16_t len)
{
    HAL_UART_Transmit(&huart3, buf, len, 3000);
}

void USART3_ReceiveData(uint8_t *buf, uint16_t len)
{
    HAL_UART_Receive(&huart3, buf, len, 3000);
}

uint16_t USART3_ReceiveToIdle(uint8_t *buf, uint16_t len)
{
    uint16_t rxlen;
    HAL_UARTEx_ReceiveToIdle(&huart3, buf, len, &rxlen, 3000);
    return rxlen;
}