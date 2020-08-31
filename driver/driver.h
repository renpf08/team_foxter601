#ifndef DRIVER_H
#define DRIVER_H

#include "../common/common.h"
#include <timer.h>          /* Chip timer functions */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <panic.h>          /* Support for applications to panic */
#include <debug.h>          /* Simple host interface to the UART driver */
#include "../ancs_client.h"

typedef struct {
	init 			uart_init;
	read 			uart_read;
	write 			uart_write;
}uart_t;

typedef struct {
	init             timer_init;
	timer_start_func timer_start;
	timer_delete	 timer_del;
}timer_t;

typedef struct {
	init             battery_init;
	read			 battery_voltage_read;
}battery_t;

typedef struct {
	init			key_init;
}key_t;

typedef struct {
	init			flash_init;
	fread			flash_read;
	fwrite			flash_write;
}flash_t;

typedef struct{
	init			vibrator_init;
	on			    vibrator_on;
	off			    vibrator_off;
}vibrator_t;

typedef struct {
	init 			gsensor_init;
	read			gsensor_read;
}gsensor_t;

typedef struct {
	init 			magnetometer_init;
	read			magnetometer_read;
}magnetometer_t;

typedef struct {
	init			motor_init;
	positive        motor_positive_first_half;
	positive        motor_positive_second_half;	
	stop			motor_stop;
	negtive         motor_negtive_first_half;	
	negtive         motor_negtive_second_half;
}motor_t;

typedef struct {
    #if USE_UART_PRINT
	uart_t 		*uart;
    #endif
	timer_t 	*timer;
	battery_t 	*battery;
	key_t     	*keya;
	key_t     	*keyb;
	key_t     	*keym;	
	flash_t     *flash;
	vibrator_t  *vibrator;
	gsensor_t   *gsensor;
	magnetometer_t *magnetometer;
	motor_t		*motor_hour;
	motor_t		*motor_minute;
	motor_t		*motor_activity;
	motor_t		*motor_date;
	motor_t		*motor_battery_week;
	motor_t		*motor_notify;
}driver_t;

s16 timer_create(uint32 timeout, timer_callback_arg handler);
s16 csr_keya_event_handler(u32 key_num, u32 key_status);
s16 csr_keyb_event_handler(u32 key_num, u32 key_status);
s16 csr_keym_event_handler(u32 key_num, u32 key_status);
s16 csr_magnetometer_event_handler(u32 num, u32 status);
driver_t *get_driver(void);

#endif
