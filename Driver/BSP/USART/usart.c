#include "usart.h"

/* 定义句柄 */
UART_HandleTypeDef huart2;           /* 声明UART2句柄 */
DMA_HandleTypeDef hdma_usart2_rx;    /* 声明USART2接收DMA句柄 */

/* 接收缓冲区 */
uint8_t rxBuffer[RX_BUFFER_SIZE];    /* 实际接收缓冲区 */
volatile uint16_t rxSize = 0;        /* 实际接收到的数据长度 */
volatile uint8_t rxCompleteFlag = 0; /* 接收完成标志 */

/* UART + DMA + 空闲中断初始化函数 */
void USART2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0}; /* GPIO初始化结构体 */
    
    /* 使能USART2、GPIOA、DMA1时钟 */
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
    
    /* 配置USART2 TX引脚PA2为复用推挽输出模式 */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* 配置USART2 RX引脚PA3为浮空输入模式 */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* 初始化UART参数 */
    huart2.Instance = USART2;                        /* 指定USART2外设 */
    huart2.Init.BaudRate = 9600;                   /* 波特率9600 */
    huart2.Init.WordLength = UART_WORDLENGTH_8B;     /* 8位数据位 */
    huart2.Init.StopBits = UART_STOPBITS_1;          /* 1位停止位 */
    huart2.Init.Parity = UART_PARITY_NONE;           /* 无校验 */
    huart2.Init.Mode = UART_MODE_TX_RX;              /* 发送+接收模式 */
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;     /* 无硬件流控 */
    huart2.Init.OverSampling = UART_OVERSAMPLING_16; /* 16倍过采样 */
    HAL_UART_Init(&huart2);                          /* 初始化UART1 */
    
    /* 配置DMA参数用于USART2 RX */
    hdma_usart2_rx.Instance = DMA1_Channel6;                         /* 指定DMA1通道5 */
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;            /* 外设到内存 */
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;                /* 外设地址不自增 */
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;                    /* 内存地址自增 */
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;   /* 外设字节对齐 */
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;      /* 内存字节对齐 */
    hdma_usart2_rx.Init.Mode = DMA_CIRCULAR;                         /* 循环模式 */
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_HIGH;                /* 高优先级 */
    HAL_DMA_Init(&hdma_usart2_rx);                                  /* 初始化DMA */
    
    /* 将DMA句柄与UART句柄进行关联 */
    __HAL_LINKDMA(&huart2, hdmarx, hdma_usart2_rx);
    
    /* 配置DMA和USART2相关中断的优先级和使能 */
    HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);  /* 设置DMA1通道5中断优先级 */
    HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);          /* 使能DMA1通道5中断 */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);         /* 设置USART2中断优先级 */
    HAL_NVIC_EnableIRQ(USART2_IRQn);                 /* 使能USART2中断 */
    
    /* 启动DMA方式接收，准备接收128字节数据到rxBuffer */
    HAL_UART_Receive_DMA(&huart2, rxBuffer, RX_BUFFER_SIZE);
    
    /* 使能UART空闲中断（IDLE），用于变长包分包 */
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
}

/* 发送数据函数 */
void USART2_SendData(uint8_t *data, uint16_t size)
{
    HAL_UART_Transmit(&huart2, data, size, HAL_MAX_DELAY); /* 阻塞方式发送数据 */
}

/* USART2中断服务函数 */
void USART2_IRQHandler(void)
{
    /* 判断是否为IDLE中断（空闲线） */
    if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE) != RESET)
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart2);                       /* 清除IDLE中断标志 */
        HAL_UART_DMAStop(&huart2);                                /* 停止当前DMA接收 */
        rxSize = RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx); /* 计算已接收字节数 */
        rxCompleteFlag = 1;                                       /* 标记接收完成 */
        HAL_UART_Receive_DMA(&huart2, rxBuffer, RX_BUFFER_SIZE);   /* 重新启动DMA接收 */
    }
    HAL_UART_IRQHandler(&huart2); /* 处理HAL库内部其他中断事件 */
}

/* DMA1通道6中断服务函数（USART2 RX DMA） */
void DMA1_Channel6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart2_rx);  /* 调用HAL库DMA RX中断处理函数 */
}