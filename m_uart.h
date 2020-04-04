
#ifndef __M_UART_H__
#define __M_UART_H__

#include "user_config.h"

#if USE_M_LOG
/**
*  @file    m_uart.h
*  @author  maliwen
*  @date    2019/06/24
*  @history 1.0.0.0(°æ±¾ºÅ)
*           1.0.0.0: Creat              2019/06/24
*/

#define UART_TX_BUF_SIZE                128      //! 128                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                128    //! 128                                         /**< UART RX buffer size. */

uint16 m_uart_recev_irq(void *data, uint16 dataLenInWords, uint16 *more);
void m_uart_init(void);
#endif //! end with USE_M_LOG

#endif /** end of __M_UART_H__ */
