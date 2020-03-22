#ifndef DRIVER_H
#define DRIVER_H

#include"../common/common.h"

typedef struct {
	init 			uart_init;
	read 			uart_read;
	write 			uart_write;
	uninit 			uart_uninit;
}uart_t;

typedef struct {
	init             timer_init;
	timer_start_func timer_start;
	uninit           timer_uninit;
}timer_t;

typedef struct {
	init             battery_init;
	read			 battery_voltage_read;
	uninit           battery_uninit;
}battery_t;

typedef struct {
	init			key_init;
	uninit          key_uninit;
}key_t;

typedef struct {
	init			flash_init;
	fread			flash_read;
	fwrite			flash_write;
	uninit			flash_uninit;
}flash_t;

typedef struct{
	init			vibrator_init;
	on			    vibrator_on;
	off			    vibrator_off;
	uninit			vibrator_uninit;
}vibrator_t;

typedef struct {
	init 			gsensor_init;
	read			gsensor_read;
	uninit			gsensor_uninit;
}gsensor_t;

typedef struct {
	init 			magnetometer_init;
	read			magnetometer_read;
	uninit			magnetometer_uninit;
}magnetometer_t;

typedef struct {
	uart_t 		*uart;
	timer_t 	*timer;
	battery_t 	*battery;
	key_t     	*keya;
	flash_t     *flash;
	vibrator_t  *vibrator;
	gsensor_t   *gsensor;
	magnetometer_t *magnetometer;
}driver_t;

s16 csr_keya_event_handler(u16 key_num, u16 key_status);
driver_t *get_driver(void);

#endif
