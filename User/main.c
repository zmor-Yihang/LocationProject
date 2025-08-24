#include "delay/delay.h"
#include "debug/debug.h"
#include "sys/sys.h"
#include "at6558r/at6558r.h"
#include "ds3553/ds3553.h"

int main(void)
{      
    HAL_Init();                         /* HAL库初始化 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 系统时钟初始化 */
    delay_init(72);                     /* 延时函数初始化 */  
    DEBUG_Init();                       /* 调试接口初始化 */
    AT6558R_Init();
    DS3553_Init();

    AT6558R_PrintInfo();                /* 打印AT6558R信息 */
    DS3553_PrintInfo();

    while(1)
    {
        DS3553_GetStepCount();
        HAL_Delay(1000);
        DEBUG_Printf("DS3553 COUNTER: %d\r\n", countOfStep);
    }
}
