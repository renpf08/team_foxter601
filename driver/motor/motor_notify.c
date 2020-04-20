#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <timer.h>
#include "../driver.h"

#define POS_HIGH(num) PioSet((num), 1UL)
#define POS_LOW(num) PioSet((num), 0UL)

#define COM_HIGH(num) PioSet((num), 1UL)
#define COM_LOW(num) PioSet((num), 0UL)

#define NEG_HIGH(num) PioSet((num), 1UL)
#define NEG_LOW(num) PioSet((num), 0UL)

typedef struct {
	motor_cfg_t cfg;
}csr_motor_notify_cfg_t;

static csr_motor_notify_cfg_t csr_motor_notify_cfg = {
	.cfg = {
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
	},
};

static s16 csr_motor_notify_positive_first_half(void *args)
{
	PioSets(BIT_MASK(csr_motor_notify_cfg.cfg.pos.num)| \
			BIT_MASK(csr_motor_notify_cfg.cfg.com.num)| \
			BIT_MASK(csr_motor_notify_cfg.cfg.neg.num),
			0x00UL);

	POS_HIGH(csr_motor_notify_cfg.cfg.pos.num);
	return 0;
}

static s16 csr_motor_notify_positive_second_half(void *args)
{
	PioSets(BIT_MASK(csr_motor_notify_cfg.cfg.pos.num)| \
			BIT_MASK(csr_motor_notify_cfg.cfg.com.num)| \
			BIT_MASK(csr_motor_notify_cfg.cfg.neg.num),
			0x00UL);

	COM_HIGH(csr_motor_notify_cfg.cfg.com.num);
	NEG_HIGH(csr_motor_notify_cfg.cfg.neg.num);	
	return 0;
}

static s16 csr_motor_notify_negtive_first_half(void *args)
{
	PioSets(BIT_MASK(csr_motor_notify_cfg.cfg.pos.num)| \
			BIT_MASK(csr_motor_notify_cfg.cfg.com.num)| \
			BIT_MASK(csr_motor_notify_cfg.cfg.neg.num),
			0x00UL);

	NEG_HIGH(csr_motor_notify_cfg.cfg.neg.num);
	return 0;
}

static s16 csr_motor_notify_negtive_second_half(void *args)
{
	PioSets(BIT_MASK(csr_motor_notify_cfg.cfg.pos.num)| \
			BIT_MASK(csr_motor_notify_cfg.cfg.com.num)| \
			BIT_MASK(csr_motor_notify_cfg.cfg.neg.num),
			0x00UL);

	POS_HIGH(csr_motor_notify_cfg.cfg.pos.num);
	COM_HIGH(csr_motor_notify_cfg.cfg.com.num);
	return 0;
}

static s16 csr_motor_notify_stop(void *args)
{
	PioSets(BIT_MASK(csr_motor_notify_cfg.cfg.pos.num)| \
			BIT_MASK(csr_motor_notify_cfg.cfg.com.num)| \
			BIT_MASK(csr_motor_notify_cfg.cfg.neg.num),
			0x0000UL);
			
	//TimeDelayUSec(1);
	return 0;
}

static s16 csr_motor_notify_init(cfg_t *args, event_callback cb)
{
	csr_motor_notify_cfg.cfg.pos.group = args->motor_notify_cfg.pos.group;
	csr_motor_notify_cfg.cfg.pos.num = args->motor_notify_cfg.pos.num;

	csr_motor_notify_cfg.cfg.com.group = args->motor_notify_cfg.com.group;
	csr_motor_notify_cfg.cfg.com.num = args->motor_notify_cfg.com.num;

	csr_motor_notify_cfg.cfg.neg.group = args->motor_notify_cfg.neg.group;
	csr_motor_notify_cfg.cfg.neg.num = args->motor_notify_cfg.neg.num;

	//wait for complete
	PioSetModes(BIT_MASK(csr_motor_notify_cfg.cfg.pos.num)| \
				BIT_MASK(csr_motor_notify_cfg.cfg.com.num)| \
				BIT_MASK(csr_motor_notify_cfg.cfg.neg.num),
				pio_mode_user);
	
	PioSetDir(csr_motor_notify_cfg.cfg.pos.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_notify_cfg.cfg.com.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_notify_cfg.cfg.neg.num, PIO_DIR_OUTPUT);
	
	PioSetPullModes(BIT_MASK(csr_motor_notify_cfg.cfg.pos.num)| \
					BIT_MASK(csr_motor_notify_cfg.cfg.com.num)| \
					BIT_MASK(csr_motor_notify_cfg.cfg.neg.num),
					pio_mode_no_pulls);

	PioSets(BIT_MASK(csr_motor_notify_cfg.cfg.pos.num)| \
			BIT_MASK(csr_motor_notify_cfg.cfg.com.num)| \
			BIT_MASK(csr_motor_notify_cfg.cfg.neg.num),
			0x0000UL);
	return 0;
}

static s16 csr_motor_notify_uninit(void)
{
	csr_motor_notify_cfg.cfg.pos.group = 0;
	csr_motor_notify_cfg.cfg.pos.num = 0;

	csr_motor_notify_cfg.cfg.com.group = 0;
	csr_motor_notify_cfg.cfg.com.num = 0;

	csr_motor_notify_cfg.cfg.neg.group = 0;
	csr_motor_notify_cfg.cfg.neg.num = 0;

	return 0;
}

motor_t csr_motor_notify = {
	.motor_init = csr_motor_notify_init,
	.motor_positive_first_half = csr_motor_notify_positive_first_half,
	.motor_positive_second_half = csr_motor_notify_positive_second_half,	
	.motor_stop = csr_motor_notify_stop,
	.motor_negtive_first_half = csr_motor_notify_negtive_first_half,
	.motor_negtive_second_half = csr_motor_notify_negtive_second_half,
	.motor_uninit = csr_motor_notify_uninit,
};
