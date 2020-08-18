#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <timer.h>
#include "../driver.h"

static motor_cfg_t csr_motor_hour_cfg = {
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

static s16 csr_motor_hour_run(void *args)
{
    static u8 step[none] = {0, 0};
    u8 dir = (u8)args;

	PioSetDir(csr_motor_hour_cfg.pos.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_hour_cfg.neg.num, PIO_DIR_OUTPUT);

	PioSets(BIT_MASK(csr_motor_hour_cfg.pos.num)| \
			BIT_MASK(csr_motor_hour_cfg.com.num)| \
			BIT_MASK(csr_motor_hour_cfg.neg.num),
			0x00UL);

    if(dir == pos) {
        if(step[dir] == 0) {
    	    POS_HIGH(csr_motor_hour_cfg.pos.num);
        } else {
        	COM_HIGH(csr_motor_hour_cfg.com.num);
        	NEG_HIGH(csr_motor_hour_cfg.neg.num);	
        }
        step[dir] = (step[dir]==0)?1:0;
    } else if(dir == neg) {
        if(step[dir] == 0) {
    	    NEG_HIGH(csr_motor_hour_cfg.neg.num);
        } else {
        	POS_HIGH(csr_motor_hour_cfg.pos.num);
        	COM_HIGH(csr_motor_hour_cfg.com.num);
        }
        step[dir] = (step[dir]==0)?1:0;
    }
    
	return 0;
}

static s16 csr_motor_hour_stop(void *args)
{
	PioSets(BIT_MASK(csr_motor_hour_cfg.pos.num)| \
			BIT_MASK(csr_motor_hour_cfg.com.num)| \
			BIT_MASK(csr_motor_hour_cfg.neg.num),
			0x0000UL);

	PioSetDir(csr_motor_hour_cfg.pos.num, PIO_DIR_INPUT);
	PioSetDir(csr_motor_hour_cfg.neg.num, PIO_DIR_INPUT);
	return 0;
}

static s16 csr_motor_hour_init(cfg_t *args, event_callback cb)
{
	csr_motor_hour_cfg.pos.group = args->motor_hour_cfg.pos.group;
	csr_motor_hour_cfg.pos.num = args->motor_hour_cfg.pos.num;

	csr_motor_hour_cfg.com.group = args->motor_hour_cfg.com.group;
	csr_motor_hour_cfg.com.num = args->motor_hour_cfg.com.num;

	csr_motor_hour_cfg.neg.group = args->motor_hour_cfg.neg.group;
	csr_motor_hour_cfg.neg.num = args->motor_hour_cfg.neg.num;

	//wait for complete
	PioSetModes(BIT_MASK(csr_motor_hour_cfg.pos.num)| \
				BIT_MASK(csr_motor_hour_cfg.com.num)| \
				BIT_MASK(csr_motor_hour_cfg.neg.num),
				pio_mode_user);
	
	PioSetDir(csr_motor_hour_cfg.pos.num, PIO_DIR_INPUT);
	PioSetDir(csr_motor_hour_cfg.com.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_hour_cfg.neg.num, PIO_DIR_INPUT);	
	
	PioSetPullModes(BIT_MASK(csr_motor_hour_cfg.pos.num)| \
					BIT_MASK(csr_motor_hour_cfg.com.num)| \
					BIT_MASK(csr_motor_hour_cfg.neg.num),
					pio_mode_no_pulls);

	PioSets(BIT_MASK(csr_motor_hour_cfg.com.num), 0x0000UL);
	return 0;
}

motor_t csr_motor_hour = {
	.motor_init = csr_motor_hour_init,
	.motor_stop = csr_motor_hour_stop,
	.motor_run = csr_motor_hour_run,
};
