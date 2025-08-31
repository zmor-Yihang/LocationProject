/**
 * @file    at6558r.c
 * @brief   AT6558R GNSS定位芯片驱动程序
 * @details AT6558R是一款高性能、低功耗的多模GNSS定位芯片，支持多种卫星导航系统：
 *          - GPS (美国全球定位系统)
 *          - BDS (中国北斗卫星导航系统)
 *          - GLONASS (俄罗斯格洛纳斯系统)
 * @note    使用前需要初始化USART2接口和GPIOB3控制引脚
 */

#include "at6558r.h"

/* 引用全局位置数据变量（定义在 APP/location/location.c 中，类型在 user_config.h 中） */
extern LocationDataTypeDef locationData;

/* AT命令临时缓冲区 */
uint8_t tempBufferOfCmd[32];

/**
 * @brief   计算AT命令的NMEA校验和并格式化命令
 * @details 按照NMEA 0183协议标准计算XOR校验和，并将命令格式化为标准格式。
 *          NMEA命令格式：$命令*校验和\r\n
 *          校验和计算方法：对'$'和'*'之间的所有字符进行XOR运算
 * @param   cmd 指向需要计算校验和的命令字符串的指针
 * @retval  None
 * @note    该函数为静态函数，仅在本文件内部使用
 * @warning 确保cmd指向有效的字符串，且tempBufferOfCmd有足够空间
 */
static void AT6558R_CalculateChecksum(char *cmd)
{
    char *ptr = cmd;      /* 字符串遍历指针 */
    uint8_t checksum = 0; /* XOR校验和累加器 */

    /* 遍历命令字符串中的每个字符，计算XOR校验和 */
    while (*ptr)
    {
        checksum ^= *ptr++; /* 异或运算并移动指针 */
    }

    /* 按照NMEA 0183标准格式化命令：$命令*校验和\r\n */
    sprintf((char *)tempBufferOfCmd, "$%s*%02X\r\n", cmd, checksum);
}

/**
 * @brief   发送AT命令到AT6558R GNSS芯片
 * @details 将AT命令按照NMEA协议格式化并通过UART接口发送给芯片。
 *          发送流程：计算校验和 -> 格式化命令 -> UART发送
 * @param   cmd 指向需要发送的AT命令字符串的指针
 * @retval  None
 * @note    该函数为静态函数，仅在本文件内部使用
 * @warning 确保USART2已正确初始化，且cmd指向有效的命令字符串
 */
static void AT6558R_SendCmd(char *cmd)
{
    /* 计算命令的NMEA校验和并格式化为标准格式 */
    AT6558R_CalculateChecksum(cmd);

    /* 通过USART2接口发送格式化后的AT命令 */
    USART2_SendString((char *)tempBufferOfCmd);
}

/**
 * @brief   打印AT6558R GNSS芯片的详细信息
 * @details 通过发送一系列AT查询命令来获取并显示芯片的完整信息，
 *          包括固件版本、硬件序列号、接收器模式、客户编号等。
 *          所有查询结果会通过UART接收并存储在rxBuffer中。
 * @param   None
 * @retval  None
 * @note    执行后需要一定时间等待芯片响应，建议调用后延时500ms以上
 * @warning 确保rxBuffer有足够空间存储所有响应数据
 */
void AT6558R_PrintInfo(void)
{
    /* 发送固件版本查询命令，获取芯片软件版本信息 */
    AT6558R_SendCmd(AT6558R_Info_FirmwareVersion);

    /* 发送序列号查询命令，获取芯片唯一标识 */
    AT6558R_SendCmd(AT6558R_Info_SerialNumber);

    /* 发送多模接收器模式查询命令，获取支持的卫星系统 */
    AT6558R_SendCmd(AT6558R_Info_MultimodeReceiverMode);

    /* 发送客户编号查询命令，获取厂商定制信息 */
    AT6558R_SendCmd(AT6558R_Info_CustomerNumber);

    /* 发送升级代码查询命令，获取固件升级相关信息 */
    AT6558R_SendCmd(AT6558R_Info_UpgradeCode);

    /* 延时等待AT6558R芯片处理所有查询命令并发送响应数据 */
    /* 延时时间必须足够长，否则会导致数据接收不完整 */
    HAL_Delay(500);

    /* 打印接收缓冲区中的所有响应信息 */
    DEBUG_Printf("-------------------AT6558R-------------------\r\n");
    DEBUG_Printf("%s", rxBuffer);
    DEBUG_Printf("---------------------------------------------\r\n");
}

/**
 * @brief   初始化AT6558R GNSS定位模块
 * @details 完成AT6558R模块的完整初始化流程，包括硬件初始化和参数配置：
 *          1. 初始化GPIO控制引脚（启动芯片）
 *          2. 初始化UART通信接口
 *          3. 配置GNSS数据输出频率
 *          4. 设置多模卫星工作模式
 * @param   None
 * @retval  None
 * @note    此函数必须在使用其他AT6558R相关功能前调用
 * @warning 确保相关的时钟已使能，GPIO和USART2外设可正常工作
 */
void AT6558R_Init(void)
{
    /* 初始化GPIOB3引脚并设置为高电平，启动AT6558R芯片 */
    /* 该引脚控制芯片的电源使能，高电平有效 */
    GPIOB3_Init();

    /* 初始化USART2串口通信接口 */
    /* USART2用于发送AT命令和接收GNSS数据 */
    USART2_Init();

    /* 配置GNSS数据输出频率为1Hz */
    /* 1Hz表示每秒输出一次完整的定位信息 */
    AT6558R_SendCmd(AT6558R_FREQUENCY_1Hz);

    /* 设置GNSS工作模式为双模（GPS + 北斗BDS） */
    /* 双模可以同时接收GPS和北斗卫星信号，提高定位精度和可靠性 */
    AT6558R_SendCmd(AT6558R_MODE_Dual);
}

/**
 * @brief   验证GNSS数据的完整性
 * @details 通过检查接收缓冲区中的关键NMEA句子来验证数据完整性。
 *          该函数主要检查GGA和TXT句子是否都存在：
 *          - GGA句子：包含定位质量、经纬度、高度等核心定位信息
 *          - TXT句子：包含芯片状态、卫星数量等文本信息
 * @param   None
 * @retval  uint8_t 数据完整性状态
 *          @arg 1: 数据完整，包含所需的关键句子
 *          @arg 0: 数据不完整，缺少关键信息
 * @note    建议在处理GNSS数据前调用此函数进行验证
 * @warning 需要确保rxBuffer中包含完整的NMEA数据流
 */
uint8_t AT6558R_VerifyIntegrityOfGPSData(void)
{
    /* 检查接收缓冲区是否同时包含GGA和TXT关键句子 */
    /* GGA句子包含核心定位信息，TXT句子包含状态信息 */
    if (strstr((char *)rxBuffer, "GGA") != NULL && strstr((char *)rxBuffer, "TXT") != NULL)
    {
        return 1; /* 数据完整，包含必要的NMEA句子 */
    }

    return 0; /* 数据不完整，缺少关键的NMEA句子 */
}

/**
 * @brief 验证接收到的 GPS 数据是否有效（GNRMC 语句状态为 'A'）
 *
 * @details
 * 在全局接收缓冲区 rxBuffer 中查找以 "$GNRMC" 开头的 NMEA 语句，
 * 并解析该语句中的状态字段（A = 数据有效，V = 数据无效）。
 * 如果找到 GNRMC 且状态字段为 'A'，则返回 1，表示 GPS 数据有效；
 * 否则返回 0，表示 GPS 数据无效或未找到有效的 GNRMC 语句。
 *
 * @return uint8_t
 * @retval 1 表示 GPS 数据有效（状态字段为 'A'）
 * @retval 0 表示 GPS 数据无效、未找到 GNRMC 或解析失败
 *
 * @note
 * - 函数假定存在一个名为 rxBuffer 的全局缓冲区并包含完整的 NMEA 消息串。
 * - 函数使用 strstr 查找 "$GNRMC" 并使用 sscanf 提取状态字符。
 *
 * @warning
 * - 在调用 strstr 或 sscanf 之前未对返回值进行空指针检查可能导致未定义行为（例如当 rxBuffer 中不包含 "$GNRMC" 时）。
 * - 当前实现将单字符与字符串常量比较（'A' 与 "A"）会产生逻辑错误；应使用字符比较或字符串比较函数。
 *
 * @bug
 * - 若 rxBuffer 中不包含 "$GNRMC"，strstr 返回 NULL，后续对 NULL 的 sscanf 调用会崩溃。
 * - 使用了错误的比较方法（temp == "A"），应为 temp == 'A'。
 *
 * @see rxBuffer
 * @since 1.0
 */
uint8_t AT6558R_VerifyValidityOfGPSData(void)
{
    char *GNRMC = NULL;
    uint8_t temp = 0;

#ifdef ENABLE_GNRMC_DEMO
    /* 用于演示/测试：覆盖为可修改的静态示例行（便于 strtok 使用） */
    static char demo_gnrmc[] = "$GNRMC,201150.000,A,3106.67898,N,12113.52954,E,5.19,77.74,160125,,,A,V*31";
    GNRMC = demo_gnrmc;
#else
    /* 在全局接收缓冲区中查找 GNRMC 语句 */
    GNRMC = strstr((char *)rxBuffer, "$GNRMC");
#endif

    sscanf(GNRMC, "%*[^AV]%c", &temp);
    if (temp == 'A')
    {
        return 1; // GPS数据有效
    }
    else
    {
        return 0; // GPS数据无效
    }
}

/**
 * @brief  从接收缓冲区解析并提取最近一条 $GNRMC NMEA 语句的位置信息与时间/日期。
 *
 * @details
 * 本函数在全局接收缓冲（示例实现中以静态字符串替代）中查找以 "$GNRMC" 开头的一行，
 * 截取至 '*'（校验和起始符）或换行符为止，复制到本地缓冲并以逗号分割字段，按 RMC 语句格式解析：
 * - UTC 时间字段（hhmmss.sss）解析为 hour / minute / second；
 * - 纬度字段按 ddmm.mmmm 格式解析为十进制度（deg + min/60）；
 * - 经度字段按 dddmm.mmmm 格式解析为十进制度；
 * - 纬度方向 'N'/'S' 与经度方向 'E'/'W' 分别映射为 0/1 的枚举值（或标志位）；
 * - 日期字段按 ddmmyy 存入日历结构（year 为两位数 yy）。
 * - 结果写入全局结构 locationData，并调用 DEBUG_Printf 输出调试信息。
 *
 * @param void  无参数。
 *
 * @return void  解析结果通过全局变量 locationData 返回；函数本身不返回值。
 *
 * @note
 * - 输入假定为以 null 结尾的 C 字符串且包含 NMEA RMC 语句，例如：
 *   "$GNRMC,hhmmss.sss,A,ddmm.mmmm,N,dddmm.mmmm,E,speed,track,ddmmyy,,,A*CS"
 * - 本函数不计算或校验 '*' 后的校验和（校验和被截断并忽略）。
 * - 本函数不强制验证 status 字段是否为 'A'（有效），调用方若需可自行检查。
 *
 * @par 字段索引映射（基于 RMC 规范）:
 * - 0: $GNRMC
 * - 1: UTC time (hhmmss.sss)
 * - 2: status (A/V)
 * - 3: latitude (ddmm.mmmm)
 * - 4: N/S
 * - 5: longitude (dddmm.mmmm)
 * - 6: E/W
 * - 9: date (ddmmyy)
 *
 * @par 实现细节与限制
 * - 使用固定大小本地缓冲 line[128] 存放截取的 GNRMC 行；若行长超过缓冲区则会被截断；
 * - 使用 strtok() 以 ',' 分割字段，最多保存 16 个字段指针；
 * - 纬度解析假定前 2 位为度（dd），经度假定前 3 位为度（ddd）；分钟部分转换为度时按 min/60；
 * - 纬度方向 'N'->0，'S'->1；经度方向 'E'->0，'W'->1（具体映射请参见 locationData 定义）；
 * - 日期按 ddmmyy 存入 locationData.calendar，year 为两位数（yy）；
 * - 若字段不足（少于 7 个）或关键字段长度不足以解析（例如纬度/经度字符串过短），
 *   函数会提前返回且不修改对应的输出字段。
 *
 * @see rxBuffer（接收缓冲），locationData（输出结构定义），DEBUG_Printf（调试输出）
 */

/* 辅助函数：计算当月天数（基于两位年，按 2000+yy 推断闰年） */
static int days_in_month_yy(int yy /*0-99*/, int month /*1-12*/)
{
    int y = 2000 + yy;
    int leap = ((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0);
    switch (month)
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        return 31;
    case 4:
    case 6:
    case 9:
    case 11:
        return 30;
    case 2:
        return leap ? 29 : 28;
    default:
        return 30;
    }
}

void AT6558R_ExtractGNRMCData(void)
{
    char *GNRMC = NULL;

#ifdef ENABLE_GNRMC_DEMO
    /* 在演示模式下，直接使用示例行进行解析 */
    static char demo_line[] = "$GNRMC,201150.000,A,3106.67898,N,12113.52954,E,5.19,77.74,160125,,,A,V*31";
    GNRMC = demo_line;
#else
    /* 在非演示模式下，从接收缓冲区中查找 GNRMC 语句 */
    GNRMC = strstr((char *)rxBuffer, "$GNRMC");
#endif

    /* 使用 strtok 逐字段解析（直接对已就绪的一整行进行分割） */
    char *tokens[16] = {0};
    uint8_t idx = 0;
    char *p = GNRMC; /* 直接从 GNRMC 开始分割 */
    char *tok = NULL;
    while ((tok = strtok(p, ",")) != NULL && idx < sizeof(tokens) / sizeof(tokens[0]))
    {
        tokens[idx++] = tok; /* 保存每个字段的指针 */
        p = NULL;            /* 后续调用传 NULL 以继续分割同一字符串 */
    }

    /* 解析时间（UTC） */
    if (tokens[1] && strlen(tokens[1]) >= 6)
    {
        char *t = tokens[1];
        locationData.time.hour = (uint8_t)((t[0] - '0') * 10 + (t[1] - '0'));   /* 提取小时 */
        locationData.time.minute = (uint8_t)((t[2] - '0') * 10 + (t[3] - '0')); /* 提取分钟 */
        locationData.time.second = (uint8_t)((t[4] - '0') * 10 + (t[5] - '0')); /* 提取秒 */
    }

    /* 解析纬度 */
    if (tokens[3])
    {
        float latVal = 0.0f;
        /* ddmm.mmmm */
        if (strlen(tokens[3]) >= 4)
        {
            char degbuf[8] = {0};
            /* 纬度前两位为度 */
            strncpy(degbuf, tokens[3], 2);       /* 复制度部分字符串 */
            int deg = atoi(degbuf);              /* 将度部分转换为整数 */
            float minutes = atof(tokens[3] + 2); /* 将分钟及小数部分转换为浮点 */
            latVal = deg + minutes / 60.0f;      /* 转换为十进制度 */
        }
        locationData.latitude = latVal; /* 存储解析后的纬度 */
    }
    /* 纬度方向 */
    if (tokens[4] && tokens[4][0] != '\0')
    {
        locationData.latitude_direction = (tokens[4][0] == 'N') ? 0 : 1; /* 0:N,1:S，保存方向标志 */
    }

    /* 解析经度 */
    if (tokens[5])
    {
        float lonVal = 0.0f;
        /* dddmm.mmmm */
        if (strlen(tokens[5]) >= 5)
        {
            char degbuf[8] = {0};
            /* 经度前三位为度 */
            strncpy(degbuf, tokens[5], 3);       /* 复制度部分字符串 */
            int deg = atoi(degbuf);              /* 将度部分转换为整数 */
            float minutes = atof(tokens[5] + 3); /* 将分钟及小数部分转换为浮点 */
            lonVal = deg + minutes / 60.0f;      /* 转换为十进制度 */
        }
        locationData.longitude = lonVal; /* 存储解析后的经度 */
    }
    /* 经度方向 */
    if (tokens[6] && tokens[6][0] != '\0')
    {
        locationData.longitude_direction = (tokens[6][0] == 'E') ? 0 : 1; /* 0:E,1:W，保存方向标志 */
    }

    /* 解析日期 ddmmyy（UTC 日期） */
    if (idx > 9 && tokens[9] && strlen(tokens[9]) >= 6)
    {
        char *d = tokens[9];
        locationData.calendar.day = (uint8_t)((d[0] - '0') * 10 + (d[1] - '0'));   /* 提取日 */
        locationData.calendar.month = (uint8_t)((d[2] - '0') * 10 + (d[3] - '0')); /* 提取月 */
        locationData.calendar.year = (uint8_t)((d[4] - '0') * 10 + (d[5] - '0'));  /* 提取年（两位） */
    }

    /* ---------- 新增：UTC -> 东八区（UTC+8）本地时间换算，仅用于打印 ---------- */
    uint8_t local_day = locationData.calendar.day;
    uint8_t local_month = locationData.calendar.month;
    uint8_t local_year = locationData.calendar.year; /* 仍使用两位年 */

    uint8_t local_hour = locationData.time.hour + 8; /* +8 小时 */
    uint8_t local_min = locationData.time.minute;
    uint8_t local_sec = locationData.time.second;

    /* 处理进位（本例只有小时偏移，无需处理分钟/秒的借位） */
    while (local_hour >= 24)
    {
        local_hour -= 24;
        /* 日期 +1，处理月末/年末 */
        int dim = days_in_month_yy(local_year, local_month);
        local_day += 1;
        if (local_day > dim)
        {
            local_day = 1;
            local_month += 1;
            if (local_month > 12)
            {
                local_month = 1;
                local_year = (local_year + 1) % 100; /* 仍维持两位年表示 */
            }
        }
    }
    /* ---------- 本地时间换算结束 ---------- */

    /* 将东八区时间存到locationData */
    locationData.calendar.day = local_day;
    locationData.calendar.month = local_month;
    locationData.calendar.year = local_year;
    locationData.time.hour = local_hour;
    locationData.time.minute = local_min;
    locationData.time.second = local_sec;

    /* 成功解析后打印调试信息（使用东八区本地时间） */
    DEBUG_Printf("GNRMC(Local,+8): %02d-%02d-%02d %02d:%02d:%02d, %.6f %s, %.6f %s\r\n",
                 local_day,
                 local_month,
                 local_year,
                 local_hour,
                 local_min,
                 local_sec,
                 locationData.latitude,
                 (locationData.latitude_direction == 0) ? "N" : "S",
                 locationData.longitude,
                 (locationData.longitude_direction == 0) ? "E" : "W"); /* 输出本地时间 */
}

void AT6558R_EnterLowPowerMode(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET); // 拉低GPIOB3引脚，进入低功耗模式
}

void AT6558R_Wakeup(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET); // 拉高GPIOB3引脚，唤醒模块
    HAL_Delay(1000);                                    // 等待100ms，确保模块完全唤醒
}
