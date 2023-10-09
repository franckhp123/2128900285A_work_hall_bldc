/*
 * @Descripttion : 打印输出文件h文件
 * @version      : 1.0.0
 * @Author       : Lichar
 * @Date         : 2022-10-15 17:44:52
 * @LastEditors  : Lichar
 * @LastEditTime : 2023-02-02 15:58:33
 */
#ifndef __DEBUG_PRINTF_H
#define __DEBUG_PRINTF_H

/******************************参数声明***********************************************/
// #define DEBUG_PRINTF

/******************************定义声明***********************************************/


/******************************变量声明***********************************************/

/******************************函数声明***********************************************/
#ifdef DEBUG_PRINTF

void _dbg_printf(const char *format, ...);

// #define PAPER_AIRPLANE_PRINT(title,fmt,args...) _dbg_printf("{"#title":"fmt"}\n",##args)

#endif

#endif
































