#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "sys/sys.h"

#define IP "112.125.89.8"   /*  服务器IP地址 */
#define PORT 43458          /*  服务器端口号 */

/* 使能 GNRMC 演示 */
#define ENABLE_GNRMC_DEMO 

/* 使能调试接口 */
#define DEBUG_ENABLE

typedef struct
{
    uint8_t year;  // 年
    uint8_t month; // 月
    uint8_t day;   // 日
} CalendarTypeDef;

typedef struct
{
    uint8_t hour;   // 时
    uint8_t minute; // 分
    uint8_t second; // 秒
} TimeTypeDef;

typedef struct
{
    CalendarTypeDef calendar;    // 日期
    TimeTypeDef time;            // 时间
    uint8_t latitude_direction;  // 纬度方向 (0: N, 1: S)
    uint8_t longitude_direction; // 经度方向 (0: E, 1: W)
    float latitude;              // 纬度
    float longitude;             // 经度
    uint32_t steps;              // 步数

    uint8_t ID[33];              // 设备ID

    uint8_t json_data[513];       // JSON格式的位置信息

} LocationDataTypeDef;

#endif
