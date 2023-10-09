
#include "adc.h"
#include "cgc.h"
#include "comppga.h"
#include "elc.h"
#include "macrodriver.h"
#include "port.h"
#include "sau.h"
#include "tau.h"
#include "tmrd.h"
#include "userdefine.h"
#include "r_cg_intp.h"
#include "r_cg_it.h"
#include "r_cg_wdt.h"

void R_Systeminit(void)
{
   PIOR1 = 0x00U;           // 定时器RJ TRJO0和P30/P01复用
   R_CGC_Get_ResetSource(); // 时钟配置,已在编译器配置项内配置过
   R_CGC_Create();          // 系统时钟配置
   R_PORT_Create();         // GPIO配置
   R_TAU0_Create();         // 定时器配置
   R_TMRD0_Create();        // 定时器RD配置(PWM) //
   R_SAU0_Create();         // 串口配置
   R_ADC_Create();          // ADC配置
   R_COMPPGA_Create();      // 比较器/放大器配置
   R_ELC_Create();          // ELC配置
   R_INTC_Create();
   R_IT_Create();
   R_WDT_Create();//使能看门狗
   IAWCTL = 0x00U;
}

/***********************************************************************************************************************
 * Function Name: hdwinit
 * Description  : This function initializes hardware setting.
 * Arguments    : None
 * Return Value : None
 ***********************************************************************************************************************/
void hdwinit(void)
{
   DI();
   R_Systeminit();
}
