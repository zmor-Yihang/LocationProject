#include "gpio.h"

void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 开启AFIO时钟，禁用J-Link调试端口PB3，将其重映射为普通GPIO */
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
    
    /* 使能GPIOB时钟 */   
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 配置PB3引脚为输出模式 */
    GPIO_InitStruct.Pin = GPIO_PIN_3;                /* 选择PB3引脚 */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;       /* 推挽输出模式 */
    GPIO_InitStruct.Pull = GPIO_NOPULL;               /* 无上拉/下拉 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;      /* 高速模式 */

    /* 初始化GPIOB */
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* 设置初始状态为高电平 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
}


