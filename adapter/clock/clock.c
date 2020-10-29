#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <timer.h>          /* Chip timer functions */
#include "../adapter.h"

s16 clock_init(adapter_callback cb);

typedef struct {
	driver_t *drv;
	clock_t clock;
	adapter_callback cb;
	s16 tid;
}clock_cfg_t;

static clock_cfg_t clock_cfg = {
	.drv = NULL,
	.clock = {
		.year = 1970,
		.month = 1,
		.day = 1,
		.week = MONDAY,
		.hour = 0,
		.minute = 0,
		.second = 0,
	},
	.cb = NULL,
	.tid = TIMER_INVALID,
};

/*use external variables to save more space than internal variables*/
u8 day[13] = {0, 31,28,31,30,31,30,31,31,30,31,30,31};
static void clock_timer_increase(void)
{
    u8 update_flag = 0;
    day[2] = (((clock_cfg.clock.year%4==0) && (clock_cfg.clock.year%100!=0))||(clock_cfg.clock.year%400==0))?29:28;
    
	if(clock_cfg.clock.second > 59) {
		clock_cfg.clock.second = 0;
		clock_cfg.clock.minute++;
        update_flag = 1;
	}

	if(clock_cfg.clock.minute > 59) {
		clock_cfg.clock.minute = 0;
		clock_cfg.clock.hour++;
	}

	if(clock_cfg.clock.hour > 23) {
		clock_cfg.clock.hour = 0;
		clock_cfg.clock.day++;
		clock_cfg.clock.week++;
		if(clock_cfg.clock.week > SUNDAY) {
			clock_cfg.clock.week = MONDAY;
		}
	}

	if(clock_cfg.clock.day > day[clock_cfg.clock.month]) {
		clock_cfg.clock.day = 1;
		clock_cfg.clock.month++;
	}

	if(clock_cfg.clock.month > 12) {
		clock_cfg.clock.month = 1;
		clock_cfg.clock.year++;
	}

    if(update_flag == 1) {
		if(NULL != clock_cfg.cb) {
			clock_cfg.cb(CLOCK_1_MINUTE, NULL);
		}
    }
    /*
    cmd_set_time_t time;
    time.cmd = 02;
    time.year[0] = clock_cfg.clock.year&0x00FF;
    time.year[1] = clock_cfg.clock.year>>8&0x00FF;
    time.month = clock_cfg.clock.month;
    time.day = clock_cfg.clock.day;
    time.hour = clock_cfg.clock.hour;
    time.minute = clock_cfg.clock.minute;
    time.second = clock_cfg.clock.second;
    time.week = clock_cfg.clock.week;
    BLE_SEND_LOG((u8*)&time, sizeof(cmd_set_time_t));*/
}

static void clock_cb_handler(u16 id)
{
	clock_cfg.tid = TIMER_INVALID;
	clock_cfg.tid = clock_cfg.drv->timer->timer_start(1000, clock_cb_handler);
	clock_cfg.clock.second++;
	clock_timer_increase();	
}

s16 clock_init(adapter_callback cb)
{
	clock_cfg.cb = cb;
	clock_cfg.drv = get_driver();
	clock_cfg.drv->timer->timer_start(1000, clock_cb_handler);
	return 0;
}

clock_t *clock_get(void)
{
	return &clock_cfg.clock;
}

