#ifndef __LOWPOWER_H__
#define __LOWPOWER_H__

#include "at6558r/at6558r.h"
#include "qs100/qs100.h"
#include "ds3553/ds3553.h"
#include "rtc/rtc.h"
#include "pwr/pwr.h"

void LOWPOWER_EnterLowPower(uint32_t seconds);
void LOWPOWER_Wakeup(void);

#endif
