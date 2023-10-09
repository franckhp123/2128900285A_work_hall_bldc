
#ifndef __PID_H
#define __PID_H

#include "macrodriver.h"
#include "userdefine.h"

/******************************参数声明***********************************************/
#define SPEED_LastErr           0          
#define SPEED_OUT               0

/******************************定义声明***********************************************/
typedef struct
{
  s32  K1;       // K1 = Kp * (1 + Ki)
  s32  K2;       // K2 = Kp
  s32  lastErr;  // 上一次偏差值 
  s32  out;      // 上一次输出值
  s32  outMin;   // 最小限幅值
  s32  outMax;   // 最大限幅值
  u8   timePIUpGiveSpeed;   //PI环给定速度时间
  u16  PIPremeChgtime;      //PI参数调节时间 
  u8   u8RefSpeedTimeValue; //PI换给定速度改变时基
  u16  u16PILowSpeedTimeVal;   //启动时PI低速运行时间
}S_INC_PI_PARA_T; // 增量式PI参数 

/******************************变量声明***********************************************/
extern  S_INC_PI_PARA_T  g_sSpeedIncPI; //速度增量式PI

/******************************函数声明***********************************************/
s32 IncrementPI_Ctrl(s16 ref,s16 fdb,S_INC_PI_PARA_T *ps);  
void SpeedIncPI_Init(void); 
uint16_t MotorSpeedStepCtrl(uint16_t u16TargetSpeed, uint16_t u16RefSpeed);


#endif



























