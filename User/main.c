#include "delay/delay.h"
#include "debug/debug.h"
#include "sys/sys.h"
#include "usart/usart.h"
#include "gpio/gpio.h"

int main(void)
{      
    /* 系统初始化顺序 */
    HAL_Init();                         /* HAL库初始化 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 系统时钟初始化 */
    delay_init(72);                     /* 延时函数初始化 */
    DEBUG_Init();                       /* 调试接口初始化 */
    GPIO_Init();                        /* GPIO初始化 */
    USART_Init();                      /* USART2初始化 */

    uint8_t rxBuffer[512];
    
    while(1)
    {
        printf("hello\r\n");
        HAL_Delay(1000);
        
        /* 清空接收缓冲区 */
        memset(rxBuffer, 0, sizeof(rxBuffer));
        
        /* 接收数据，超时时间1000ms */
        HAL_UART_Receive(&huart2, rxBuffer, sizeof(rxBuffer)-1, 1000);

        printf("Received: %s\r\n", rxBuffer);

    }
}
