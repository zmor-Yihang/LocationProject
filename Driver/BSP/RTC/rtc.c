#include "rtc.h"

RTC_HandleTypeDef hrtc;

void RTC_Init(void)
{
    /* 使能时钟 */
    __HAL_RCC_BKP_CLK_ENABLE();
        
    /* 配置LSI时钟源 */
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY) == RESET);
    
    /* 选择RTC时钟源为LSI */
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
    
    /* 使能RTC时钟 */
    __HAL_RCC_RTC_ENABLE();
    
    /* 配置RTC */
    hrtc.Instance = RTC;
    hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;  /* 自动配置1秒预分频 */
    
    HAL_RTC_Init(&hrtc);
}

void RTC_SetAlarm(uint32_t seconds)
{
    RTC_AlarmTypeDef sAlarm = {0};
    
    /* 获取当前时间 */
    RTC_TimeTypeDef sTime = {0};
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    
    sAlarm.AlarmTime.Hours = sTime.Hours;
    sAlarm.AlarmTime.Minutes = sTime.Minutes;
    sAlarm.AlarmTime.Seconds = sTime.Seconds + seconds;
    
    HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BIN);
}