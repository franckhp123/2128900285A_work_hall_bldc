/*
 * @Descripttion : 无刷电机操作相关h文件
 * @version      : 1.0.0
 * @Author       : Lichar
 * @Date         : 2022-10-15 17:44:52
 * @LastEditors  : Lichar
 * @LastEditTime : 2023-02-02 15:58:33
 */
#ifndef __BLDC_H
#define __BLDC_H

#include "macrodriver.h" 
#include "r_cg_intp.h"
#include "tmrd.h"

/******************************参数声明***********************************************/
//hall开关
#define	HALL_INT_ON() 	   R_INTC0_Start();R_INTC4_Start();R_INTC3_Start()
#define	HALL_INT_OFF() 	R_INTC0_Stop();R_INTC4_Stop();R_INTC3_Stop()
//关闭pwm
#define	OUTPUT_PWM_OFF()     TRDGRB0 = SET_FREQUENCY_PWM_VALUE;\
                              TRDGRC0 = SET_FREQUENCY_PWM_VALUE;\
                              TRDGRD0 = SET_FREQUENCY_PWM_VALUE;\
                              TRDGRA1 = SET_FREQUENCY_PWM_VALUE;\
                              TRDGRB1 = SET_FREQUENCY_PWM_VALUE;\
                              TRDGRC1 = SET_FREQUENCY_PWM_VALUE;\
                              TRDGRD1 = SET_FREQUENCY_PWM_VALUE;
//方向
#define   CW     0
#define   CCW    1
//下桥
#define AL_IO	0b00010000
#define BL_IO	0b00000010
#define CL_IO	0b00000001
//上桥
#define AH_PWM	0b11110101   //W  H-BO L-D0 //互补
#define BH_PWM	0b10101111   //V  H-A1 L-C1
#define CH_PWM	0b01011111   //U  H-B1 L-D1
//hall io
#define	HALL_U		P3_bit.no1  
#define	HALL_V		P13_bit.no7
#define	HALL_W		P3_bit.no0
//占空比设置
#define cDUTY_10         240  
#define cDUTY_15         360
#define cDUTY_18         432       
#define cDUTY_20         480
#define cDUTY_23         552 

#define cDUTY_25         600  
#define cDUTY_30         720
#define cDUTY_35         840
#define cDUTY_40         960
#define cDUTY_45         1080
#define cDUTY_50         1200
#define cDUTY_55         1320      

#define cDUTY_60         1440
#define cDUTY_70         1680
#define cDUTY_75         1800
#define cDUTY_79         1896
#define cDUTY_80         1920
#define cDUTY_85         2040
#define cDUTY_90         2160
#define cDUTY_95         2280
#define cDUTY_100        2400

#define cDUTY_FULL       SET_FREQUENCY_PWM_VALUE  //TRDGRA0 满占空比
#define cDUTY_MIN        cDUTY_15 

/******************************定义声明***********************************************/
typedef struct
{
   u8 u8Standby;   // 0 待机
   u8 u8Boot;      // 1 自举
   u8 u8Position;  // 2 定位
   u8 u8Run;       // 3 闭环运行
   u8 u8Stop;      // 4 停机
   u8 u8Brake;     // 5 刹车
   u8 u8Error;     // 6 故障
}U_MOTOR_STA_T; //电机工作状态类型

typedef struct
{
   u16 bStandbyFinish   :1;  // 0  待机状态完成,允许进入其他状态
   u16 bBootFinsih      :1;  // 1  自举状态完成,允许进入其他状态
   u16 bPositionFinish  :1;  // 2  定位角度状态完成,允许进入其他状态
   u16 bStartFinish     :1;  // 3  启动状态完成,允许进入其他状态
   u16 bRunFinish       :1;  // 4  闭环运行状态完成,允许进入其他状态
   u16 bStopFinish      :1;  // 5  停机状态完成,允许进入其他状态
   u16 bBrakeFinish     :1;  // 6  刹车状态完成,允许进入其他状态
   u16 bErrorFinish     :1;  // 7  故障状态完成,允许进入其他状态
   u16 bAlarmFinish     :1;  // 8  告警状态完成,允许进入其他状态
   u16 bPowerOffFinish  :1;  // 9  关机掉电状态完成,允许进入其他状态
   u16 reserved10       :1;  // 10 预留
   u16 reserved11		:1;  // 11 预留
   u16 reserved12		:1;  // 12 预留
   u16 reserved13		:1;  // 13 预留
   u16 reserved14		:1;  // 14 预留
   u16 reserved15		:1;  // 15 预留
}U_MOTOR_STAT_FLAG_T; //电机状态切换事件标志类型

typedef struct  //电机状态切换事件触发位
{
   u16 bInsertImpulse    :1;  // 0  需要注入脉冲标志
   u16 bFluxStartRead    :1;  // 1  开始读取磁链标志
   u16 bFluxToBemf       :1;  // 2  置从磁链切换到高速标志
   u16 bDisBemfCalcu     :1;  // 3  禁止反电势运算标志
   u16 bBemfRising       :1;  // 4  反电势上升标志
   u16 bBemfFalling      :1;  // 5  反电势下降标志
   u16 bGetBemfZeroCross :1;  // 6  已获得过零点标志
   u16 bDelayTime        :1;  // 7  换相成功
   u16 bfluxCommuSucess  :1;  // 8  磁链换相成功标志                 
   u16 bRecvFinish       :1;  // 9  数据接收完成标志
   u16 bCrash            :1;  // 10 碰撞标志
   u16 reserved11        :1;  // 11 预留
   u16 reserved12        :1;  // 12 预留
   u16 reserved13        :1;  // 13 预留
   u16 reserved14        :1;  // 14 预留
   u16 reserved15        :1;  // 15 预留
}U_MOTOR_WORK_FLAG_T; //电机工作标志类型

typedef enum
{
   eSTANDBY_ACT,   // 0 停机
   eBOOT_ACT,      // 1 自举
   ePOSITION_ACT,  // 2 定位角度
   eSTART_ACT,     // 3 启动
   eRUN_ACT,       // 4 运行
   eSTOP_ACT,      // 5 停机
   eBRAKE_ACT,     // 6 刹车
   eERROR_ACT,     // 7 故障
}E_MOTOR_STAT_T;

typedef enum
{
   eNULL_PHASE = 0, //无效相位
   eUV = 1,
   eWU = 2,
   eWV = 3,
   eVW = 4,
   eUW = 5,
   eVU = 6,
}E_MOTOR_PHASE_T; //BLDC相位

typedef enum  //CCW和CW定义:从电机非主轴端且与轴平行的视角去看,电机主轴是
{
   eCW,   //正转
   eCCW,  //反转
   eMID   //中间
}E_MOTOR_DIR_T;  //电机旋转方向

typedef struct
{
  u8   u8SampCnt; // 采样数量
  u8   u8Shift;   // 移位除法
  u8   u8Cnt;     // 累加计数
  u16  u16Max;    // 无符号最大值
  u16  u16Min;    // 无符号最小值
  u32  u32Sum;    // 无符号累加值
  s32  i32Max;    // 有符号最大值
  s32  i32Min;    // 有符号最小值
  s32  i32Sum;    // 有符号累加值
}S_EXTREMUM_MEAN_FILT_T; // 去极值平均滤波结构体  

typedef struct
{
   U_MOTOR_STA_T           uStaCnt;        //状态时间计数器  
   U_MOTOR_STAT_FLAG_T     uStatFlag;      //状态标志
   U_MOTOR_WORK_FLAG_T     uWorkFlag;      //工作标志
   E_MOTOR_DIR_T           eDir;           //工作转向
   E_MOTOR_DIR_T           eDirBuf;        //工作转向缓存
   E_MOTOR_DIR_T           eTargetDir;     //目标转向
   E_MOTOR_STAT_T          eAct;           //当前动作状态
   uint8_t                 u8Phase;        //当前相位
   S_EXTREMUM_MEAN_FILT_T  sSpeedFilt;     //电机速度去极值平均滤波     
   u16                     u16Speed;       //当前转速
   u16                     u16RefSpeed;    //给定转速(速度环给定)
   u16                     u16TargetSpeed; //目标转速(用户给定)
   u8                      u8TimeCntForceChgPhase; //强制换相时间
   u8                      u8TimeCntFluxToForce;   //磁链切强拉时间计数器
   u8                      u8MotorRecvBuf[6];      //接收主机数据缓存
   u8                      u8MotorSendBuf[4];      //发送主机数据缓存
   u8                      RecvCnt;                //接收主机数据计数器
   u8                      u8BrakeTime;            //刹车时间
   u16                     u16TimeInterPI;         //进入PI环时间
   u16                     u16ZeroCurAD;           //电机零点电流AD
   u16                     u16CurrADBuf[4];        //电流缓存AD
   u8                      u8MotorStatus;          //电机状态
   u8                      u8MotorErrStaTxCnt;     //电机故障状态发送次数
   u16                     PIRefSpeedUpValue;      //PI环给定速度步进值  
   u8                      u8StartChagedPhaseCnt;  //启动换相次数 
   u8                      u8PWMPerionIntCnt;      //PWM周期中断计数器  
   u8                      u8WorkModeInRun;        //电机工作模式  
   u16                     u16TimeUpInRun;         //工作模式下电机自增工作时间(非PI操作)
}S_MOTOR_T;  //电机结构体

typedef struct
{
   u16 u16CurrPwmDuty;     //当前占空比
   u16 u16CurrPwmDutyBuf;  //当前占空比缓存(用在PWM中断中)
   u16 u16TargetPwmDuty;   //目标占空比
   u16 g_aTzc[3];          // 过零时间数组
   u16 g_u16LastZeroTimerVal; // 上一次过零定时器值
   u16 g_u16NowZeroTimerVal;  // 本次过零定时器值
   uint8_t uHallValue;//hall值
   uint8_t MotorPosition;//电机位置
   uint8_t uHallValueExpect;//hall期望值
   uint8_t MotorDirection;//电机方向
   uint8_t hall_isr_flag;//hall换向标志
}S_DRIVE_T;//驱动结构体(驱动模式占空比档位等)

typedef enum
{
   eSTAG0_1stINT,  // 0 阶段0 第1次中断   
   eSTAG0_2stINT,  // 1 阶段0 第2次中断
   eSTAG1_1stINT,  // 2 阶段1 第1次中断
   eSTAG1_2stINT,  // 3 阶段1 第2次中断
   eSTAG2_1stINT,  // 4 阶段2 第1次中断
   eSTAG2_2stINT,  // 5 阶段2 第2次中断
}E_MOTOR_RUN_SUB_STAT_T; //电机运行子状态
/******************************变量声明***********************************************/
extern S_MOTOR_T g_sMotor;             // 电机相关变量
extern S_DRIVE_T g_sDrive;             // 驱动相关变量

/******************************函数声明***********************************************/
void NullPhase(void);
void UH_VL_HPwmLON(void);
void UH_WL_HPwmLON(void);
void VH_WL_HPwmLON(void);
void VH_UL_HPwmLON(void);
void WH_UL_HPwmLON(void);
void WH_VL_HPwmLON(void);
void BLDC_ConsumeBrake(void); 
void BLDC_StopMotor(void);
void PWM_DutyUpdata(u16 duty);  
void PwmDrv_PwmOutDisable(void);//关6管  
u8 BLDC_u8ZeroCrossTimeToSpeed(S_MOTOR_T *psMotor,u16 aZeroCrossTime[]); 
void HallISR(void);
uint8_t Hall_Get(void);
uint8_t GetMotorPosition(uint8_t hallvalue);
void Commutate_Phase(uint8_t Position);
void Cap_PreCharge(uint8_t Position);
uint8_t Expect_Hall_Value(uint8_t Position);
void PWM_PidDutyUpdata(u16 duty);   
void PWM_StartDutyUpdata(u16 duty);

#endif
