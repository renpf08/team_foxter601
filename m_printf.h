
#ifndef __M_PRINTF_H__
#define __M_PRINTF_H__

/**
*  @file    m_printf.h
*  @author  maliwen
*  @date    2020/02/28
*  @history 1.0.0.0(版本号)
*           1.0.0.0: Creat              2020/02/28
*/

#include "user_config.h"

#if USE_M_LOG
typedef char *  va_list;  
#define _INTSIZEOF(n)	((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))  
#define va_start(ap,v)	(ap = (va_list)&v + _INTSIZEOF(v) )  
#define va_arg(ap,t)	(*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))  
#define va_end(ap)		(ap = (va_list)0)

#define M_PRINT_BUFF_SIZE    128 //! printf格式化buffer最大长度

#define M_LOG_ERROR(...)    m_log(__FILE__, __func__, __LINE__, "<error>", __VA_ARGS__)
#define M_LOG_WARNING(...)  m_log(__FILE__, __func__, __LINE__, "<warning>", __VA_ARGS__)
#define M_LOG_INFO(...)     m_log(__FILE__, __func__, __LINE__, "<info>", __VA_ARGS__)
#define M_LOG_DEBUG(...)    m_log(__FILE__, __func__, __LINE__, "<debug>", __VA_ARGS__)

int m_printf(const char * sFormat, ...);
int m_nprintf(unsigned size, const char * sFormat, ...);
int m_sprintf(char *buf, const char * sFormat, ...);
int m_snprintf(char *buf, unsigned size, const char * sFormat, ...);
int m_log(const char* file, const char* func, unsigned line, const char* level, const char * sFormat, ...);
void m_printf_init(void);
#endif //! end with USE_M_LOG

#endif /** end of __M_PRINTF_H__ */
