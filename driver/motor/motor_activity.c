#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <timer.h>
#include "../driver.h"

static motor_cfg_t csr_motor_activity_cfg = {
	.pos = {
		.group = 0,
		.num = 0,
	},
	.com = {
		.group = 0,
		.num = 0,
	},
	.neg = {
		.group = 0,
		.num = 0,
	},
};

static s16 csr_motor_activity_positive_first_half(void *args)
{
	PioSetDir(csr_motor_activity_cfg.pos.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_activity_cfg.neg.num, PIO_DIR_OUTPUT);

	PioSets(BIT_MASK(csr_motor_activity_cfg.pos.num)| \
			BIT_MASK(csr_motor_activity_cfg.com.num)| \
			BIT_MASK(csr_motor_activity_cfg.neg.num),
			0x00UL);

	POS_HIGH(csr_motor_activity_cfg.pos.num);
	return 0;
}

static s16 csr_motor_activity_positive_second_half(void *args)
{
	PioSetDir(csr_motor_activity_cfg.pos.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_activity_cfg.neg.num, PIO_DIR_OUTPUT);

	PioSets(BIT_MASK(csr_motor_activity_cfg.pos.num)| \
			BIT_MASK(csr_motor_activity_cfg.com.num)| \
			BIT_MASK(csr_motor_activity_cfg.neg.num),
			0x00UL);

	COM_HIGH(csr_motor_activity_cfg.com.num);
	NEG_HIGH(csr_motor_activity_cfg.neg.num);	
	return 0;
}

static s16 csr_motor_activity_negtive_first_half(void *args)
{

	PioSetDir(csr_motor_activity_cfg.pos.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_activity_cfg.neg.num, PIO_DIR_OUTPUT);

	PioSets(BIT_MASK(csr_motor_activity_cfg.pos.num)| \
			BIT_MASK(csr_motor_activity_cfg.com.num)| \
			BIT_MASK(csr_motor_activity_cfg.neg.num),
			0x00UL);

	NEG_HIGH(csr_motor_activity_cfg.neg.num);
	return 0;
}

static s16 csr_motor_activity_negtive_second_half(void *args)
{
	PioSetDir(csr_motor_activity_cfg.pos.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_activity_cfg.neg.num, PIO_DIR_OUTPUT);

	PioSets(BIT_MASK(csr_motor_activity_cfg.pos.num)| \
			BIT_MASK(csr_motor_activity_cfg.com.num)| \
			BIT_MASK(csr_motor_activity_cfg.neg.num),
			0x00UL);

	POS_HIGH(csr_motor_activity_cfg.pos.num);
	COM_HIGH(csr_motor_activity_cfg.com.num);
	return 0;
}

static s16 csr_motor_activity_stop(void *args)
{
	PioSets(BIT_MASK(csr_motor_activity_cfg.pos.num)| \
			BIT_MASK(csr_motor_activity_cfg.com.num)| \
			BIT_MASK(csr_motor_activity_cfg.neg.num),
			0x0000UL);
			
	PioSetDir(csr_motor_activity_cfg.pos.num, PIO_DIR_INPUT);
	PioSetDir(csr_motor_activity_cfg.neg.num, PIO_DIR_INPUT);
	return 0;
}

static s16 csr_motor_activity_init(cfg_t *args, event_callback cb)
{
	csr_motor_activity_cfg.pos.group = args->motor_activity_cfg.pos.group;
	csr_motor_activity_cfg.pos.num = args->motor_activity_cfg.pos.num;

	csr_motor_activity_cfg.com.group = args->motor_activity_cfg.com.group;
	csr_motor_activity_cfg.com.num = args->motor_activity_cfg.com.num;

	csr_motor_activity_cfg.neg.group = args->motor_activity_cfg.neg.group;
	csr_motor_activity_cfg.neg.num = args->motor_activity_cfg.neg.num;

	//wait for complete
	PioSetModes(BIT_MASK(csr_motor_activity_cfg.pos.num)| \
				BIT_MASK(csr_motor_activity_cfg.com.num)| \
				BIT_MASK(csr_motor_activity_cfg.neg.num),
				pio_mode_user);
	
	PioSetDir(csr_motor_activity_cfg.pos.num, PIO_DIR_INPUT);
	PioSetDir(csr_motor_activity_cfg.com.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_activity_cfg.neg.num, PIO_DIR_INPUT);
	
	PioSetPullModes(BIT_MASK(csr_motor_activity_cfg.pos.num)| \
					BIT_MASK(csr_motor_activity_cfg.com.num)| \
					BIT_MASK(csr_motor_activity_cfg.neg.num),
					pio_mode_no_pulls);

	PioSets(BIT_MASK(csr_motor_activity_cfg.com.num), 0x0000UL);
	return 0;
}

motor_t csr_motor_activity = {
	.motor_init = csr_motor_activity_init,
	.motor_positive_first_half = csr_motor_activity_positive_first_half,
	.motor_positive_second_half = csr_motor_activity_positive_second_half,	
	.motor_stop = csr_motor_activity_stop,
	.motor_negtive_first_half = csr_motor_activity_negtive_first_half,
	.motor_negtive_second_half = csr_motor_activity_negtive_second_half,
};
