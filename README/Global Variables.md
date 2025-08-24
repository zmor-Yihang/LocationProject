GPS定位模块
1. **rxBuffer[]：**AT6558R GPS芯片发送的数据通过USART2和DMA1转运在缓冲区 rxBuffer[]中
2. **rxSize：**接收到的存在rxbuffer[]中的数据长度
3. **rxCompleteFlag：**USART2空闲中断触发该标志位置1，标志数据帧接收完成

计步模块
4. **countOfStep：**存储步数的全局变量