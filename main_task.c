/*
 * @Descripttion : 主任务c文件
 * @version      : 1.0.0
 * @Author       : Lichar
 * @Date         : 2022-10-15 17:44:52
 * @LastEditors  : Lichar
 * @LastEditTime : 2023-02-02 15:58:33
 */

/* Includes ------------------------------------------------------------------*/
#include "main_task.h"
#include "bldc.h"
#include "delay_time.h"
#include "pid.h"
#include "sub_task.h"
#include "tau.h"
#include "tmrd.h"  
#include "adc.h"

/* Define --------------------------------------------------------------------*/

/* Parameter -----------------------------------------------------------------*/
U_ERROR_FLAG_T g_sFlagErr; // 故障位域
E_SYS_STA_T g_eSysState;   // 系统工作状态

/* Function ------------------------------------------------------------------*/

/**
 * @brief  启动状态计数器
 * @return Null
 * @note
 */
static void Motor_StateCnt(u8 *pStatCnt)
{
   if (++(*pStatCnt) > 250)
   {
      *pStatCnt = 250;
   }
}
/**
 * @brief  预启动函数
 * @return Null
 * @note 串口中断接到主机命令进入这个函数
 */
static void Motor_Standby_Act(void)
{
   static uint8_t HOff_flag = 0;

   if (HOff_flag == 0)
   {
      BLDC_ConsumeBrake(); // 刹车自举
      HOff_flag = 1;
   }

   Motor_StateCnt(&g_sMotor.uStaCnt.u8Standby);        // 待机状态计数
   if (g_sMotor.uStaCnt.u8Standby >= cSTANDBY_1ms_CNT) // 待机状态计时到
   {
      HOff_flag = 0;             // 自举时间到
      g_sMotor.eAct = eBOOT_ACT; // 切换
   }
}
/**
 * @brief  自举函数
 * @return Null
 * @note
 */
static void Motor_Boot_Act(void)
{
   // 初始化参数配置
   g_sDrive.u16CurrPwmDuty = 0;
   g_sDrive.u16TargetPwmDuty = 0;

   SpeedIncPI_Init();      // PI参数初始化
   PwmDrv_PwmOutDisable(); // 工作前先关六管,防止直通

   g_sDrive.uHallValue = Hall_Get();                             // 读取当前hall值
   if ((g_sDrive.uHallValue == 0) || (g_sDrive.uHallValue == 7)) // 错误的hall值
   {
      return;
   }
   // 计算hall位置
   g_sDrive.MotorPosition = GetMotorPosition(g_sDrive.uHallValue);

   HALL_INT_ON();   // 开启hall的io中断
   R_TMRD0_Start(); // 开启pwm输出

   if (TESTBIT(g_sMotor.u8MotorRecvBuf[2], 7)) // 检测正反转
      g_sMotor.eDir = eCCW;
   else
      g_sMotor.eDir = eCW;
   g_sDrive.MotorDirection = g_sMotor.eDir; // 获取方向位

   Cap_PreCharge(g_sDrive.MotorPosition); // 自举电容充电
   Delay_us(10);
   PWM_StartDutyUpdata(0); // 清空pwm计数器

   g_sMotor.eAct = eSTART_ACT; // 切换
   g_sDrive.hall_isr_flag = 0; // 置位标志位
}
/**
 * @brief  启动函数
 * @return Null
 * @note
 */
static void Motor_Start_Act(void)
{
#if 1 // 强起--读取hall换向标志，只到换向触发了才进行切换
   if (g_sDrive.hall_isr_flag == 1)
   {
      g_sMotor.sSpeedFilt.u8Cnt = 0;  // 清速度滤波计数器
      g_sMotor.sSpeedFilt.u32Sum = 0; // 清速度滤波累加和
      g_sMotor.u16Speed = 0;          // 电机速度清0
      g_sDrive.g_aTzc[0] = 0;         // 速度计算:过零计数器清0
      g_sDrive.g_aTzc[1] = 0;
      g_sDrive.g_aTzc[2] = 0;
      g_sDrive.g_u16LastZeroTimerVal = _FFFF_TAU_TDR00_VALUE; // 上次过零定时器时间值初始化
      R_TAU0_Channel0_Start();                                // 开启定时器0-CH0(换相间隔时间)
      TCR00 = _FFFF_TAU_TDR00_VALUE;                          // 时间计数器恢复为默认值

      g_sMotor.eAct = eRUN_ACT; // 切换
   }
   else
   {
      g_sDrive.u16CurrPwmDuty += 20;
      // g_sDrive.u16CurrPwmDuty++;                                 // 累加pwm值，从而冲破电机的死区
      PWM_StartDutyUpdata(g_sDrive.u16CurrPwmDuty);                 // 启动pwm控制器
      g_sDrive.uHallValue = Hall_Get();                             // 读取当前hall值
      if ((g_sDrive.uHallValue == 0) || (g_sDrive.uHallValue == 7)) // 错误的hall值
      {
         return;
      }
      // 计算hall位置
      g_sDrive.MotorPosition = GetMotorPosition(g_sDrive.uHallValue);
      Commutate_Phase(g_sDrive.MotorPosition);
      // 获得期望的hall值
      g_sDrive.uHallValueExpect = Expect_Hall_Value(g_sDrive.MotorPosition);
   }
#else // 强定位--只定位一个位置
   g_sMotor.sSpeedFilt.u8Cnt = 0;  // 清速度滤波计数器
   g_sMotor.sSpeedFilt.u32Sum = 0; // 清速度滤波累加和
   g_sMotor.u16Speed = 0;          // 电机速度清0
   g_sDrive.g_aTzc[0] = 0;         // 速度计算:过零计数器清0
   g_sDrive.g_aTzc[1] = 0;
   g_sDrive.g_aTzc[2] = 0;
   g_sDrive.g_u16LastZeroTimerVal = _FFFF_TAU_TDR00_VALUE; // 上次过零定时器时间值初始化
   R_TAU0_Channel0_Start();                                // 开启定时器0-CH0(换相间隔时间)
   TCR00 = _FFFF_TAU_TDR00_VALUE;                          // 时间计数器恢复为默认值

   g_sDrive.u16CurrPwmDuty = cDUTY_MIN;                          // 启动占空比
   PWM_StartDutyUpdata(g_sDrive.u16CurrPwmDuty);                 // 启动pwm控制器
   g_sDrive.uHallValue = Hall_Get();                             // 读取当前hall值
   if ((g_sDrive.uHallValue == 0) || (g_sDrive.uHallValue == 7)) // 错误的hall值
   {
      return;
   }
   // 计算hall位置
   g_sDrive.MotorPosition = GetMotorPosition(g_sDrive.uHallValue);
   Commutate_Phase(g_sDrive.MotorPosition);

   g_sMotor.eAct = eRUN_ACT;                                                                                      // 切换
#endif
}
/**
 * @brief  hall超时保护
 * @return Null
 * @note
 */
static void MotorRunHallTimeOutProtect(void)
{
#define HALL_PROTECT_TIME_OUT 2000         // 设置超时时间
#define HALL_PROTECT_FAIL_COUNT_MAX 3      // 换向错误最大次数
#define HALL_PROTECT_FAIL_RESET_TIME 60000 // 换向错误恢复时间

   // 1ms一次循环
   static uint16_t hall_count = 0;            // 换向超时计数器
   static uint8_t hall_value_last = 0;        // 上一次hall值
   static uint8_t hall_fail_count = 0;        // 换向错误计数器
   static uint16_t hall_reset_fail_count = 0; // 换向错误复位计数器

   if (hall_value_last != g_sDrive.uHallValue) // 上一次的hall值与当前的值不一致，就刷新计数器
   {
      hall_value_last = g_sDrive.uHallValue; // 刷新上一次的值
      hall_count = 0;                        // 重置数据
   }
   else // 如果没有任何变化
   {
      if (++hall_count >= HALL_PROTECT_TIME_OUT)
      {
         g_eSysState = cSTA_STOP; // 重新启动
         RstVarBeforInStopSta();
         hall_count = 0; // 重置数据
         if (++hall_fail_count >= HALL_PROTECT_FAIL_COUNT_MAX)
         {
            g_sFlagErr.Bits.bMotorBlock = 1; //置位堵转
            R_TMRD0_Stop();
            R_TAU0_Channel0_Stop(); // CH0换相时间监控 CH1延时换相设定
            R_ADC_Stop();
            hall_fail_count = 0; // 重置数据
         }
      }
   }
   // 超过一定时间后重置报错次数，保证错误连续,但是只有在真正运行的时候才能删除
   if (++hall_reset_fail_count >= HALL_PROTECT_FAIL_RESET_TIME)
   {
      hall_reset_fail_count = 0; // 重置数据
      hall_fail_count = 0;       // 重置数据
   }
}
/**
 * @brief  运动函数
 * @return Null
 * @note
 */
static void Motor_Run_Act(void)
{
   g_sMotor.u16TargetSpeed = (u16)(64) * (g_sMotor.u8MotorRecvBuf[2] & 0x3f);
   BLDC_u8ZeroCrossTimeToSpeed(&g_sMotor, g_sDrive.g_aTzc); // 得出实时转速

#if 0 // 带增量式提升速度
   g_sDrive.u16TargetPwmDuty = (u16)IncrementPI_Ctrl(g_sMotor.u16RefSpeed, g_sMotor.u16Speed, &g_sSpeedIncPI); // 速度增量式PI
   g_sDrive.u16CurrPwmDuty = g_sDrive.u16TargetPwmDuty;
   if (++g_sSpeedIncPI.timePIUpGiveSpeed >= g_sSpeedIncPI.u8RefSpeedTimeValue)
   {
      g_sSpeedIncPI.timePIUpGiveSpeed = 0;
      g_sMotor.u16RefSpeed = MotorSpeedStepCtrl(g_sMotor.u16TargetSpeed, g_sMotor.u16RefSpeed); // 给定速度输入到PI环:当前转速逐步到目标转速
   }
#else // 直接pid拉起
   g_sDrive.u16TargetPwmDuty = (u16)IncrementPI_Ctrl(g_sMotor.u16TargetSpeed, g_sMotor.u16Speed, &g_sSpeedIncPI); // 速度增量式PI
   g_sDrive.u16CurrPwmDuty = g_sDrive.u16TargetPwmDuty;
#endif
   PWM_PidDutyUpdata(g_sDrive.u16CurrPwmDuty); // pid的pwm控制器
}
/**
 * @brief  工作状态下的电机处理
 * @return Null
 * @note
 */
static void MotorWalkDisp_InRunSta(void)
{
   switch (g_sMotor.eAct)
   {
   case eSTANDBY_ACT:
      Motor_Standby_Act();
      break; // 待机动作
   case eBOOT_ACT:
      Motor_Boot_Act();
      break; // 自举
   case eSTART_ACT:
      Motor_Start_Act();
      break; // 启动
   case eRUN_ACT:
      Motor_Run_Act();
      MotorRunHallTimeOutProtect(); // hall超时保护
      break;                        // 运行动作
   default:
      break;
   }
}
/**
 * @brief  主任务参数清除
 * @return Null
 * @note 主循环任务,1ms周期
 */
static void MainTaskStopParamClear(void)
{
   g_sDrive.u16CurrPwmDuty = 0;
   g_sMotor.u16Speed = 0;
}
/**
 * @brief  主任务
 * @return Null
 * @note 主循环任务,1ms周期
 */
void Main_MotorTask(void)
{
   static uint16_t task_first = 0;

   switch (g_eSysState)
   {
   case cSTA_SELFCHK:
      if ((task_first & 0x01) == 0)
      {
         BLDC_ConsumeBrake();      // 完全刹住
         MainTaskStopParamClear(); // 清除参数
         task_first = 0x01;
      }
      /*------------------------------------*/
      // 操作
      if (g_sFlagErr.all) // 错误检测
         g_eSysState = cSTA_ERR;
      else
         g_eSysState = cSTA_WAIT;
      break;
   case cSTA_WAIT:
      if ((task_first & 0x02) == 0)
      {
         BLDC_ConsumeBrake();      // 完全刹住
         MainTaskStopParamClear(); // 清除参数
         task_first = 0x02;
      }
      /*------------------------------------*/
      // 操作
      if (g_sFlagErr.all) // 错误检测
         g_eSysState = cSTA_ERR;
      break;
   case cSTA_ERR:
      if ((task_first & 0x04) == 0)
      {
         BLDC_ConsumeBrake();      // 完全刹住
         MainTaskStopParamClear(); // 清除参数
         task_first = 0x04;
      }
      /*------------------------------------*/
      // 操作
      break;
   case cSTA_RUN:
      if ((task_first & 0x10) == 0)
      {
         task_first = 0x10;
      }
      /*------------------------------------*/
      // 操作
      MotorWalkDisp_InRunSta();
      if (g_sFlagErr.all) // 错误检测
         g_eSysState = cSTA_ERR;
      break;
   case cSTA_STOP:
      if ((task_first & 0x20) == 0)
      {
         BLDC_ConsumeBrake();      // 完全刹住
         MainTaskStopParamClear(); // 清除参数
         task_first = 0x20;
      }
      /*------------------------------------*/
      // 操作
      if (g_sFlagErr.all)
      {
         g_eSysState = cSTA_ERR; // 有故障切换为故障状态
      }
      else
      {
         if (++g_sMotor.u8BrakeTime >= 200) // 刹车200ms转待机状态
         {
            g_sMotor.u8BrakeTime = 0;
            g_eSysState = cSTA_WAIT;
         }
      }
      break; // 停机刹车动作
   default:
      break;
   }
}
/*************************************************END OF FILE*********************************************/
