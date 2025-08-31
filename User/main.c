#include "delay/delay.h"
#include "debug/debug.h"
#include "sys/sys.h"
#include "location/location.h"

int main(void)
{
    HAL_Init();                         /* HAL库初始化 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 系统时钟初始化 */
    delay_init(72);                     /* 延时函数初始化 */
    DEBUG_Init();                       /* 调试接口初始化 */

    LOCATION_SendLocationData(20);      /* 发送定位数据并进入低功耗模式，20秒后唤醒(20s发送一次) */

    while (1)
    {

    }
}
