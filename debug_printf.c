/*
 * @Descripttion : 打印输出文件c文件
 * @version      : 1.0.0
 * @Author       : jack ye
 * @Date         : 2022-10-15 17:44:52
 * @LastEditors  : jack ye
 * @LastEditTime : 2022-12-10 08:13:07
 */

/* Includes ------------------------------------------------------------------*/
#include "debug_printf.h"
#include <stdarg.h>
#include "sau.h"

/* Define --------------------------------------------------------------------*/

/* Parameter -----------------------------------------------------------------*/

/* Function ------------------------------------------------------------------*/

#ifdef DEBUG_PRINTF
/*
 * @name        : int2char
 * @msg         : int转换为char
 * @Date        : 2022-12-02 13:43:10
 * @param        {int} *value	要转换的整形数
 * @param        {char} *string 转换后的字符串
 * @param        {int} radix =10 表示10进制，其他结果为0
 * @return       {*}
 */
static char *int2char(int value, char *string, int radix)
{
	int i, d;
	int flag = 0;
	char *ptr = string;

	//仅适用于十进制数
	if (radix != 10)
	{
		*ptr = 0;
		return string;
	}

	if (!value)
	{
		*ptr++ = 0x30;
		*ptr = 0;
		return string;
	}

	//如果这是一个负值插入负号
	if (value < 0)
	{
		*ptr++ = '-';

		//输出值为负号
		value *= -1;
	}

	for (i = 10000; i > 0; i /= 10)
	{
		d = value / i;

		if (d || flag)
		{
			*ptr++ = (char)(d + 0x30);
			value -= (d * i);
			flag = 1;
		}
	}

	//空的终止符
	*ptr = 0;

	return string;
}
/*
 * @name        : float2char
 * @msg         : float转换为字符
 * @Date        : 2022-12-02 13:43:10
 * @param        {float} slope	浮点型数
 * @param        {char} *buffer 存储的字符数组
 * @param        {int} n 字符数组的长度
 * @return       {*}
 */
static void float2char(float slope, char *buffer, int n)
{
	int temp, i, j;

	if (slope >= 0) // 判断是否大于0
		buffer[0] = '+';
	else
	{
		buffer[0] = '-';
		slope = -slope;
	}

	temp = (int)slope;			// 取整数部分
	for (i = 0; temp != 0; i++) // 计算整数部分的位数
		temp /= 10;
	temp = (int)slope;

	for (j = i; j > 0; j--) // 将整数部分转换成字符串型
	{
		buffer[j] = temp % 10 + '0';
		temp /= 10;
	}

	buffer[i + 1] = '.';
	slope -= (int)slope;
	for (i = i + 2; i < n - 1; i++) // 将小数部分转换成字符串型
	{
		slope *= 10;
		buffer[i] = (int)slope + '0';
		slope -= (int)slope;
	}

	buffer[n - 1] = '\0';
}
/*
 * @name        : _dbg_printf
 * @msg         : 打印
 * @Date        : 2022-12-05 10:09:05
 * @return       {*}
 */
void _dbg_printf(const char *format, ...)
{
	const char *s;
	int d;
	char buf[16];
	unsigned char txdata;

	va_list ap = 0;
	va_start(ap, format);

	while (*format != 0)
	{
		if (*format == 0x5c)
		{
			switch (*++format)
			{
			case 'r':
				txdata = 0x0d;
				UartSendData(txdata);
				format++;
				break;
			case 'n':
				txdata = 0x0a;
				UartSendData(txdata);
				format++;
				break;
			default:
				format++;
				break;
			}
		}
		else if (*format == '%')
		{
			switch (*++format)
			{
			case 's':
				s = va_arg(ap, const char *);
				for (; *s; s++)
				{
					UartSendData(*((unsigned char *)s));
				}
				format++;
				break;
			case 'd':
				d = va_arg(ap, int);
				int2char(d, buf, 10);
				for (s = buf; *s; s++)
				{
					UartSendData(*((unsigned char *)s));
				}
				format++;
				break;
			case 'x':
			{
				d = va_arg(ap, int);
				int2char(d, buf, 16);
				for (s = buf; *s; s++)
				{
					UartSendData(*((unsigned char *)s));
				}
				format++;
				break;
			}
			case 'f':
			{
				double num = va_arg(ap, double);
				float2char(num, buf, 10);//转换经度为10
				for (s = buf; *s; s++)
				{
					UartSendData(*((unsigned char *)s));
				}
				format++;
				break;
			}
			default:
				format++;
				break;
			}
		}
		else
		{
			UartSendData(*((unsigned char *)format));
			format++;
		}
	}
}
#endif
/*************************************************END OF FILE*********************************************/