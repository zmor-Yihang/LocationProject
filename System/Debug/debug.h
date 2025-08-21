#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "sys/sys.h"
#include "stdio.h"
#include "string.h"

void DEBUG_Init(void);

/* 使能调试接口 */
#define DEBUG_ENABLE

/* 宏定义控制调试开关 */
#ifdef DEBUG_ENABLE
    #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : \
                         (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))

    #define DEBUG_Printf(fmt, ...) printf("[%s, %d] "fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
#else
    #define __FILENAME__ 
    #define DEBUG_Printf(fmt, ...)
#endif

#endif
