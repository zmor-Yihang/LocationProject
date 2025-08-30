#ifndef __LOCATION_H__
#define __LOCATION_H__

#include "lowPower/lowPower.h"
#include "at6558r/at6558r.h"
#include "qs100/qs100.h"
#include "Debug/debug.h"

typedef struct
{
    float latitude;  // 纬度
    float longitude; // 经度
    uint32_t steps;  // 步数
} LocationDataTypeDef;

void LOCATION_SendLocationData(uint32_t seconds);

#endif
