/*
 * @Descripttion : 主任务h文件
 * @version      : 1.0.0
 * @Author       : Lichar
 * @Date         : 2022-10-15 17:44:52
 * @LastEditors  : Lichar
 * @LastEditTime : 2023-02-02 15:58:33
 */
#ifndef __MAIN_TASK_H
#define __MAIN_TASK_H

#include "macrodriver.h"  

/******************************参数声明***********************************************/
#define cSTANDBY_1ms_CNT         5  //开始启动时,待机状态时间修改为5ms 
#define cBOOT_1ms_CNT            40
#define cPOSITION_1ms_CNT        10
#define cSTART_1ms_CNT           1
#define cRUN_1ms_CNT             10 
#define cSTOP_1ms_CNT            10 
#define cBRAKE_1ms_CNT           10 
#define cERROR_1ms_CNT           10 

/******************************定义声明***********************************************/
//事件触发标志联合体
typedef union
{
   uint16_t all;
   struct
   {
      uint16_t bMosOverTmp     :1; // 00 MOS过温故障      
      uint16_t bMosTempUnder   :1; // 01 MOS低温故障 
      uint16_t bBatTempOver    :1; // 02 电池包高温故障
	  uint16_t bBatTempUnder   :1; // 03 电池包低温故障
	  uint16_t bStart          :1; // 04 预驱动故障
	  uint16_t bHall           :1; // 05 HALL故障
	  uint16_t bIdcLimit       :1; // 06 限流超时
	  uint16_t bAvgIdcShort    :1; // 07 均值电流短路
	  uint16_t bAvgIdcOver     :1; // 08 均值电流过流
	  uint16_t bShortCurrent	  :1; // 09 峰值电流短路
	  uint16_t bPeakIdcOver    :1; // 10 峰值电流过流
	  uint16_t bBusVdcOver	  :1; // 11 直流母线过压
	  uint16_t bBusVdcUnder    :1; // 12 直流母线欠压
	  uint16_t bMotorBlock	  :1; // 13 电机堵转
	  uint16_t bPosition	      :1; // 14 定位失败故障  
	  uint16_t bObstacles	  :1; // 15 障碍物故障   
   }Bits;
}U_ERROR_FLAG_T;

typedef enum 
{
   cSTA_SELFCHK , //自检状态
   cSTA_WAIT    , //待机全关状态   
   cSTA_RUN     , //运行状态
   cSTA_STOP    , //停机刹车状态
   cSTA_ERR       //故障状态
}E_SYS_STA_T;

/******************************变量声明***********************************************/
extern  U_ERROR_FLAG_T       g_sFlagErr;           
extern  E_SYS_STA_T          g_eSysState;             

/******************************函数声明***********************************************/
void Main_MotorTask(void);

#endif































