
/*
 * @Descripttion : 子任务任务c文件
 * @version      : 1.0.0
 * @Author       : Lichar
 * @Date         : 2022-10-15 17:44:52
 * @LastEditors  : Lichar
 * @LastEditTime : 2023-02-02 15:58:33
 */

/* Includes ------------------------------------------------------------------*/
#include "delay_time.h"

/* Define --------------------------------------------------------------------*/

/* Parameter -----------------------------------------------------------------*/

/* Function ------------------------------------------------------------------*/

/**
 * @brief  us延迟
 * @param[in] {cnt} 输入微秒
 * @return
 * @note
 */
void Delay_us(uint16_t cnt)
{
   uint16_t i;

   for (i = 0; i < cnt; i++)
   {
      NOP();
      NOP();
      NOP();
      NOP();
      NOP();
      NOP();
      NOP();
      NOP();
      NOP();
      NOP();
      NOP();
      NOP();
      NOP();
      NOP();
   }
}
/**
 * @brief  ms延迟
 * @param[in] {cnt} 输入毫秒
 * @return
 * @note
 */
void Delay_ms(uint16_t cnt)
{
   uint16_t i;

   for (i = 0; i < cnt; i++)
   {
      Delay_us(1000);
   }
}
/*************************************************END OF FILE*********************************************/
