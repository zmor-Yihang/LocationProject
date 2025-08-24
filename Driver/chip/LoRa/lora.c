#include "lora.h"

void LORA_Init(void)
{
    SPI_Init();
    GPIOB0_Init();  /* 初始化LoRa芯片RST引脚, 复位 */
    GPIOB1_Init();  /* 初始化LoRa芯片BUSY引脚, 只读 */
    GPIOB2_Init();  /* 初始化LoRa芯片TxEN引脚, 写使能, 默认失能 */
    GPIOB12_Init(); /* 初始化LoRa芯片RxEN引脚, 读使能, 默认失能 */
}


void LORA_Reset(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);  /* 拉高RST引脚，复位LoRa芯片 */
    HAL_Delay(10);                                          /* 等待10毫秒 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);  /* 拉低RST引脚，释放LoRa芯片 */
}