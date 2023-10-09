#include "adc.h"       
#include "macrodriver.h"
#include "tau.h"
#include "userdefine.h"
/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#pragma interrupt r_tau0_channel0_interrupt(vect = INTTM00)
#pragma interrupt r_tau0_channel1_interrupt(vect = INTTM01)
#pragma interrupt r_tau0_channel2_interrupt(vect = INTTM02)
#pragma interrupt r_tau0_channel3_interrupt(vect = INTTM03)

static void __near r_tau0_channel0_interrupt(void)
{
   TMIF00 = 0U;
}
/***********************************************************************************************************************
 * Function Name: r_tau0_channel1_interrupt
 * Description  : This function INTTM00 interrupt service routine.
 * Arguments    : None
 * Return Value : None
 ***********************************************************************************************************************/
static void __near r_tau0_channel1_interrupt(void) 
{
   TMIF01 = 0U;
}
/***********************************************************************************************************************
 * Function Name: r_tau0_channel2_interrupt
 * Description  : This function INTTM02 interrupt service routine.
 * Arguments    : None
 * Return Value : None
 ***********************************************************************************************************************/
static void __near r_tau0_channel2_interrupt(void)
{
   /* Start user code for adding. Do not edit comment generated here */
   /* End user code. Do not edit comment generated here */
}
/***********************************************************************************************************************
 * Function Name: r_tau0_channel3_interrupt
 * Description  : This function INTTM03 interrupt service routine.
 * Arguments    : None
 * Return Value : None
 ***********************************************************************************************************************/
static void __near r_tau0_channel3_interrupt(void) // 1ms定时器
{
#if 0
   uint8_tFlag_1ms = 1;
#endif

   TMIF03 = 0U;
}
