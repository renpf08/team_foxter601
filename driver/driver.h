#ifndef DRIVER_H
#define DRIVER_H

#include"../common/common.h"

typedef struct {
	init uart_init;
	read uart_read;
	write uart_write;
	uninit uart_uninit;
}uart_t;

typedef struct {
	init             timer_init;
	timer_start_func timer_start;
	uninit           timer_uninit;
}timer_t;

typedef struct {
	uart_t *uart;
	timer_t *timer;
}driver_t;

driver_t *get_driver(void);

#endif
