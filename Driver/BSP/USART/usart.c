#include "usart.h"

UART_HandleTypeDef huart2;

void USART_Init(void)
{
    /* 配置UART参数 */
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 9600;					 /* 设置波特率 */
    huart2.Init.WordLength = UART_WORDLENGTH_8B;	 /* 设置传输数据为长度 */
    huart2.Init.StopBits = UART_STOPBITS_1;		  	 /* 设置停止位长度 */
    huart2.Init.Parity = UART_PARITY_NONE;			 /* 设置奇偶校验 */
    huart2.Init.Mode = UART_MODE_TX_RX;				 /* 设置传输模式 */	
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;	 /* 硬件流控制 */
    huart2.Init.OverSampling = UART_OVERSAMPLING_16; /* 设置过采样 */
    
        /* 定义结构体 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* 使能时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();
    
    /* 配置GPIO引脚 */
    /* USART2 TX - PA2 */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;		/* 推挽输出模式 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* USART2 RX - PA3 */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; 	/* 输入模式 */
    GPIO_InitStruct.Pull = GPIO_NOPULL;			/* 无上下拉 */
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* 初始化UART */
    HAL_UART_Init(&huart2);
}
