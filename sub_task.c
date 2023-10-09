/*
 * @Descripttion : 子任务任务c文件
 * @version      : 1.0.0
 * @Author       : Lichar
 * @Date         : 2022-10-15 17:44:52
 * @LastEditors  : Lichar
 * @LastEditTime : 2023-02-02 15:58:33
 */

/* Includes ------------------------------------------------------------------*/
#include "sub_task.h"
#include "bldc.h"
#include "debug_printf.h"
#include "main_task.h"

/* Define --------------------------------------------------------------------*/

/* Parameter -----------------------------------------------------------------*/

/* Function ------------------------------------------------------------------*/

/**
 * @brief  电机任务
 * @return Null
 * @note
 */
static void Sub_MotorDrive(void) // 驱动子任务 10ms/time
{
}
/**
 * @brief  交互任务
 * @return Null
 * @note
 */
static void Sub_HMI(void)
{
}
#ifdef DEBUG_PRINTF
/**
 * @brief  打印电机状态
 * @param[in] {ch} 电机状态值
 * @return 电机位置
 * @note
 */
static debug_printf_motor_status(uint8_t ch)
{
   switch (ch)
   {
   case 0x01: // 电机短路状态
      _dbg_printf("{"
                  "info"
                  ":"
                  "short"
                  "}\n");
      break;
   case 0x02: // 电机堵转故障
      _dbg_printf("{"
                  "info"
                  ":"
                  "block"
                  "}\n");
      break;
   default:
      _dbg_printf("{"
                  "info"
                  ":"
                  "runing"
                  "}\n");
      break;
   }
}
#endif
/**
 * @brief  主从机串口通讯模块
 * @return Null
 * @note ：
 * u8MotorRecvBuf[0]:头码0x54
 * u8MotorRecvBuf[1]:从机地址
 * u8MotorRecvBuf[2]:速度(Bit5-Bit0)+方向(Bit7)+割草电机位(Bit6)
 */
static void Sub_WireComm(void)
{
#ifdef DEBUG_PRINTF
   // PAPER_AIRPLANE_PRINT(plotter, "%d,%d", g_sMotor.u16Speed, g_sDrive.u16CurrPwmDuty);
   // 纸飞机打印

   typedef enum
   {
      LEFT_MOTOR = 0x01,
      RIGHT_MOTOR = 0x02,
      BLADE_MOTOR = 0x03,
   } eMotorAddr_enum;

   if (g_sFlagErr.Bits.bShortCurrent) // 电机短路状态
   {
      g_sMotor.u8MotorStatus = 0x01;
   }
   else if (g_sFlagErr.Bits.bMotorBlock) // 电机堵转故障
   {
      g_sMotor.u8MotorStatus = 0x02;
   }
   else
   {
      g_sMotor.u8MotorStatus = 0;
   }

   switch (g_sMotor.u8MotorRecvBuf[1]) // 区分电机进行打印
   {
   case LEFT_MOTOR:
      _dbg_printf("{"
                  "LeftMotor"
                  ":"
                  "%d,%d"
                  "}\n",
                  g_sMotor.u16Speed, g_sDrive.u16CurrPwmDuty);
      _dbg_printf("{"
                  "count"
                  ":"
                  "%d"
                  "}\n",
                  g_sDrive.g_aTzc[0]);
      debug_printf_motor_status(g_sMotor.u8MotorStatus); // 打印电机状态
      break;
   case RIGHT_MOTOR:
      _dbg_printf("{"
                  "rightMotor"
                  ":"
                  "%d,%d"
                  "}\n",
                  g_sMotor.u16Speed, g_sDrive.u16CurrPwmDuty);
      _dbg_printf("{"
                  "count"
                  ":"
                  "%d"
                  "}\n",
                  g_sDrive.g_aTzc[0]);
      debug_printf_motor_status(g_sMotor.u8MotorStatus); // 打印电机状态
      break;
   case BLADE_MOTOR:
      _dbg_printf("{"
                  "bladeMotor"
                  ":"
                  "%d,%d"
                  "}\n",
                  g_sMotor.u16Speed, g_sDrive.u16CurrPwmDuty);
      _dbg_printf("{"
                  "count"
                  ":"
                  "%d"
                  "}\n",
                  g_sDrive.g_aTzc[0]);
      debug_printf_motor_status(g_sMotor.u8MotorStatus); // 打印电机状态
      break;
   default:
      _dbg_printf("unknow device");
      break;
   }

#else
   if (g_sFlagErr.Bits.bShortCurrent) // 电机短路状态
   {
      g_sMotor.u8MotorStatus = 0x01;
   }
   else if (g_sFlagErr.Bits.bMotorBlock) // 电机堵转故障
   {
      g_sMotor.u8MotorStatus = 0x02;
   }
   else if (g_sFlagErr.Bits.bHall) // 电机堵转故障
   {
      g_sMotor.u8MotorStatus = 0x03;
   }
   else
   {
      g_sMotor.u8MotorStatus = 0;
   }
   g_sMotor.u8MotorSendBuf[0] = 0x54;
   g_sMotor.u8MotorSendBuf[1] = g_sMotor.u8MotorRecvBuf[1]; // 设置发送从机地址

   g_sMotor.u8MotorSendBuf[2] = (g_sMotor.u8MotorStatus); // 电机运行状态 g_sBusIdc.RealValue//
   g_sMotor.u8MotorSendBuf[3] = 0xff;
#endif
}
/**
 * @brief  保留任务
 * @return Null
 */
static void Sub_Reserved(void)
{
   ;
}
/**
 * @brief  子任务循环
 * @return Null
 */
void Sub_LoopTask(void)
{
   static uint8_t s_subTaskNum = 0; // 静态局部变量作用域本函数,生存期

   switch (s_subTaskNum)
   {
   case 0:
      Sub_Reserved();
      break; // 0 预留
   case 1:
      Sub_MotorDrive();
      break; // 1 驱动
   case 2:
      break;
   case 3:
      break;
   case 4:
      break;
   case 5:
      Sub_HMI();
      break; // 5 人机接口
   case 6:
      Sub_WireComm();
      break; // 6 有线通讯(UART,CAN,LIN)
   case 7:
      break;
   case 8:
      break;
   case 9:
      Sub_Reserved();
      break; // 9 预留
   default:
      break;
   }
   if (++s_subTaskNum > 9)
      s_subTaskNum = 0;
}
/**
 * @brief  恢复进入工作状态前的变量初始值
 * @return Null
 */
void RstVarBeforInRunSta(void)
{
   g_sMotor.eAct = eSTANDBY_ACT;   // 进入工作状态前置电机动作为待机动作
   g_sMotor.uStaCnt.u8Standby = 0; // 待机状态计数清0
}
/**
 * @brief  恢复进入刹车停机状态前的变量初始值
 * @return Null
 */
void RstVarBeforInStopSta(void)
{
   g_sMotor.u8BrakeTime = 0;
}
/*************************************************END OF FILE*********************************************/
