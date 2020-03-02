#ifndef DRIVER_H
#define DRIVER_H

#include"../common/common.h"

typedef int (*event_callback)(EVENT_E ev);

typedef int (*init)(void *args);
typedef int (*run)(void);
typedef int (*uninit)(void);
typedef int (*read)(void *args);
typedef int (*write)(void *args);

typedef struct {
	init uart_init;
	read uart_read;
	write uart_write;
	uninit uart_uninit;
}uart_t;


typedef struct {
	uart_t uart;
	
}driver_t;

#endif
