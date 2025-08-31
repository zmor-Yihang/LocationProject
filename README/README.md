# LocationProject - 低功耗GPS定位追踪系统

## 项目概述

本项目是一个基于STM32F103C8T6微控制器的低功耗GPS定位追踪系统。系统集成了GPS定位、NB-IoT通信、计步监测和LoRa无线通信功能，采用周期性唤醒工作模式，实现位置追踪、步数统计和数据远程传输。

## 系统架构

### 硬件平台
- **主控芯片**: STM32F103C8T6 (ARM Cortex-M3)
- **GPS模块**: AT6558R (支持GPS+北斗双模定位)
- **通信模块**: QS100 (NB-IoT模组)
- **计步器**: DS3553 (超低功耗加速度计步器)
- **LoRa模块**: LLCC68 (可选无线通信)

### 软件架构
```
LocationProject/
├── APP/                    # 应用层
│   ├── location/          # 定位功能模块
│   └── lowPower/          # 低功耗管理
├── Driver/                # 驱动层
│   ├── BSP/               # 板级支持包
│   ├── HAL_Driver/        # STM32 HAL库
│   └── chip/              # 芯片驱动
│       ├── at6558r/       # GPS驱动
│       ├── qs100/         # NB-IoT驱动
│       ├── ds3553/        # 计步器驱动
│       └── LoRa/          # LoRa驱动
├── System/                # 系统模块
├── User/                  # 用户代码
└── README/                # 项目文档
```

## 功能特性

### 1. 定位功能
- 支持GPS和北斗双模定位
- NMEA协议数据解析
- 自动时区转换(东八区)
- 定位数据完整性验证
- 低功耗定位模式

### 2. 计步功能
- 24位步数计数器
- I2C接口通信
- 超低功耗设计
- 实时步数更新

### 3. 数据通信
- NB-IoT网络连接
- JSON格式数据传输
- 服务器IP: 112.125.89.8:43458 (http://netlab.luatos.com/)
- 自动重连机制

### 4. 低功耗管理
- RTC定时唤醒
- 待机功耗 (待实测)
- 外设独立电源控制 (锂电池)
- 休眠策略 (RTC闹钟唤醒 + 低功耗模式)

## 数据格式

### JSON数据包结构
```json
{
  "date": "2024-01-15",
  "time": "14:30:25",
  "latitude": 31.2304,
  "longitude": 121.4737,
  "steps": 12580,
  "deviceId": "device_001"
}
```

### 数据字段说明
| 字段名 | 类型 | 描述 |
|--------|------|------|
| date | string | 日期 (YYYY-MM-DD) |
| time | string | 时间 (HH:MM:SS) |
| latitude | float | 纬度 (度) |
| longitude | float | 经度 (度) |
| steps | int | 步数计数 |
| deviceId | string | 设备唯一标识 |

## 硬件接口配置

### USART配置
- **USART2**: GPS模块 (AT6558R)
  - 波特率: 9600 bps
  - 引脚: PA2(TX), PA3(RX)
  - DMA: 通道6循环模式

- **USART3**: NB-IoT模块 (QS100)
  - 波特率: 115200 bps
  - 引脚: PB10(TX), PB11(RX)

### I2C配置
- **I2C1**: 计步器 (DS3553)
  - 时钟: 100 kHz
  - 引脚: PB6(SCL), PB7(SDA)
  - 片选: PB5

### SPI配置
- **SPI1**: LoRa模块 (LLCC68)
  - 时钟: 8 MHz
  - 引脚: PA5(SCK), PA6(MISO), PA7(MOSI)
  - 片选: PA4

## 软件参数

### 工作参数
- **唤醒周期**: 20秒
- **定位超时**: 30秒
- **网络超时**: 10秒
- **重试次数**: 3次

### 调试配置
- **调试接口**: USART1 (115200 bps)
- **调试开关**: DEBUG_ENABLE宏定义
- **GNRMC演示**: ENABLE_GNRMC_DEMO宏定义

## 性能指标

### 功耗特性
- **运行模式**: 
- **休眠模式**: 
- **唤醒时间**: 
- **定位时间**: 冷启动<35s, 热启动<1s

### 通信性能
- **NB-IoT频段**: B3/B5/B8
- **数据传输**: TCP协议
- **数据包大小**: <200字节
- **网络延迟**: <1s

## 使用说明

### 1. 硬件连接
按照硬件接口配置连接各模块，确保电源稳定。

### 2. 软件配置
修改`user_config.h`中的服务器配置：
```c
#define SERVER_IP   "112.125.89.8"
#define SERVER_PORT 43458
```

### 3. 编译烧录
使用Keil MDK或IAR编译项目，通过ST-Link烧录到STM32。

### 4. 调试测试
通过串口调试工具查看调试信息，确认各模块工作正常。

## 开发环境

### 工具链
- **IDE**: Keil MDK 5.38 或 IAR Embedded Workbench
- **编译器**: ARMCC 5.06 或 ARMCLANG 6.16
- **调试器**: ST-Link V2
- **串口工具**: 任意串口调试工具

### 依赖库
- **STM32 HAL库**: V1.1.8
- **cJSON库**: V1.7.15

## 文件结构

### 核心文件
- `main.c`: 主程序入口
- `location.c/h`: 定位功能实现
- `lowPower.c/h`: 低功耗管理
- `user_config.h`: 用户配置

### 驱动文件
- `at6558r.c/h`: GPS驱动
- `qs100.c/h`: NB-IoT驱动
- `ds3553.c/h`: 计步器驱动
- `lora.c/h`: LoRa驱动

### 系统文件
- `usart.c/h`: 串口驱动
- `delay.c/h`: 延时函数
- `debug.c/h`: 调试接口
- `cJSON.c/h`: JSON解析

### 常见问题
1. **GPS无法定位**
   - 室内信号不好
   - 确认GPS模块供电
   - 查看GNRMC数据输出

2. **NB-IoT连接失败**
   - 检查SIM卡状态
   - 确认网络信号强度
   - 验证服务器地址 http://netlab.luatos.com/ 重启后会改变端口号

3. **计步器无数据**
   - 检查I2C连接
   - 确认器件地址
   - 验证电源供电

4. **AT6558R收不到数据**
   - PB3是AT6558R的RST引脚，stm32默认PB3是Jlink调试端口，需要重映射为普通GPIO输出

### 调试信息
开启DEBUG_ENABLE宏定义后，可通过USART1查看详细调试信息：
- GPS数据解析状态
- NB-IoT网络连接
- 计步器数据读取
- 低功耗模式切换

## 版本历史

### V1.0.0 (2025-08-31)
- 初始版本发布
- 实现基本定位功能
- 支持NB-IoT通信
- 集成计步器
- 低功耗管理
- LoRa通信待进一步实现
