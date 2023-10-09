/*-------------------------------------------------------------------------------------------------------------
  @Function: IncrementPI_Ctrl
  @Brief: 增量式PI控制器
  @Description: 根据当前值与预设目标值,采用增量式PI算法计算后得到对应的输出值
                △Uk = Kp(e(k) - e(k-1)) + e(k)*Kp*T/Ti
                     = Kp(1+T/Ti)*e(k) - Kp*e(k-1)
                     = K1*e(k) + K2*e(k-1)
                K1 = Kp(1 + T/Ti) = Kp(1 + T/kT) = Kp(1 + 1/k) = Kp(1 + Ki)
                K2 = -Kp
                U(k) = U(k-1) + △U(k)
  @Calls:
  @Called By: while(1)中maintask子任务
  @Input:  当前值、预设目标值、PI参数结构体指针
  @Output: null
  @Return: PI控制器输出值
  @Remark:
  @History:
  <Date> <Author> <Version> <Description>
  2008.10.20,Lichar,V1,初版
  2008.01.10,Lichar,V2,修改PI输出会溢出的BUG(用于限幅输出的值和累加的输出不是同一个值)
-------------------------------------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "pid.h"
#include "BLDC.h"
#include "main_task.h"

/* Define --------------------------------------------------------------------*/

/* Parameter -----------------------------------------------------------------*/
S_INC_PI_PARA_T g_sSpeedIncPI; // 速度增量式PI

/* Function ------------------------------------------------------------------*/

/**
 * @brief  pid控制器
 * @param[in] {ref} 目标值
 * @param[in] {fdb} 实际值
 * @param[in] {ps} pid结构体
 * @return
 * @note
 */
s32 IncrementPI_Ctrl(s16 ref, s16 fdb, S_INC_PI_PARA_T *ps)
{
   s32 err = 0;

   err = ref - fdb;                                  // 目标值-当前值:得出本次偏差
   ps->out += (ps->K1 * err + ps->K2 * ps->lastErr); // 上一次输出值=k1*本次偏差+k2*上一次偏差
   ps->lastErr = err;                                // 存贮本次偏差

   if (ps->out > ps->outMax)
   {
      ps->out = ps->outMax;
   }
   else if (ps->out < ps->outMin)
   {
      ps->out = ps->outMin;
   }

   return (ps->out >> 16);
}
/**
 * @brief  初始化
 * @return
 * @note
 */
void SpeedIncPI_Init(void)
{
   g_sMotor.u16TargetSpeed = (u16)(64) * (g_sMotor.u8MotorRecvBuf[2] & 0x3f);

#if 0
   if (g_sMotor.u16TargetSpeed >= 1856     // 29RPM
       || g_sMotor.u16TargetSpeed == 1600  // 25RPM
       || g_sMotor.u16TargetSpeed == 1280) // 20RPM*64减速比 正常割草时KP=0.05
   {
      g_sSpeedIncPI.K1 = (int32_t)(65536.0L * 0.01 * (1.0 + 0.2)); // K1 = Kp*(1+Ki)
      g_sSpeedIncPI.K2 = (int32_t)(-65536.0L * 0.01);              // K2 = -Kp
   }
   else if ((g_sMotor.u16TargetSpeed >= 1536) && (g_sMotor.u16TargetSpeed < 1856))
   {
      g_sSpeedIncPI.K1 = (int32_t)(65536.0L * 0.06 * (1.0 + 0.6)); // K1 = Kp*(1+Ki)
      g_sSpeedIncPI.K2 = (int32_t)(-65536.0L * 0.06);              // K2 = -Kp
   }
   else
   {
      g_sSpeedIncPI.K1 = (int32_t)(65536.0L * 0.06 * (1.0 + 0.6)); // K1 = Kp*(1+Ki)
      g_sSpeedIncPI.K2 = (int32_t)(-65536.0L * 0.06);              // K2 = -Kp
   }
#else
   if (g_sMotor.u16TargetSpeed >= 1856     // 29RPM
       || g_sMotor.u16TargetSpeed == 1600  // 25RPM
       || g_sMotor.u16TargetSpeed == 1280) // 20RPM*64减速比 正常割草时KP=0.05
   {
      g_sSpeedIncPI.K1 = (int32_t)(65536.0L * 0.01 * (1.0 + 0.3)); // K1 = Kp*(1+Ki)
      g_sSpeedIncPI.K2 = (int32_t)(-65536.0L * 0.01);              // K2 = -Kp
   }
   else if ((g_sMotor.u16TargetSpeed >= 1536) && (g_sMotor.u16TargetSpeed < 1856))
   {
      g_sSpeedIncPI.K1 = (int32_t)(65536.0L * 0.03 * (1.0 + 0.6)); // K1 = Kp*(1+Ki)
      g_sSpeedIncPI.K2 = (int32_t)(-65536.0L * 0.03);              // K2 = -Kp
   }
   else
   {
      g_sSpeedIncPI.K1 = (int32_t)(65536.0L * 0.03 * (1.0 + 0.6)); // K1 = Kp*(1+Ki)
      g_sSpeedIncPI.K2 = (int32_t)(-65536.0L * 0.03);              // K2 = -Kp 
   }
#endif
   g_sSpeedIncPI.lastErr = SPEED_LastErr;                   // 上一次偏差值
   g_sSpeedIncPI.out = SPEED_OUT;                           // 上一次输出值
   g_sSpeedIncPI.outMin = (int32_t)(65536.0L * cDUTY_MIN);  // 最小限幅
   g_sSpeedIncPI.outMax = (int32_t)(65536.0L * cDUTY_FULL); // 最大限幅

   g_sMotor.PIRefSpeedUpValue = 20;       // 给定速度步进值3000*0.01
   g_sSpeedIncPI.u8RefSpeedTimeValue = 1; // PI环周期给定时间基准值(10ms)
}
/**
 * @brief  电机转速控制斜坡函数
 * @param[in] {u16TargetSpeed} 目标速度
 * @param[in] {u16RefSpeed} 参考速度
 * @return
 * @note 对参考速度值用步进值进行加/减操作,使参考速度值接近目标速度值
 */
uint16_t MotorSpeedStepCtrl(uint16_t u16TargetSpeed, uint16_t u16RefSpeed)
{
   if (u16RefSpeed + g_sMotor.PIRefSpeedUpValue <= u16TargetSpeed)
   {
      u16RefSpeed += g_sMotor.PIRefSpeedUpValue;
   }
   else if (u16RefSpeed > u16TargetSpeed + g_sMotor.PIRefSpeedUpValue)
   {
      u16RefSpeed -= g_sMotor.PIRefSpeedUpValue;
   }
   else
   {
      u16RefSpeed = u16TargetSpeed;
   }

   return u16RefSpeed;
}
/*************************************************END OF FILE*********************************************/
