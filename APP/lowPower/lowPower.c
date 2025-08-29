#include "lowPower/lowPower.h"

void LOWPOWER_EnterLowPower(void)
{
    QS100_EnterLowPowerMode();
    AT6558R_EnterLowPowerMode();
    /*  */
    RTC_Init();
    RTC_SetAlarm(10); // 设置20秒后唤醒

    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU); // 清除唤醒标志
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB); // 清除待机标志

    DEBUG_Printf("Entering Low Power Mode...\r\n");
    HAL_Delay(1000);            // 确保所有设置生效
    
    HAL_PWR_EnterSTANDBYMode(); // 进入待机模式
    DEBUG_Printf("!!!mei Jin Low Power Mode!!!\r\n");
}

void LOWPOWER_Wakeup(void)
{
    QS100_Wakeup();
    AT6558R_Wakeup();
    DEBUG_Printf("Wake up from Low Power Mode\r\n");
}