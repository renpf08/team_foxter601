#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"../driver.h"

static int csr_uart_read(void *args)
{


}

static int csr_uart_write(void *args)
{


}

static int csr_uart_init(void *args)
{

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
