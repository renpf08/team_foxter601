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

static s16 csr_motor_activity_positive_first_half(void *args)
{
	return csr_motor_item_positive_first_half(activity_motor, NULL);
}

static s16 csr_motor_activity_positive_second_half(void *args)
{
	return csr_motor_item_positive_second_half(activity_motor, NULL);
}

static s16 csr_motor_activity_negtive_first_half(void *args)
{
	return csr_motor_item_negtive_first_half(activity_motor, NULL);
}

static s16 csr_motor_activity_negtive_second_half(void *args)
{
	return csr_motor_item_negtive_second_half(activity_motor, NULL);
}

static s16 csr_motor_activity_stop(void *args)
{
	return csr_motor_item_stop(activity_motor, NULL);
}

static s16 csr_motor_activity_init(cfg_t *args, event_callback cb)
{
	return csr_motor_item_init(activity_motor, args, cb);
}

motor_t csr_motor_activity = {
	.motor_init = csr_motor_activity_init,
	.motor_positive_first_half = csr_motor_activity_positive_first_half,
	.motor_positive_second_half = csr_motor_activity_positive_second_half,	
	.motor_stop = csr_motor_activity_stop,
	.motor_negtive_first_half = csr_motor_activity_negtive_first_half,
	.motor_negtive_second_half = csr_motor_activity_negtive_second_half,
};
