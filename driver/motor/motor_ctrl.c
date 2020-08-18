#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <timer.h>
#include <mem.h>
#include "../driver.h"

static motor_cfg_t csr_motor_cfg[max_motor];

static s16 csr_motor_run(u8 motor, u8 dir)
{
    static u8 step[none] = {0, 0};
    
	PioSetDir(csr_motor_cfg[motor].pos.num, PIO_DIR_OUTPUT);
	PioSetDir(csr_motor_cfg[motor].neg.num, PIO_DIR_OUTPUT);

	PioSets(BIT_MASK(csr_motor_cfg[motor].pos.num)| \
			BIT_MASK(csr_motor_cfg[motor].com.num)| \
			BIT_MASK(csr_motor_cfg[motor].neg.num),
			0x00UL);

    if(dir == pos) {
        if(step[dir] == 0) {
    	    POS_HIGH(csr_motor_cfg[motor].pos.num);
        } else {
        	COM_HIGH(csr_motor_cfg[motor].com.num);
        	NEG_HIGH(csr_motor_cfg[motor].neg.num);	
        }
        step[dir] = (step[dir]==0)?1:0;
    } else if(dir == neg) {
        if(step[dir] == 0) {
    	    NEG_HIGH(csr_motor_cfg[motor].neg.num);
        } else {
        	POS_HIGH(csr_motor_cfg[motor].pos.num);
        	COM_HIGH(csr_motor_cfg[motor].com.num);
        }
        step[dir] = (step[dir]==0)?1:0;
    }
    
	return 0;
}

static s16 csr_motor_stop(u8 motor)
{
	PioSets(BIT_MASK(csr_motor_cfg[motor].pos.num)| \
			BIT_MASK(csr_motor_cfg[motor].com.num)| \
			BIT_MASK(csr_motor_cfg[motor].neg.num),
			0x0000UL);
			
	PioSetDir(csr_motor_cfg[motor].pos.num, PIO_DIR_INPUT);
	PioSetDir(csr_motor_cfg[motor].neg.num, PIO_DIR_INPUT);
	return 0;
}

static s16 csr_motor_init(cfg_t *args, event_callback cb)
{
    u8 i = 0;
    motor_cfg_t motor_cfg[max_motor] = {
        [minute_motor] = {.pos = {args->motor_minute_cfg.pos.group, args->motor_minute_cfg.pos.num},
                          .com = {args->motor_minute_cfg.com.group, args->motor_minute_cfg.com.num},
                          .neg = {args->motor_minute_cfg.neg.group, args->motor_minute_cfg.neg.num},},
        [hour_motor] = {.pos = {args->motor_hour_cfg.pos.group, args->motor_hour_cfg.pos.num},
                          .com = {args->motor_hour_cfg.com.group, args->motor_hour_cfg.com.num},
                          .neg = {args->motor_hour_cfg.neg.group, args->motor_hour_cfg.neg.num},},
        [activity_motor] = {.pos = {args->motor_activity_cfg.pos.group, args->motor_activity_cfg.pos.num},
                          .com = {args->motor_activity_cfg.com.group, args->motor_activity_cfg.com.num},
                          .neg = {args->motor_activity_cfg.neg.group, args->motor_activity_cfg.neg.num},},
        [date_motor] = {.pos = {args->motor_date_cfg.pos.group, args->motor_date_cfg.pos.num},
                          .com = {args->motor_date_cfg.com.group, args->motor_date_cfg.com.num},
                          .neg = {args->motor_date_cfg.neg.group, args->motor_date_cfg.neg.num},},
        [battery_week_motor] = {.pos = {args->motor_battery_week_cfg.pos.group, args->motor_battery_week_cfg.pos.num},
                          .com = {args->motor_battery_week_cfg.com.group, args->motor_battery_week_cfg.com.num},
                          .neg = {args->motor_battery_week_cfg.neg.group, args->motor_battery_week_cfg.neg.num},},
        [notify_motor] = {.pos = {args->motor_notify_cfg.pos.group, args->motor_notify_cfg.pos.num},
                          .com = {args->motor_notify_cfg.com.group, args->motor_notify_cfg.com.num},
                          .neg = {args->motor_notify_cfg.neg.group, args->motor_notify_cfg.neg.num},},
    };
    
	MemCopy(csr_motor_cfg, motor_cfg, max_motor*sizeof(motor_cfg_t));

	//wait for complete
	for(i = 0; i < max_motor; i++) {
    	PioSetModes(BIT_MASK(csr_motor_cfg[i].pos.num)| \
    				BIT_MASK(csr_motor_cfg[i].com.num)| \
    				BIT_MASK(csr_motor_cfg[i].neg.num),
    				pio_mode_user);
    	
    	PioSetDir(csr_motor_cfg[i].pos.num, PIO_DIR_INPUT);
    	PioSetDir(csr_motor_cfg[i].com.num, PIO_DIR_OUTPUT);
    	PioSetDir(csr_motor_cfg[i].neg.num, PIO_DIR_INPUT);
    	
    	PioSetPullModes(BIT_MASK(csr_motor_cfg[i].pos.num)| \
    					BIT_MASK(csr_motor_cfg[i].com.num)| \
    					BIT_MASK(csr_motor_cfg[i].neg.num),
    					pio_mode_no_pulls);

    	PioSets(BIT_MASK(csr_motor_cfg[i].com.num), 0x0000UL);

        //motor init and off
        csr_motor_stop(i);
	}
	return 0;
}

motor_ctrl_t csr_motor_ctrl = {
	.motor_init = csr_motor_init,
	.motor_stop = csr_motor_stop,
	.motor_run = csr_motor_run,
};
