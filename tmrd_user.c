
#include "macrodriver.h"
#include "tmrd.h"
#include "tau.h" 
#include "userdefine.h"
#include "pid.h"


/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#pragma interrupt r_tmrd0_interrupt(vect=INTTRD0)


/***********************************************************************************************************************
* Function Name: r_tmrd0_interrupt
* Description  : None
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void __near r_tmrd0_interrupt(void)   
{
//   u8 u8DelayCnt = 0;
//   static  u8 UpPwmDutyCnt = 0;
   volatile u8 trdsr_dummy; 
   
   trdsr_dummy = TRDSR0;
   TRDSR0 = trdsr_dummy&0x1C; //清故障标志位        
}


