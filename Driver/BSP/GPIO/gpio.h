#ifndef __GPIO_H__
#define __GPIO_H__

#include "sys/sys.h"

void GPIOB3_Init(void);     /*  GPS芯片通信使能引脚 */

void GPIOB5_Init(void);     /*  DS3553计步芯片片选引脚 */

void GPIOB0_Init(void);     /*  LoRa芯片RST引脚, 复位 */
void GPIOB1_Init(void);     /*  LoRa芯片BUSY引脚, 只读 */
void GPIOB2_Init(void);     /*  LoRa芯片TxEN引脚, 写使能 */
void GPIOB12_Init(void);    /*  LoRa芯片RxEN引脚, 读使能 */

void GPIOB13_Init(void);    /*  QS100芯片Wakeup引脚, 唤醒 */


#endif
