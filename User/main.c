#include "delay/delay.h"
#include "debug/debug.h"
#include "sys/sys.h"
#include "lowPower/lowPower.h"

int main(void)
{
    HAL_Init();                         /* HAL库初始化 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 系统时钟初始化 */
    delay_init(72);                     /* 延时函数初始化 */
    DEBUG_Init();                       /* 调试接口初始化 */

    while (1)
    {
        LOWPOWER_Wakeup();

        QS100_Init();
        QS100_SendData("Hello after Wakeup", strlen("Hello after Wakeup"));

        LOWPOWER_EnterLowPower();
        HAL_Delay(10000);
    }
}
