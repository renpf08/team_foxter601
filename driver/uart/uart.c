#include <gatt.h>
#include <timer.h>
#include <panic.h>
#include <mem.h>
#include <uart.h>
#include "user_config.h"
#include "../driver.h"

static int csr_uart_read(void *args)
{

}

static int csr_uart_write(void *args)
{

}

static int csr_uart_init(void *args)
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
	return 0;
}

static int csr_uart_uninit(void)
{

	return 0;
}

uart_t csr_uart = {
	.uart_init = csr_uart_init,
	.uart_read = csr_uart_read,
	.uart_write = csr_uart_write,
	.uart_uninit = csr_uart_uninit,
};
