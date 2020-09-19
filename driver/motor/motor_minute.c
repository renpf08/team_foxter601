#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <timer.h>
#include "../driver.h"

extern s16 csr_motor_item_init(u16 motor_num, cfg_t *args, event_callback cb);
extern s16 csr_motor_item_positive_first_half(u16 motor_num, void *args);
extern s16 csr_motor_item_positive_second_half(u16 motor_num, void *args);
extern s16 csr_motor_item_negtive_first_half(u16 motor_num, void *args);
extern s16 csr_motor_item_negtive_second_half(u16 motor_num, void *args);
extern s16 csr_motor_item_stop(u16 motor_num, void *args);

static s16 csr_motor_minute_positive_first_half(void *args)
{
	return csr_motor_item_positive_first_half(minute_motor, NULL);
}

static s16 csr_motor_minute_positive_second_half(void *args)
{
	return csr_motor_item_positive_second_half(minute_motor, NULL);
}

static s16 csr_motor_minute_negtive_first_half(void *args)
{
	return csr_motor_item_negtive_first_half(minute_motor, NULL);
}

static s16 csr_motor_minute_negtive_second_half(void *args)
{
	return csr_motor_item_negtive_second_half(minute_motor, NULL);
}

static s16 csr_motor_minute_stop(void *args)
{
	return csr_motor_item_stop(minute_motor, NULL);
}

static s16 csr_motor_minute_init(cfg_t *args, event_callback cb)
{
	return csr_motor_item_init(minute_motor, args, cb);
}

motor_t csr_motor_minute = {
	.motor_init = csr_motor_minute_init,
	.motor_positive_first_half = csr_motor_minute_positive_first_half,
	.motor_positive_second_half = csr_motor_minute_positive_second_half,	
	.motor_stop = csr_motor_minute_stop,
	.motor_negtive_first_half = csr_motor_minute_negtive_first_half,
	.motor_negtive_second_half = csr_motor_minute_negtive_second_half,
};
