#include "rtc.h"

RTC_HandleTypeDef hrtc;

void RTC_Init(void)
{
    /* 使能PWR时钟并解锁备份域 */
    PWR_Init();

    /* 配置LSI时钟源 */
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY) == RESET)
        ;

    /* 选择RTC时钟源为LSI */
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

    __HAL_RCC_RTC_ENABLE(); // 使能RTC时钟

    /* 配置RTC */
    hrtc.Instance = RTC;
    hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND; /* 自动配置1秒预分频 */

    HAL_RTC_Init(&hrtc);
}

void RTC_SetAlarm(uint32_t seconds)
{
    RTC_AlarmTypeDef sAlarm = {0};

    /* 获取当前时间 */
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    // 计算闹钟时间，处理秒数溢出
    uint32_t totalSeconds = sTime.Seconds + seconds;
    sAlarm.AlarmTime.Hours = sTime.Hours;
    sAlarm.AlarmTime.Minutes = sTime.Minutes;
    sAlarm.AlarmTime.Seconds = totalSeconds % 60;
    
    // 处理分钟进位
    uint32_t additionalMinutes = totalSeconds / 60;
    sAlarm.AlarmTime.Minutes = (sAlarm.AlarmTime.Minutes + additionalMinutes) % 60;
    
    // 处理小时进位
    uint32_t additionalHours = (sTime.Minutes + additionalMinutes) / 60;
    sAlarm.AlarmTime.Hours = (sAlarm.AlarmTime.Hours + additionalHours) % 24;

    HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BIN);
}