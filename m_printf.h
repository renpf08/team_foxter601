
#ifndef __M_PRINTF_H__
#define __M_PRINTF_H__

/**
*  @file    m_printf.h
*  @author  maliwen
*  @date    2020/02/28
*  @history 1.0.0.0(版本号)
*           1.0.0.0: Creat              2020/02/28
*/

typedef char *  va_list;  
#define _INTSIZEOF(n)	((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))  
#define va_start(ap,v)	(ap = (va_list)&v + _INTSIZEOF(v) )  
#define va_arg(ap,t)	(*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))  
#define va_end(ap)		(ap = (va_list)0)

#define M_SEND_BUFF_SIZE    255 //! 输出到串口的最大数据长度
#define M_RECV_BUFF_SIZE    255 //! 接收串口输入的最大数据长度
#define M_PRINT_BUFF_SIZE   64 //! 输出到串口前，用于格式化数据的缓存区长度

int m_printf(const char * sFormat, ...);
int m_sprintf(char *buf, const char * sFormat, ...);
void m_printf_test(void);

#endif /** end of __M_PRINTF_H__ */
