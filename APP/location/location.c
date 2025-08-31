/**
 * @file location.c
 * @brief 位置与传感器数据采集与发送逻辑实现
 *
 * 本文件负责从 AT6558R 模块获取 GPS 数据、从 DS3553 获取步数，并将
 * 解析后的位置信息与步数打包为 JSON 字符串，通过 QS100 模块发送。
 */

#include "location.h"

/**
 * @brief 全局位置数据结构实例
 *
 * 用于在模块内部保存 ID、时间、坐标、步数以及生成的 JSON 数据。
 * 初始化为全零。
 */
LocationDataTypeDef locationData = {0};

/**
 * @brief 尝试从 AT6558R 获取并验证 GPS 数据
 *
 * 该函数会循环最多 10 次尝试读取 GPS 数据并进行两步检查：完整性检查
 * (AT6558R_VerifyIntegrityOfGPSData) 和有效性检查
 * (AT6558R_VerifyValidityOfGPSData)。当两步检查均通过时返回 1，否则在
 * 达到最大重试次数后返回 0。
 *
 * 依赖：全局接收缓冲区 rxBuffer 以及外部函数 AT6558R_VerifyIntegrityOfGPSData
 * 和 AT6558R_VerifyValidityOfGPSData。函数内部通过 DEBUG_Printf 输出
 * 调试信息，并在每次失败后调用 HAL_Delay(1000) 等待 1 秒后重试，避免
 * 短时间内高频请求。
 *
 * @return uint8_t 1 表示获取到完整且有效的 GPS 数据；0 表示获取失败（重试 10 次后）
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


/**
 * @brief 读取并更新步数数据
 *
 * 本函数负责初始化步数传感器 DS3553，并读取当前步数计数，存入
 * 全局变量 locationData.steps 中，同时打印当前步数以便调试。
 *
 * 无返回值，若传感器初始化或读取失败，应由底层函数通过调试信息提示。
 */
static void LOCATION_GetStepData(void)
{
    DS3553_Init();
    locationData.steps = DS3553_GetStepCount();
    DEBUG_Printf("Current Step Count: %lu\r\n", locationData.steps);
}


/**
 * @brief 处理并打包位置与步数信息为 JSON
 *
 * 先调用 AT6558R_ExtractGNRMCData() 更新全局的 locationData（时间、坐标等）
 * 然后使用 cJSON 构造一个包含 ID、datetime、latitude、lat_dir、longitude、
 * lon_dir、steps 的 JSON 对象，将其序列化为紧凑字符串并复制到
 * locationData.json_data 中。
 *
 * 注意：cJSON_PrintUnformatted 返回分配的字符串，函数内部调用 cJSON_free
 * 释放该内存。复制时应确保 destination 有足够空间以避免溢出（调用者
 * 需在设计时保证）。
 */
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


/**
 * @brief 初始化外设并发送位置信息
 *
 * @param seconds 进入低功耗模式前的延迟（秒），调用 LOWPOWER_EnterLowPower(seconds)
 *
 * 函数流程：
 *  - 初始化 AT6558R（GPS）与 QS100（通信）模块
 *  - 从低功耗模式唤醒
 *  - 尝试获取并验证 GPS 数据（调用 LOCATION_GetGPSData）
 *    - 若成功：获取步数、处理数据为 JSON 并通过 QS100_SendData 发送
 *    - 若失败：发送错误信息字符串
 *  - 最后调用 LOWPOWER_EnterLowPower(seconds) 返回低功耗状态
 */
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