/*
 * @Descripttion : 无刷电机操作相关c文件
 * @version      : 1.0.0
 * @Author       : Lichar
 * @Date         : 2022-10-15 17:44:52
 * @LastEditors  : Lichar
 * @LastEditTime : 2023-02-02 15:58:33
 */

/* Includes ------------------------------------------------------------------*/
#include "bldc.h"
#include "main_task.h"
#include "sau.h"
#include "tau.h"
#include "tmrd.h"  
#include "adc.h"

/* Define --------------------------------------------------------------------*/

/* Parameter -----------------------------------------------------------------*/
// 定义hall向量表
const uint8_t HallTable[6] = {1, 3, 2, 6, 4, 5};
// 定义一种方向的pwm输出表
const uint8_t CW_PWM_OUT[6] = {BH_PWM, CH_PWM, CH_PWM, AH_PWM, AH_PWM, BH_PWM}; // BUT MOTOR IS CCW   //VH WH WH UH UH VH
const uint8_t CW_P1_OUT[6] = {AL_IO, AL_IO, BL_IO, BL_IO, CL_IO, CL_IO};        // UL UL VL VL WL WL
// 定义一种方向的pwm输出表
const uint8_t CCW_PWM_OUT[6] = {AH_PWM, AH_PWM, BH_PWM, BH_PWM, CH_PWM, CH_PWM}; // BUT MOTOR IS CW    //UH UH VH VH WH WH
const uint8_t CCW_P1_OUT[6] = {BL_IO, CL_IO, CL_IO, AL_IO, AL_IO, BL_IO};        // VL WL WL UL UL VL

S_MOTOR_T g_sMotor; // 电机相关变量
S_DRIVE_T g_sDrive; // 驱动相关变量

/* Function ------------------------------------------------------------------*/

/**
 * @brief  超前换相使用单步
 * @return Null
 * @note
 */
void UH_VL_HPwmLON(void)
{
   P1 &= 0xc2; // 0b 1100 0010
   P1 |= 0x02;
   TRDOER1 = 0xDF; // UH-VL
}
/**
 * @brief  超前换相使用单步
 * @return Null
 * @note
 */
void UH_WL_HPwmLON(void)
{
   P1 &= 0xd0;
   P1 |= 0x10;
   TRDOER1 = 0xDF; // UH-WL
}
/**
 * @brief  超前换相使用单步
 * @return Null
 * @note
 */
void VH_WL_HPwmLON(void)
{
   P1 &= 0xd0;
   P1 |= 0x10;
   TRDOER1 = 0xEF; // VH-WL
}
/**
 * @brief  超前换相使用单步
 * @return Null
 * @note
 */
void VH_UL_HPwmLON(void)
{
   P1 &= 0xc1;
   P1 |= 0x01;
   TRDOER1 = 0xEF; // VH-UL
}
/**
 * @brief  超前换相使用单步
 * @return Null
 * @note
 */
void WH_UL_HPwmLON(void)
{
   P1 &= 0xc1;
   P1 |= 0x01;
   TRDOER1 = 0xFD; // WH-UL
}
/**
 * @brief  超前换相使用单步
 * @return Null
 * @note
 */
void WH_VL_HPwmLON(void)
{
   P1 &= 0xe0;
   P1 |= 0x02;
   TRDOER1 = 0xFD; // WH-VL
}
/**
 * @brief  无相线
 * @return Null
 * @note
 */
void NullPhase(void)
{
   return;
}
/**
 * @brief  电机能耗制动,开下三管
 * @return Null
 * @note
 */
void BLDC_ConsumeBrake(void)
{
   BLDC_StopMotor();
   TRDOER1 |= 0xFA;
   P1 &= 0xc0;
   P1 |= 0x13; // 开下三MOS
}
/**
 * @brief  电机急停
 * @return Null
 * @note
 */
void BLDC_StopMotor(void)
{
   TRDOER1 |= 0xFA;  // 关闭定时器输出,各PWM口为普通IO口(无PWM波形,中断仍会触发)
   P1 &= 0xc0;       // 拉低各端口
   HALL_INT_OFF();   // 关闭hall中断
   OUTPUT_PWM_OFF(); // 清除pwm寄存器的值
}
/**
 * @brief  关闭六个管子
 * @return Null
 * @note
 */
void PwmDrv_PwmOutDisable(void)
{
   P1 &= 0xc0;
   TRDOER1 = 0xFA; // 0xc8=开所有PWM 0xFA=关PWM(只是pwm没了波形,中断还会触发)
}
/**
 * @brief  电机速度计算
 * @param[in] {*psMotor} 电机结构体
 * @param[in] {aZeroCrossTime} 过零时间
 * @return 结果
 * @note   采用过零的算法进行计算
 */
u8 BLDC_u8ZeroCrossTimeToSpeed(S_MOTOR_T *psMotor, u16 aZeroCrossTime[])
{
   u16 u16SpeedBuf = 0;

   if (aZeroCrossTime[0] != 0 && aZeroCrossTime[1] != 0 && aZeroCrossTime[2] != 0)
   {
      u16SpeedBuf = (u32)(22500000) / (aZeroCrossTime[0] + aZeroCrossTime[1] + aZeroCrossTime[2]);
      psMotor->sSpeedFilt.u32Sum += u16SpeedBuf;
      psMotor->sSpeedFilt.u8Cnt++;
      if (psMotor->sSpeedFilt.u8Cnt == 1)
      {
         psMotor->sSpeedFilt.u16Max = u16SpeedBuf;
         psMotor->sSpeedFilt.u16Min = u16SpeedBuf;
      }
      else
      {
         if (u16SpeedBuf > psMotor->sSpeedFilt.u16Max)
         {
            psMotor->sSpeedFilt.u16Max = u16SpeedBuf;
         }
         else if (u16SpeedBuf < psMotor->sSpeedFilt.u16Min)
         {
            psMotor->sSpeedFilt.u16Min = u16SpeedBuf;
         }
      }
      if (psMotor->sSpeedFilt.u8Cnt >= 6)
      {
         psMotor->u16Speed = (u16)((psMotor->sSpeedFilt.u32Sum - psMotor->sSpeedFilt.u16Max - psMotor->sSpeedFilt.u16Min) >> 2);
         psMotor->sSpeedFilt.u8Cnt = 0;
         psMotor->sSpeedFilt.u32Sum = 0;
         return 1;
      }
      else
      {
         return 0;
      }
   }
   else
   {
      return 0;
   }
}
/**
 * @brief  获取hall值
 * @return hall值
 * @note
 */
uint8_t Hall_Get(void)
{
   volatile uint8_t curhallvalue;

   if (HALL_V == 1)
      curhallvalue = 0x04;
   else
      curhallvalue = 0;
   if (HALL_U == 1)
      curhallvalue |= 0x02;
   if (HALL_W == 1)
      curhallvalue |= 0x01;

   return curhallvalue;
}
/**
 * @brief  通过hall值获取电机位置
 * @param[in] {hallvalue} 输入hall值
 * @return 电机位置
 * @note
 */
uint8_t GetMotorPosition(uint8_t hallvalue)
{
   uint8_t i;
   for (i = 0; i < 6; i++)
   {
      if (HallTable[i] == hallvalue)
         return i;
   }
   return 0xff;
}
/**
 * @brief  换向函数
 * @param[in] {Position} 输入电机位置值
 * @return
 * @note
 */
void Commutate_Phase(uint8_t Position)
{
   if (g_sDrive.MotorDirection == CCW) // 6->4->5->1->3->2
   {
      TRDOER1 = CCW_PWM_OUT[Position];         // 使能pwm波形
      P1 = (P1 & 0xc0) | CCW_P1_OUT[Position]; // 设置下桥io
   }
   else if (g_sDrive.MotorDirection == CW) // 2->3->1->5->4->6
   {
      TRDOER1 = CW_PWM_OUT[Position];         // 使能pwm波形
      P1 = (P1 & 0xc0) | CW_P1_OUT[Position]; // 设置下桥io
   }
}
/**
 * @brief  电容充电
 * @param[in] {Position} 输入电机位置值
 * @return
 * @note
 */
void Cap_PreCharge(uint8_t Position)
{
   TRDOER1 = 0XFF; // 禁止输出
   if (g_sDrive.MotorDirection == CW)
   {
      P1 |= CW_P1_OUT[Position];
   }
   else
   {
      P1 |= CCW_P1_OUT[Position];
   }
}
/**
 * @brief  期望的hall值
 * @param[in] {Position} 输入电机位置值
 * @return hall值
 * @note
 */
uint8_t Expect_Hall_Value(uint8_t Position)
{
   uint8_t NextHallValue;
   if (g_sDrive.MotorDirection == CCW)
   {
      NextHallValue = HallTable[(Position + 5) % 6];
   }
   else if (g_sDrive.MotorDirection == CW)
   {
      NextHallValue = HallTable[(Position + 1) % 6];
   }
   return NextHallValue;
}
/**
 * @brief  hall处理函数
 * @return
 * @note 放在中断中
 */
void HallISR(void)
{
#define HALL_PROTECT_FAIL_COUNT_MAX 60      // 换向错误最大次数
#define HALL_PROTECT_FAIL_RESET_TIME 10000 // 换向错误恢复时间

   if (g_eSysState == cSTA_RUN && (g_sMotor.eAct == eRUN_ACT || g_sMotor.eAct == eSTART_ACT)) // 在运行状态中进行处理
   {
      static uint8_t hall_fail_count = 0;
      static uint16_t hall_reset_fail_count = 0; // 换向错误复位计数器

      g_sDrive.hall_isr_flag = 1; // 置位hall中断标志位

      g_sDrive.uHallValue = Hall_Get();                               // 获取hall值
      g_sDrive.MotorPosition = GetMotorPosition(g_sDrive.uHallValue); // 获取电机位置
      Commutate_Phase(g_sDrive.MotorPosition);                        // 输出对应波形

      /****************hall乱相保护***************/
      if (g_sDrive.uHallValue != g_sDrive.uHallValueExpect)      // 如果当前值不等于期望的hall值
      {
         if (++hall_fail_count >= HALL_PROTECT_FAIL_COUNT_MAX)
         {
            g_sFlagErr.Bits.bHall = 1; // 置位hall乱相保护
            R_TMRD0_Stop();
            R_TAU0_Channel0_Stop(); // CH0换相时间监控 CH1延时换相设定
            R_ADC_Stop();
            hall_fail_count = 0;
         }
      }
      // 超过一定时间后重置报错次数，保证错误连续,但是只有在真正运行的时候才能删除
      if (++hall_reset_fail_count >= HALL_PROTECT_FAIL_RESET_TIME)
      {
         hall_reset_fail_count = 0; // 重置数据
         hall_fail_count = 0;       // 重置数据
      }
      /****************计算换相间隔时间***************/
      g_sDrive.g_u16NowZeroTimerVal = TCR00; // 读取本次过零点时刻值
      g_sDrive.g_aTzc[2] = g_sDrive.g_aTzc[1];
      g_sDrive.g_aTzc[1] = g_sDrive.g_aTzc[0];
      if (g_sDrive.g_u16LastZeroTimerVal >= g_sDrive.g_u16NowZeroTimerVal) // 上次过零时刻点>本次过零时刻点
      {
         g_sDrive.g_aTzc[0] = g_sDrive.g_u16LastZeroTimerVal - g_sDrive.g_u16NowZeroTimerVal;
      }
      else
      {
         g_sDrive.g_aTzc[0] = _FFFF_TAU_TDR00_VALUE - g_sDrive.g_u16NowZeroTimerVal + g_sDrive.g_u16LastZeroTimerVal;
      }
      g_sDrive.g_u16LastZeroTimerVal = g_sDrive.g_u16NowZeroTimerVal; // 存贮本次过零点时刻值(作为上次过零时刻值)
      /**************************************************/
      // 获得期望的hall值
      g_sDrive.uHallValueExpect = Expect_Hall_Value(g_sDrive.MotorPosition);
   }
}
/**
 * @brief  更新占空比-pid控制器
 * @param[in] {duty} 占空比
 * @return
 * @note
 */
void PWM_PidDutyUpdata(u16 duty)
{
   if (duty >= cDUTY_FULL)
      duty = cDUTY_FULL;
   else if (duty <= cDUTY_MIN)
      duty = cDUTY_MIN; // 10%最小占空比

   if (duty > TRDGRA0)
   {
      duty = TRDGRA0;
   }
   else
   {
      duty = TRDGRA0 - duty;
   }

   TRDMR &= 0x11; // D1 C1 D0作为通用寄存器; C0作为A0缓冲器
   TRDGRD0 = duty;
   TRDGRC1 = duty;
   TRDGRD1 = duty;
   TRDMR |= 0xe0; // D1作为B1(UH)缓冲器; C1作为A1(VH)缓冲器; D0作为B0(WH)缓冲器;
}
/**
 * @brief  更新占空比-启动控制器
 * @param[in] {duty} 占空比
 * @return
 * @note
 */
void PWM_StartDutyUpdata(u16 duty)
{
   if (duty >= cDUTY_FULL)
      duty = cDUTY_FULL;

   if (duty > TRDGRA0)
   {
      duty = TRDGRA0;
   }
   else
   {
      duty = TRDGRA0 - duty;
   }

   TRDMR &= 0x11; // D1 C1 D0作为通用寄存器; C0作为A0缓冲器
   TRDGRD0 = duty;
   TRDGRC1 = duty;
   TRDGRD1 = duty;
   TRDMR |= 0xe0; // D1作为B1(UH)缓冲器; C1作为A1(VH)缓冲器; D0作为B0(WH)缓冲器;
}
/*************************************************END OF FILE*********************************************/
