#include "delay/delay.h"
#include "debug/debug.h"
#include "sys/sys.h"
#include "at6558r/at6558r.h"
#include "ds3553/ds3553.h"
#include "lora/lora.h"
#include "qs100/qs100.h"

int main(void)
{
    HAL_Init();                         /* HAL库初始化 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 系统时钟初始化 */
    delay_init(72);                     /* 延时函数初始化 */
    DEBUG_Init();                       /* 调试接口初始化 */
    AT6558R_Init();
    DS3553_Init();
    LORA_Init();         /* LoRa初始化 */
    QS100_Init();       /* QS100初始化 */

    // AT6558R_PrintInfo(); /* 打印AT6558R信息 */
    // DS3553_PrintInfo();  /* 打印DS3553信息 */
    // QS100_PrintInfo();   /* 打印QS100信息 */
   
    while (1)
    {
        QS100_SendData((uint8_t *)"zmor\r\n", strlen("zmor\r\n")); 
        DEBUG_Printf("Hello World!\r\n");
        HAL_Delay(3000);
    }
}
