/**
*  @file    m_uart.c
*  @author  maliwen
*  @date    2019/06/24
*  @history 1.0.0.0(版本号)
*           1.0.0.0: Creat              2019/06/24
*/
#include <gatt.h>
#include <timer.h>
#include <panic.h>
#include <mem.h>
#include <uart.h>
#include "user_config.h"
#include "m_uart.h"
#include "m_printf.h"
#include "serial_service.h"

/*============================================================================
 *  Private Definitions 
 *============================================================================*/
/* The length of a message header, in words */
#define MESSAGE_LENGTH_WORDS            (1)

/* This many events can be scheduled. */
#define MAX_QUEUE_ENTRIES               (2)

/* This is the size of the message id that will be read from UART. All the 
 * recognized message ids are defined in a separate header file.
 */
#define MAXIMUM_REQ_MESSAGE_SIZE_WORDS  (1)
#define MAXIMUM_REQ_MESSAGE_SIZE_BYTES  (MAXIMUM_REQ_MESSAGE_SIZE_WORDS << 1)

/* The maximum buffer used to store and send data from the target application
   to the host */
#define MAXIMUM_TX_BUFFER_SIZE_WORDS    (64)
/*============================================================================
 *  Private Data
 *============================================================================*/

/* The buffers used by the UART FW implementation. */
UART_DECLARE_BUFFER(uartRxBuffer, UART_BUF_SIZE_BYTES_32);
UART_DECLARE_BUFFER(uartTxBuffer, UART_BUF_SIZE_BYTES_64);

typedef struct
{
    bool bUartTimerStart; //! 串口超时截断时钟是否已经启动
    uint8 uartBuf[UART_RX_BUF_SIZE]; //! 串口接收缓存
    uint8 bufLen;
} m_uart_ctrl_t;
m_uart_ctrl_t uartCtrl;

static void m_timer_uart_recev_handler(const timer_id id);
void m_uart_recev_timer_handler(void * p_context);

void m_timer_uart_recev_init(void);

static void m_timer_uart_recev_handler(const timer_id id)
{
    uint8 len = uartCtrl.bufLen>20?20:uartCtrl.bufLen;
    SerialSendNotification(uartCtrl.uartBuf, len); //! 发送BLE通知
    m_nprintf((unsigned)uartCtrl.bufLen, (char *)uartCtrl.uartBuf);
    //M_LOG_DEBUG((char *)uartCtrl.uartBuf);
    //m_printf("%s", uartCtrl.uartBuf);
    uartCtrl.bUartTimerStart = FALSE;
    MemSet(uartCtrl.uartBuf, 0, sizeof(uartCtrl.uartBuf));
    uartCtrl.bufLen = 0;
}
/**
* @brief timer clock
* @param [in] none
* @param [out] none
* @return none
* @data 2020/02/20 18:12
* @author maliwen
* @note none
*/
void m_timer_uart_recev_init(void)
{
    /* Now starting a timer */
    const timer_id tId = TimerCreate(50000, TRUE, m_timer_uart_recev_handler);
    
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID)
    {
        m_printf("m_timer_uart_recev_init failed\r\n");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}

/**
* @brief 串口超时中断处理 
* @param [in] none
* @param [out] none
* @return none
* @note none
* @warning none
*/ 
void m_uart_recev_timer_handler(void * p_context)
{
    //MLW_LOG_DEBUG("%d - %s\r\n", uartCtrl.bufLen, uartCtrl.uartBuf);

    //m_cmd_queued(uartCtrl.uartBuf);
    uartCtrl.bUartTimerStart = FALSE;
    MemSet(uartCtrl.uartBuf, 0, sizeof(uartCtrl.uartBuf));
    uartCtrl.bufLen = 0;
}

/**
* @brief 串口接收 
* @param [in] none
* @param [out] none
* @return none
* @note none
* @warning none
*/ 
uint16 m_uart_recev_irq(void *data, uint16 dataLenInWords, uint16 *more)
{
    if(uartCtrl.bUartTimerStart == FALSE)
    {
        uartCtrl.bUartTimerStart = TRUE;
        m_timer_uart_recev_init();
    }
    
    //uint16 val = 0;
    //MemCopy(&val, data, 1);
    //uartCtrl.uartBuf[uartCtrl.bufLen++] = val&0x00FF;
    //uartCtrl.uartBuf[uartCtrl.bufLen++] = (val>>8)&0x00FF;
    MemCopy(&uartCtrl.uartBuf[uartCtrl.bufLen++], data, 1);
    //uartCtrl.uartBuf[uartCtrl.bufLen++] = chr;
    
    *more = 1;
    return 1;
}

/**
* @brief 串口初始化
* @param [in] none
* @param [out] none
* @return none
* @data 2019/06/24 16:22
* @author maliwen
* @note none
*/
void m_uart_init(void)
{
    /* Initialise host interface driver */
    UartInit(m_uart_recev_irq, 
             0, /* uart_data_out_fn */
             uartRxBuffer, UART_BUF_SIZE_BYTES_32,
             uartTxBuffer, UART_BUF_SIZE_BYTES_64,
             uart_data_unpacked);
             
    /* Configure the UART for high baud rate */
    UartConfig(HIGH_BAUD_RATE,0);
        
    /* Enable UART interface */
    UartEnable(TRUE);
    
    /* Initialise local variables */
    //req_buffer_level_in_words = 0;
    
    /* Request notification from the UART when enough data arrives 
     * to form a message header. 
     */
    UartRead(1, 0);
    
    m_timer_uart_recev_init();
}
