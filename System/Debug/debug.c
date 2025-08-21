#include "debug/debug.h"

/****************************************开始*******************************************/
/* 加入以下代码, 支持printf函数, 而不需要选择use MicroLIB */

#if (__ARMCC_VERSION >= 6010050)            /* 判断是否使用AC6编译器 */
__asm(".global __use_no_semihosting\n\t");  /* 声明不使用半主机模式 */
__asm(".global __ARM_use_no_argv \n\t");    /* AC6下声明main无参数 */

#else
/* 如果使用AC5编译器，定义__FILE结构体与半主机相关设置 */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;  /* 文件句柄 */
    /* 可根据需要增加成员，通常只用printf无需其它操作 */
};

#endif

/* 不使用半主机，重定义_ttywrch函数 */
int _ttywrch(int ch)
{
    ch = ch;  /* 避免编译器警告，空实现 */
    return ch; /* 返回ch */
}

/* 定义_sys_exit函数，防止使用半主机 */
void _sys_exit(int x)
{
    x = x;  /* 避免编译器警告，空实现 */
}

/* 定义_sys_command_string函数，兼容性需要，返回NULL */
char *_sys_command_string(char *cmd, int len)
{
    return NULL; /* 不处理命令，返回NULL */
}

/* 定义标准输出文件结构体 */
FILE __stdout;

/* 重定义fputc函数，printf最终通过fputc输出到串口 */
int fputc(int ch, FILE *f)
{
    while ((USART1->SR & 0X40) == 0);     /* 等待上一个字符发送完成 */
    USART1->DR = (uint8_t)ch;             /* 将要发送的字符ch写入DR寄存器 */
    return ch;                            /* 返回发送的字符 */
}
/*********************************************结束*************************************/

/* 定义句柄 */
UART_HandleTypeDef huart1;

void DEBUG_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}
