#ifndef __LOCATION_H__
#define __LOCATION_H__

#include "string.h"
#include "user_config.h"
#include "lowPower/lowPower.h"
#include "at6558r/at6558r.h"
#include "ds3553/ds3553.h"
#include "qs100/qs100.h"
#include "Debug/debug.h"
#include "cJSON/cJSON.h"

extern LocationDataTypeDef locationData;

void LOCATION_SendLocationData(uint32_t seconds);

#endif
