#include "lowPower/lowPower.h"

void LOWPOWER_EnterLowPower(void)
{
    QS100_EnterLowPowerMode();
    AT6558R_EnterLowPowerMode();
    /*  */
    RTC_SetAlarm(20); // 设置20秒后唤醒
    PWR_Init();

    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU); // 清除唤醒标志
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB); // 清除待机标志

    DEBUG_Printf("Entering Low Power Mode...\r\n");
    HAL_Delay(1000);            // 确保所有设置生效
    
    HAL_PWR_EnterSTANDBYMode(); // 进入待机模式
}

void LOWPOWER_Wakeup(void)
{
    QS100_Wakeup();
    AT6558R_Wakeup();
    DEBUG_Printf("Woke up from Low Power Mode\r\n");
}