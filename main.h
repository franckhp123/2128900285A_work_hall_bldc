
#ifndef __MAIN_H
#define __MAIN_H

#include "macrodriver.h"

/******************************参数声明***********************************************/

#define LED_ON()      P1_bit.no6 = 1
#define LED_OFF()     P1_bit.no6 = 0
#define LED_REV()     P1_bit.no6 = !(P1_bit.no6)
/******************************定义声明***********************************************/

/******************************变量声明***********************************************/
extern uint8_t Flag_1ms;

/******************************函数声明***********************************************/

#endif


























