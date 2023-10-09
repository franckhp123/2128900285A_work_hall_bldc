/*
 * @Descripttion : 子任务任务c文件
 * @version      : 1.0.0
 * @Author       : Lichar
 * @Date         : 2022-10-15 17:44:52
 * @LastEditors  : Lichar
 * @LastEditTime : 2023-02-02 15:58:33
 */

/* Includes ------------------------------------------------------------------*/
#include "adc.h"
#include "bldc.h"
#include "debug_printf.h"
#include "delay_time.h"
#include "macrodriver.h"
#include "main_task.h"
#include "sau.h"
#include "sub_task.h"
#include "tau.h"
#include "tmrd.h"
#include "userdefine.h"

/* Define --------------------------------------------------------------------*/

/* Parameter -----------------------------------------------------------------*/
volatile u8 *gp_uart0_rx_address; /* uart1 receive buffer address */

/* Function ------------------------------------------------------------------*/

/**
 * @brief  uart接收中断
 * @return Null
 * @note
 */
static void __near r_uart0_interrupt_receive(void)
{
#ifndef DEBUG_PRINTF
	uint8_t i = 0;
#endif
	uint8_t Recvdata = 0;

	SRIF0 = 0U;
	EI(); // 允许中断嵌套(此中断优先级为3):AD中断(过零点计算)优先级1;定时器0通道1(超前换相)中断优先级2
	Recvdata = RXD0;
#if 1
	g_sMotor.u8MotorRecvBuf[g_sMotor.RecvCnt] = Recvdata;

	switch (g_sMotor.RecvCnt)
	{
	case 0:
		g_sMotor.RecvCnt = (Recvdata == 0x54) ? g_sMotor.RecvCnt + 1 : 0;
		break;
	case 1: // 接收器件地址
	case 2: // 接收是否工作数据
	case 3: // 接收坡度角高8位
	case 4: // 接收坡度角低8位
		g_sMotor.RecvCnt++;
		break;
	case 5:
		if (Recvdata == 0xff)
		{
			g_sMotor.RecvCnt = 0;
			g_sMotor.uWorkFlag.bRecvFinish = 1;
			// Sub_SysStaChange();
			g_sMotor.u16TargetSpeed = (u16)(64) * (g_sMotor.u8MotorRecvBuf[2] & 0x3f);
			switch (g_eSysState)
			{
			case cSTA_WAIT: // 待机状态下接收到速度值,置运行状态
				if (g_sMotor.u8MotorRecvBuf[2] != 0)
				{
					g_eSysState = cSTA_RUN;
					RstVarBeforInRunSta();
					if (TESTBIT(g_sMotor.u8MotorRecvBuf[2], 7))
					{
						g_sMotor.eDir = eCCW;
					}
					else
					{
						g_sMotor.eDir = eCW;
					}
				}
				else
				{
#ifndef DEBUG_PRINTF
					for (i = 0; i < 4; i++)
					{
						UartSendData(g_sMotor.u8MotorSendBuf[i]);
					}
#endif
				}
				break;
			case cSTA_RUN: // 运行状态下,接收到停机指令或转向改变时,置刹车停机状态
				if (g_sMotor.u8MotorRecvBuf[2] == 0)
				{
					g_eSysState = cSTA_STOP;
					RstVarBeforInStopSta();
				}

				if (TESTBIT(g_sMotor.u8MotorRecvBuf[2], 7))
				{
					g_sMotor.eDirBuf = eCCW;
				}
				else
				{
					g_sMotor.eDirBuf = eCW;
				}

				if (g_sMotor.eDirBuf != g_sMotor.eDir) // 转向改变
				{
					g_eSysState = cSTA_STOP;
					RstVarBeforInStopSta();
				}
				else
				{
#ifndef DEBUG_PRINTF
					for (i = 0; i < 4; i++)
					{
						UartSendData(g_sMotor.u8MotorSendBuf[i]);
					}
#endif
				}
				break;
			case cSTA_ERR: // 停机状态再发送数据给主芯片
				if (g_sMotor.u8MotorRecvBuf[2] == 0)
				{
					Delay_ms(200); // 看门狗重启
				}
#ifndef DEBUG_PRINTF
				for (i = 0; i < 4; i++)
				{
					UartSendData(g_sMotor.u8MotorSendBuf[i]);
				}
#endif
				break;
			case cSTA_STOP: // 停机状态再发送数据给主芯片
#ifndef DEBUG_PRINTF
				for (i = 0; i < 4; i++)
				{
					UartSendData(g_sMotor.u8MotorSendBuf[i]);
				}
#endif
				break;
			default:
				break;
			}
		}
		else
		{
			g_sMotor.RecvCnt = 0;
			g_sMotor.uWorkFlag.bRecvFinish = 0;
		}
		break;
	default:
		break;
	}
#endif
}
/**
 * @brief  uart发送中断
 * @return Null
 * @note
 */
static void __near r_uart0_interrupt_send(void)
{
}
/**
 * @brief  uart错误中断
 * @return Null
 * @note
 */
static void __near r_uart0_interrupt_error(void)
{
	u8 err_type;

	*gp_uart0_rx_address = RXD0;
	err_type = (uint8_t)(SSR01 & 0x0007U);
	SIR01 = (uint16_t)err_type;
	r_uart0_callback_error(err_type);
}
/************************************************************************************************
 * Function Name: r_uart0_callback_error
 * Description  : This function is a callback function when UART0 reception error occurs.
 * Arguments    : err_type -
 *                    error type value
 * Return Value : None
 *************************************************************************************************/
static void r_uart0_callback_error(uint8_t err_type)
{
	/* Start user code. Do not edit comment generated here */
	/* End user code. Do not edit comment generated here */
}
/*************************************************END OF FILE*********************************************/
