#include "pwr.h"


void PWR_Init(void)
{ 
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();
}
