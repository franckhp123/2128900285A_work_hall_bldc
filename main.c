/*
 * @Descripttion : 子任务任务c文件
 * @version      : 1.0.0
 * @Author       : Lichar
 * @Date         : 2022-10-15 17:44:52
 * @LastEditors  : Lichar
 * @LastEditTime : 2023-02-02 15:58:33
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "comppga.h"
#include "delay_time.h"
#include "main_task.h"
#include "r_cg_it.h"
#include "r_cg_wdt.h"
#include "sau.h"
#include "sub_task.h"
#include "tmrd.h"

/* Define --------------------------------------------------------------------*/

/* Parameter -----------------------------------------------------------------*/
uint16_t led_count = 0;
uint8_t Flag_1ms = 0;

/* Function ------------------------------------------------------------------*/

/**
 * @brief  主函数入口
 * @return Null
 */
void main(void)
{
   DI();
   R_IT_Start();
   Delay_ms(10);
   R_COMP1_Start();
   R_UART0_Start(); // 波特率38400-Debug OK
   R_TMRD0_Start();
   EI(); // Enable int.

   while (1)
   {
      if (Flag_1ms) // 1ms
      {
         // 指示灯闪烁
         if (++led_count > 500)
         {
            led_count = 0;
            LED_REV();
         }

         Flag_1ms = 0;
         R_WDT_Restart();  // 喂狗--配置选项字节为118.72ms上溢时间
         Sub_LoopTask();   // 子任务循环
         Main_MotorTask(); // 主任务循环
      }
   }
}
/*************************************************END OF FILE*********************************************/
