#include "location.h"

LocationDataTypeDef locationData;

static void LOCATION_GetGPSData(void)
{

}

static void LOCATION_GetStepData(void)
{

}

static void LOCATION_ProcessData(void)
{

}

void LOCATION_SendLocationData(uint32_t seconds)
{
    LOWPOWER_Wakeup();

    AT6558R_Init();
    QS100_Init();

    LOCATION_GetGPSData();
    LOCATION_GetStepData();
    LOCATION_ProcessData();
    
    /* 待修改 */
    QS100_SendData("Location Data", strlen("Location Data"));

    LOWPOWER_EnterLowPower(seconds); // 进入低功耗模式，20秒后唤醒
}