/**
 * @file    lora.c
 * @brief   LLCC68 LoRa无线通信芯片驱动程序
 * @details LLCC68是一款低功耗长距离LoRa无线通信芯片，本驱动程序实现了：
 *          - 芯片初始化和参数配置
 *          - 发送和接收模式切换
 *          - 数据发送和接收功能
 *          - GPIO状态指示控制
 * @note    使用前需要初始化SPI接口和相关GPIO引脚
 */

#include "lora.h"

/* 全局变量：LLCC68芯片操作句柄，包含所有必要的接口函数指针 */

llcc68_handle_t gs_handle = {
    .spi_init = llcc68_interface_spi_init,                    /* SPI接口初始化函数 */
    .spi_deinit = llcc68_interface_spi_deinit,                /* SPI接口反初始化函数 */
    .spi_write_read = llcc68_interface_spi_write_read,        /* SPI读写操作函数 */
    .reset_gpio_init = llcc68_interface_reset_gpio_init,      /* 复位GPIO初始化函数 */
    .reset_gpio_deinit = llcc68_interface_reset_gpio_deinit,  /* 复位GPIO反初始化函数 */
    .reset_gpio_write = llcc68_interface_reset_gpio_write,    /* 复位GPIO写入函数 */
    .busy_gpio_init = llcc68_interface_busy_gpio_init,        /* 忙碌GPIO初始化函数 */
    .busy_gpio_deinit = llcc68_interface_busy_gpio_deinit,    /* 忙碌GPIO反初始化函数 */
    .busy_gpio_read = llcc68_interface_busy_gpio_read,        /* 忙碌GPIO读取函数 */
    .debug_print = llcc68_interface_debug_print,              /* 调试打印函数 */
    .delay_ms = llcc68_interface_delay_ms,                    /* 毫秒延时函数 */
    .receive_callback = llcc68_interface_receive_callback,    /* 接收回调函数 */
}; /**< LLCC68芯片操作句柄 */

/**
 * @brief   初始化LoRa通信模块
 * @details 完成LLCC68芯片的完整初始化流程，包括：
 *          1. 初始化芯片基本功能和接口
 *          2. 设置待机模式和时钟源
 *          3. 配置前导码和电源管理参数
 *          4. 设置中断和状态清除
 *          5. 配置LoRa调制参数和发射功率
 *          6. 设置RF频率和缓冲区地址
 *          7. 配置同步字和接收增益
 *          8. 进入接收模式
 * @param   None
 * @retval  None
 * @note    此函数必须在使用其他LoRa相关函数前调用
 *          初始化过程中如有任何步骤失败，会打印错误信息
 * @warning 确保SPI和GPIO引脚的时钟已使能，否则初始化会失败
 */
void LORA_Init(void)
{
    uint8_t res;
    uint32_t reg;
    uint8_t modulation;
    uint8_t config;

    /* 初始化LLCC68芯片基本功能 */
    res = llcc68_init(&gs_handle);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: init failed.\n");
    }
    
    /* 设置芯片进入待机模式，使用32MHz晶振 */
    res = llcc68_set_standby(&gs_handle, LLCC68_CLOCK_SOURCE_XTAL_32MHZ);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set standby failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 设置前导码检测时是否停止计时器 */
    res = llcc68_set_stop_timer_on_preamble(&gs_handle, LLCC68_LORA_DEFAULT_STOP_TIMER_ON_PREAMBLE);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: stop timer on preamble failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 设置电源调节器工作模式 */
    res = llcc68_set_regulator_mode(&gs_handle, LLCC68_LORA_DEFAULT_REGULATOR_MODE);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set regulator mode failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 配置功率放大器参数以实现目标输出功率 */
    res = llcc68_set_pa_config(&gs_handle, LLCC68_LORA_DEFAULT_PA_CONFIG_DUTY_CYCLE, LLCC68_LORA_DEFAULT_PA_CONFIG_HP_MAX);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set pa config failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 设置接收/发送失败后的回退模式为待机模式 */
    res = llcc68_set_rx_tx_fallback_mode(&gs_handle, LLCC68_RX_TX_FALLBACK_MODE_STDBY_XOSC);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set rx tx fallback mode failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 配置DIO引脚中断参数，启用所有中断 */
    res = llcc68_set_dio_irq_params(&gs_handle, 0x03FF, 0x03FF, 0x0000, 0x0000);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set dio irq params failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 清除所有中断状态位 */
    res = llcc68_clear_irq_status(&gs_handle, 0x03FF);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: clear irq status failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 设置数据包类型为LoRa模式 */
    res = llcc68_set_packet_type(&gs_handle, LLCC68_PACKET_TYPE_LORA);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set packet type failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 设置发射功率和功率爬升时间 */
    res = llcc68_set_tx_params(&gs_handle, LLCC68_LORA_DEFAULT_TX_DBM, LLCC68_LORA_DEFAULT_RAMP_TIME);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set tx params failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 设置LoRa调制参数：扩频因子、带宽、编码率、低数据率优化 */
    res = llcc68_set_lora_modulation_params(&gs_handle, LLCC68_LORA_DEFAULT_SF, LLCC68_LORA_DEFAULT_BANDWIDTH, 
                                            LLCC68_LORA_DEFAULT_CR, LLCC68_LORA_DEFAULT_LOW_DATA_RATE_OPTIMIZE);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set lora modulation params failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 将RF频率转换为寄存器值 */
    res = llcc68_frequency_convert_to_register(&gs_handle, LLCC68_LORA_DEFAULT_RF_FREQUENCY, (uint32_t *)&reg);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: convert to register failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 设置RF工作频率 */
    res = llcc68_set_rf_frequency(&gs_handle, reg);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set rf frequency failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 设置发送和接收缓冲区的基地址 */
    res = llcc68_set_buffer_base_address(&gs_handle, 0x00, 0x00);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set buffer base address failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 设置LoRa符号数超时参数 */
    res = llcc68_set_lora_symb_num_timeout(&gs_handle, LLCC68_LORA_DEFAULT_SYMB_NUM_TIMEOUT);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set lora symb num timeout failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 重置芯片统计信息 */
    res = llcc68_reset_stats(&gs_handle, 0x0000, 0x0000, 0x0000);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: reset stats failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 清除设备错误状态 */
    res = llcc68_clear_device_errors(&gs_handle);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: clear device errors failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 设置LoRa同步字，用于网络识别 */
    res = llcc68_set_lora_sync_word(&gs_handle, LLCC68_LORA_DEFAULT_SYNC_WORD);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set lora sync word failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 获取当前发射调制配置 */
    res = llcc68_get_tx_modulation(&gs_handle, (uint8_t *)&modulation);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: get tx modulation failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    modulation |= 0x04;  /* 设置特定的调制位 */
    
    /* 应用修改后的发射调制配置 */
    res = llcc68_set_tx_modulation(&gs_handle, modulation);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set tx modulation failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 设置接收增益以优化接收灵敏度 */
    res = llcc68_set_rx_gain(&gs_handle, LLCC68_LORA_DEFAULT_RX_GAIN);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set rx gain failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 设置过流保护阈值 */
    res = llcc68_set_ocp(&gs_handle, LLCC68_LORA_DEFAULT_OCP);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set ocp failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    
    /* 获取当前发射钳位配置 */
    res = llcc68_get_tx_clamp_config(&gs_handle, (uint8_t *)&config);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: get tx clamp config failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }
    config |= 0x1E;  /* 设置发射钳位参数 */
    
    /* 应用修改后的发射钳位配置 */
    res = llcc68_set_tx_clamp_config(&gs_handle, config);
    if (res != 0)
    {
        llcc68_interface_debug_print("llcc68: set tx clamp config failed.\n");
        (void)llcc68_deinit(&gs_handle);
    }

    /* 初始化完成后进入接收模式 */
    LORA_EnterReceiveMode();
}

/**
 * @brief   进入LoRa发送模式
 * @details 配置芯片进入发送状态，并设置相关GPIO控制引脚和中断：
 *          1. 控制状态指示LED：拉低接收指示灯，点亮发送指示灯
 *          2. 配置发送相关的DIO中断参数
 *          3. 清除所有中断状态位
 * @param   None
 * @retval  uint8_t 操作结果
 *          - 0: 成功进入发送模式
 *          - 1: 设置中断参数或清除中断状态失败
 * @note    发送完成后建议调用LORA_EnterReceiveMode()恢复接收模式
 *          GPIO控制：GPIOB1-接收指示灯，GPIOB2-发送指示灯
 * @warning 发送前确保芯片已正确初始化，且数据准备就绪
 */
uint8_t LORA_EnterSendMode(void)
{
    /* 控制状态指示GPIO：关闭接收指示灯 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    /* 控制状态指示GPIO：打开发送指示灯 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);

    /* 配置发送模式相关的DIO中断：发送完成、超时、CAD完成、CAD检测 */
    if (llcc68_set_dio_irq_params(&gs_handle, LLCC68_IRQ_TX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                  LLCC68_IRQ_TX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                  0x0000, 0x0000) != 0)
    {
        return 1;
    }
    
    /* 清除所有中断状态位，确保干净的中断环境 */
    if (llcc68_clear_irq_status(&gs_handle, 0x03FFU) != 0)
    {
        return 1;
    }
    
    return 0;
}

/**
 * @brief   进入LoRa接收模式
 * @details 配置芯片进入连续接收状态，包括以下步骤：
 *          1. 控制状态指示LED：拉低发送指示灯，点亮接收指示灯
 *          2. 配置接收相关的DIO中断参数
 *          3. 清除所有中断状态位
 *          4. 设置LoRa数据包参数
 *          5. 配置IQ极性设置
 *          6. 启动连续接收模式
 * @param   None
 * @retval  uint8_t 操作结果
 *          - 0: 成功进入接收模式
 *          - 1: 配置过程中任一步骤失败
 * @note    这是芯片的默认工作模式，初始化完成后会自动进入
 *          GPIO控制：GPIOB1-接收指示灯，GPIOB2-发送指示灯
 * @warning 接收过程中避免频繁切换模式，可能影响接收性能
 */
uint8_t LORA_EnterReceiveMode(void)
{
    /* 控制状态指示GPIO：关闭发送指示灯 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
    /* 控制状态指示GPIO：打开接收指示灯 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);

    uint8_t setup;
    
    /* 配置接收模式相关的DIO中断：接收完成、超时、CRC错误、CAD完成、CAD检测 */
    if (llcc68_set_dio_irq_params(&gs_handle, LLCC68_IRQ_RX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CRC_ERR | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                  LLCC68_IRQ_RX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CRC_ERR | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                  0x0000, 0x0000) != 0)
    {
        return 1;
    }
    
    /* 清除所有中断状态位，确保干净的中断环境 */
    if (llcc68_clear_irq_status(&gs_handle, 0x03FFU) != 0)
    {
        return 1;
    }
    
    /* 设置LoRa数据包参数：前导码长度、头部类型、缓冲区大小、CRC、IQ极性 */
    if (llcc68_set_lora_packet_params(&gs_handle, LLCC68_LORA_DEFAULT_PREAMBLE_LENGTH,
                                      LLCC68_LORA_DEFAULT_HEADER, LLCC68_LORA_DEFAULT_BUFFER_SIZE,
                                      LLCC68_LORA_DEFAULT_CRC_TYPE, LLCC68_LORA_DEFAULT_INVERT_IQ) != 0)
    {
        return 1;
    }
    
    /* 获取当前IQ极性配置 */
    if (llcc68_get_iq_polarity(&gs_handle, (uint8_t *)&setup) != 0)
    {
        return 1;
    }
    
    /* 根据默认配置调整IQ极性设置 */
#if LLCC68_LORA_DEFAULT_INVERT_IQ == LLCC68_BOOL_FALSE
    setup |= 1 << 2;   /* 不反转IQ极性时设置bit2 */
#else
    setup &= ~(1 << 2); /* 反转IQ极性时清除bit2 */
#endif
    
    /* 应用修改后的IQ极性配置 */
    if (llcc68_set_iq_polarity(&gs_handle, setup) != 0)
    {
        return 1;
    }
    
    /* 启动连续接收模式，芯片将持续监听信道 */
    if (llcc68_continuous_receive(&gs_handle) != 0)
    {
        return 1;
    }
    
    return 0;
}

/**
 * @brief   发送LoRa数据
 * @details 通过LoRa芯片发送指定长度的数据包，完整流程包括：
 *          1. 切换芯片到发送模式
 *          2. 调用底层传输函数发送数据
 *          3. 发送失败时恢复到接收模式
 *          4. 发送成功后自动恢复接收模式
 * @param   sendDataBuffer 指向要发送数据的缓冲区指针
 * @param   length 要发送的数据字节数 (1-255字节)
 * @retval  uint8_t 发送结果
 *          - 0: 发送成功
 *          - 1: 发送失败（模式切换失败或数据传输失败）
 * @note    发送完成后会自动恢复到接收模式
 *          使用默认的LoRa参数进行数据传输
 * @warning 确保sendDataBuffer指向的内存区域至少有length字节的有效数据
 *          发送期间芯片无法接收数据
 */
uint8_t LORA_SendData(uint8_t *sendDataBuffer, uint16_t length)
{
    /* 切换芯片到发送模式 */
    LORA_EnterSendMode();

    /* 使用LoRa协议发送数据，配置前导码、头部、CRC、IQ极性等参数 */
    if (llcc68_lora_transmit(&gs_handle, LLCC68_CLOCK_SOURCE_XTAL_32MHZ,
                             LLCC68_LORA_DEFAULT_PREAMBLE_LENGTH, LLCC68_LORA_DEFAULT_HEADER,
                             LLCC68_LORA_DEFAULT_CRC_TYPE, LLCC68_LORA_DEFAULT_INVERT_IQ,
                            (uint8_t *)sendDataBuffer, length, 0) != 0)
    {
        /* 发送失败时立即恢复接收模式 */
        LORA_EnterReceiveMode();
        return 1;
    }
    
    /* 发送成功，返回结果 */
    return 0;

    /* 发送完成后恢复接收模式 */
    LORA_EnterReceiveMode();
}

/**
 * @brief   接收LoRa数据
 * @details 处理LoRa芯片的接收中断并获取接收到的数据：
 *          1. 调用中断处理函数处理芯片状态
 *          2. 检查芯片内部接收缓冲区是否有新数据
 *          3. 如有数据则复制到用户提供的缓冲区
 *          4. 自动添加字符串结束符便于文本处理
 *          5. 清空芯片内部缓冲区准备下次接收
 * @param   receiveDataBuffer 指向存储接收数据的缓冲区指针
 * @param   length 指向存储接收数据长度的变量指针
 * @retval  uint8_t 接收处理结果
 *          - 0: 处理成功（有无数据都返回0）
 *          - 1: 中断处理失败
 * @note    需要定期调用此函数以处理接收到的数据
 *          接收到的数据会自动添加字符串结束符'\0'
 *          函数返回成功不代表一定有新数据，需检查length值
 * @warning 确保receiveDataBuffer有足够空间存储接收的数据（建议至少256字节）
 *          调用前确保芯片处于接收模式
 */
uint8_t LORA_ReceiveData(uint8_t *receiveDataBuffer, uint16_t *length)
{
    /* 处理芯片中断状态，更新接收缓冲区 */
    if(llcc68_irq_handler(&gs_handle) != 0)
    {
        return 1;
    }

    /* 检查是否有新的接收数据 */
    if(gs_handle.receive_len > 0)
    {
        /* 将芯片内部缓冲区数据复制到用户缓冲区 */
        memcpy(receiveDataBuffer, gs_handle.receive_buf, gs_handle.receive_len);
        /* 返回接收到的数据长度 */
        *length = gs_handle.receive_len;
        /* 添加字符串结束符，便于文本数据处理 */
        receiveDataBuffer[gs_handle.receive_len] = '\0';
        /* 清空芯片内部接收缓冲区 */
        memset(gs_handle.receive_buf, 0, sizeof(gs_handle.receive_buf));
        /* 重置接收数据长度计数 */
        gs_handle.receive_len = 0;
    }
    return 0;
}