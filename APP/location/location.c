#include "location.h"

LocationDataTypeDef locationData = {0};


/**
 * @brief 尝试获取并验证来自 AT6558R 的 GPS 数据。
 * @details
 * 函数最多循环 10 次，每次循环按以下顺序执行：
 *  - 调用 AT6558R_VerifyIntegrityOfGPSData() 检查是否接收到完整的 GPS 数据（完整性检测）。
 *  - 若完整性检测通过，打印接收到的 rxBuffer 内容，然后调用
 *    AT6558R_VerifyValidityOfGPSData() 进行有效性验证（例如定位是否有效、字段是否合法等）。
 *  - 若有效性检测通过，立即返回 1 表示成功。
 *  - 若任一检测失败，输出调试信息并继续重试，直至达到最大尝试次数。
 *
 * 注意：
 *  - 该函数依赖外部函数 AT6558R_VerifyIntegrityOfGPSData()、AT6558R_VerifyValidityOfGPSData()
 *    以及全局接收缓冲区 rxBuffer。
 *  - 使用 DEBUG_Printf 输出诊断信息以便调试。
 *  - 为避免无限阻塞，函数在达到 10 次尝试后会返回失败。
 *
 * @retval 1 获取到完整且有效的 GPS 数据（成功）。
 * @retval 0 在 10 次尝试后仍未获取到完整且有效的 GPS 数据（失败）。
 */

static uint8_t LOCATION_GetGPSData(void)
{
    uint8_t i = 0;

    while (i < 10)
    {
        if (AT6558R_VerifyIntegrityOfGPSData() == 1)
        {
            /* 获取到完整的GPS数据 */
            DEBUG_Printf("Integrity GPS data received\r\n");
            DEBUG_Printf("GPS Data:\r\n%s\r\n", rxBuffer);
            if (AT6558R_VerifyValidityOfGPSData() == 1)
            {
                DEBUG_Printf("Valid GPS data received\r\n");
                return 1; // 获取到有效的GPS数据，跳出循环
            }
            else
            {
                DEBUG_Printf("Invalid GPS data\r\n");
                DEBUG_Printf("Try Again Of %d...\r\n", i);
                HAL_Delay(1000); // 延时1秒后重试
                i++;
            }
        }
        else
        {
            DEBUG_Printf("Don't have Integrity GPS data\r\n");
            DEBUG_Printf("Try Again Of %d...\r\n", i);
            HAL_Delay(1000); // 延时1秒后重试
            i++;
        }
    }
    DEBUG_Printf("Failed to get valid GPS data after 10 attempts\r\n");
    return 0; /* 获取完整且有效的GPS数据失败 */
}

static void LOCATION_GetStepData(void)
{
    DS3553_Init();
    locationData.steps = DS3553_GetStepCount();
    DEBUG_Printf("Current Step Count: %lu\r\n", locationData.steps);
}

static void LOCATION_ProcessData(void)
{
    AT6558R_ExtractGNRMCData();

    cJSON *root = cJSON_CreateObject();

    /* ID */
    cJSON_AddStringToObject(root, "ID", (char *)locationData.ID);

    /* 日期时间字符串 */
    char datetime_str[20];
    sprintf(datetime_str, "%d-%d-%d %02d:%02d:%02d",
            locationData.calendar.year + 2000, locationData.calendar.month, locationData.calendar.day,
            locationData.time.hour, locationData.time.minute, locationData.time.second);
    cJSON_AddStringToObject(root, "datetime", datetime_str);

    /* 坐标 */
    cJSON_AddNumberToObject(root, "latitude",  locationData.latitude);
    cJSON_AddStringToObject(root, "lat_dir",   (locationData.latitude_direction==0) ? "N" : "S");
    cJSON_AddNumberToObject(root, "longitude", locationData.longitude);
    cJSON_AddStringToObject(root, "lon_dir",   (locationData.longitude_direction==0) ? "E" : "W");

    /* 步数 */
    cJSON_AddNumberToObject(root, "steps", locationData.steps);

    /* 输出成字符串（需要 free 掉） */
    char *json_str = cJSON_PrintUnformatted(root);

    DEBUG_Printf("JSON String:\r\n%s\r\n", json_str);
    memcpy(locationData.json_data, (uint8_t *)json_str, strlen(json_str));

    /* 确保字符串以'\0'结尾 */
    locationData.json_data[strlen(json_str)] = '\0';

    DEBUG_Printf("JSON Data:\r\n%s\r\n", locationData.json_data);

    cJSON_Delete(root);
    cJSON_free(json_str);
}

void LOCATION_SendLocationData(uint32_t seconds)
{
    AT6558R_Init();
    QS100_Init();

    LOWPOWER_Wakeup(); // 从低功耗模式唤醒

    if(LOCATION_GetGPSData())
    {
        LOCATION_GetStepData();
        LOCATION_ProcessData();
        QS100_SendData(locationData.json_data, strlen((char *)locationData.json_data));
    }
    else
    {
        QS100_SendData("No valid GPS data available", strlen("No valid GPS data available"));
    }

    LOWPOWER_EnterLowPower(seconds); // 进入低功耗模式，20秒后唤醒
}