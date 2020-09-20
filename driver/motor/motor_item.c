#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <timer.h>
#include "../driver.h"

s16 csr_motor_item_init(u16 motor_num, cfg_t *args, event_callback cb);
s16 csr_motor_item_positive_first_half(u16 motor_num, void *args);
s16 csr_motor_item_positive_second_half(u16 motor_num, void *args);
s16 csr_motor_item_negtive_first_half(u16 motor_num, void *args);
s16 csr_motor_item_negtive_second_half(u16 motor_num, void *args);
s16 csr_motor_item_stop(u16 motor_num, void *args);

#define MOTOR_STATUS_INIT(motor_num, pos_num, com_num, neg_num) \
	[motor_num] = { \
		.pos = { \
			.group = 0, \
			.num = pos_num, \
		}, \
		.com = { \
			.group = 0, \
			.num = com_num, \
		}, \
		.neg = { \
			.group = 0, \
			.num = neg_num, \
		}, \
	}

static motor_cfg_t csr_motor_item_cfg[max_motor] = {
	MOTOR_STATUS_INIT(minute_motor, 0, 0, 0),
	MOTOR_STATUS_INIT(hour_motor, 0, 0, 0),
	MOTOR_STATUS_INIT(activity_motor, 0, 0, 0),
	MOTOR_STATUS_INIT(date_motor, 0, 0, 0),
	MOTOR_STATUS_INIT(battery_week_motor, 0, 0, 0),
	MOTOR_STATUS_INIT(notify_motor, 0, 0, 0),
};

s16 csr_motor_item_init(u16 motor_num, cfg_t *args, event_callback cb)
{
	csr_motor_item_cfg[motor_num].pos.group = args->motor_item_cfg[motor_num].pos.group;
	csr_motor_item_cfg[motor_num].pos.num = args->motor_item_cfg[motor_num].pos.num;

	csr_motor_item_cfg[motor_num].com.group = args->motor_item_cfg[motor_num].com.group;
	csr_motor_item_cfg[motor_num].com.num = args->motor_item_cfg[motor_num].com.num;

	csr_motor_item_cfg[motor_num].neg.group = args->motor_item_cfg[motor_num].neg.group;
	csr_motor_item_cfg[motor_num].neg.num = args->motor_item_cfg[motor_num].neg.num;

	//wait for complete
	PioSetModes(BIT_MASK(csr_motor_item_cfg[motor_num].pos.num)| \
				BIT_MASK(csr_motor_item_cfg[motor_num].com.num)| \
				BIT_MASK(csr_motor_item_cfg[motor_num].neg.num),
				pio_mode_user);
	
	PioSetDir(csr_motor_item_cfg[motor_num].pos.num, PIO_DIR_INPUT);
	PioSetDir(csr_motor_item_cfg[motor_num].com.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_item_cfg[motor_num].neg.num, PIO_DIR_INPUT);
	
	PioSetPullModes(BIT_MASK(csr_motor_item_cfg[motor_num].pos.num)| \
					BIT_MASK(csr_motor_item_cfg[motor_num].com.num)| \
					BIT_MASK(csr_motor_item_cfg[motor_num].neg.num),
					pio_mode_no_pulls);

	PioSets(BIT_MASK(csr_motor_item_cfg[motor_num].com.num), 0x0000UL);

	return 0;
}

s16 csr_motor_item_positive_first_half(u16 motor_num, void *args)
{
	PioSetDir(csr_motor_item_cfg[motor_num].pos.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_item_cfg[motor_num].neg.num, PIO_DIR_OUTPUT);

	PioSets(BIT_MASK(csr_motor_item_cfg[motor_num].pos.num)| \
			BIT_MASK(csr_motor_item_cfg[motor_num].com.num)| \
			BIT_MASK(csr_motor_item_cfg[motor_num].neg.num),
			0x00UL);

	POS_HIGH(csr_motor_item_cfg[motor_num].pos.num);

	return 0;
}

s16 csr_motor_item_positive_second_half(u16 motor_num, void *args)
{
	PioSetDir(csr_motor_item_cfg[motor_num].pos.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_item_cfg[motor_num].neg.num, PIO_DIR_OUTPUT);

	PioSets(BIT_MASK(csr_motor_item_cfg[motor_num].pos.num)| \
			BIT_MASK(csr_motor_item_cfg[motor_num].com.num)| \
			BIT_MASK(csr_motor_item_cfg[motor_num].neg.num),
			0x00UL);

	COM_HIGH(csr_motor_item_cfg[motor_num].com.num);
	NEG_HIGH(csr_motor_item_cfg[motor_num].neg.num);

	return 0;
}

s16 csr_motor_item_negtive_first_half(u16 motor_num, void *args)
{
	PioSetDir(csr_motor_item_cfg[motor_num].pos.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_item_cfg[motor_num].neg.num, PIO_DIR_OUTPUT);

	PioSets(BIT_MASK(csr_motor_item_cfg[motor_num].pos.num)| \
			BIT_MASK(csr_motor_item_cfg[motor_num].com.num)| \
			BIT_MASK(csr_motor_item_cfg[motor_num].neg.num),
			0x00UL);

	NEG_HIGH(csr_motor_item_cfg[motor_num].neg.num);

	return 0;
}

s16 csr_motor_item_negtive_second_half(u16 motor_num, void *args)
{
	PioSetDir(csr_motor_item_cfg[motor_num].pos.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_item_cfg[motor_num].neg.num, PIO_DIR_OUTPUT);

	PioSets(BIT_MASK(csr_motor_item_cfg[motor_num].pos.num)| \
			BIT_MASK(csr_motor_item_cfg[motor_num].com.num)| \
			BIT_MASK(csr_motor_item_cfg[motor_num].neg.num),
			0x00UL);

	POS_HIGH(csr_motor_item_cfg[motor_num].pos.num);
	COM_HIGH(csr_motor_item_cfg[motor_num].com.num);

	return 0;
}

s16 csr_motor_item_stop(u16 motor_num, void *args)
{
	PioSets(BIT_MASK(csr_motor_item_cfg[motor_num].pos.num)| \
			BIT_MASK(csr_motor_item_cfg[motor_num].com.num)| \
			BIT_MASK(csr_motor_item_cfg[motor_num].neg.num),
			0x0000UL);

	PioSetDir(csr_motor_item_cfg[motor_num].pos.num, PIO_DIR_INPUT);
	PioSetDir(csr_motor_item_cfg[motor_num].neg.num, PIO_DIR_INPUT);

	return 0;
}
