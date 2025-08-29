#ifndef __RTC_H__
#define __RTC_H__

#include "sys/sys.h"

extern RTC_HandleTypeDef hrtc;

void RTC_Init(void);
void RTC_SetAlarm(uint32_t seconds);

#endif
