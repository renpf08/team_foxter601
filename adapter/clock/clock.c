#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../adapter.h"

s16 clock_init(adapter_callback cb);

typedef struct {
	driver_t *drv;
	clock_t clock;
	adapter_callback cb;
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
};

static void clock_timer_increase(void)
{
	if(60 == clock_cfg.clock.second) {
		clock_cfg.clock.second = 0;
		clock_cfg.clock.minute++;
		if(NULL != clock_cfg.cb) {
			clock_cfg.cb(CLOCK_1_MINUTE, NULL);
		}
	}

	if(60 == clock_cfg.clock.minute) {
		clock_cfg.clock.minute = 0;
		clock_cfg.clock.hour++;
	}

	if(24 == clock_cfg.clock.hour) {
		clock_cfg.clock.hour = 0;
		clock_cfg.clock.day++;
		clock_cfg.clock.week++;
		if(7 == clock_cfg.clock.week) {
			clock_cfg.clock.week = SUNDAY;
		}
	}

	if(31 == clock_cfg.clock.day) {
		clock_cfg.clock.day = 1;
		clock_cfg.clock.month++;
	}

	if(13 == clock_cfg.clock.month) {
		clock_cfg.clock.month = 1;
		clock_cfg.clock.year++;
	}
}

static void clock_cb_handler(u16 id)
{
	clock_cfg.drv->timer->timer_start(1000, clock_cb_handler);
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

s16 clock_set(clock_t *ck)
{
	clock_cfg.clock.year = ck->year;
	clock_cfg.clock.month = ck->month;
	clock_cfg.clock.day = ck->day;
	clock_cfg.clock.week = ck->week;
	clock_cfg.clock.hour = ck->hour;
	clock_cfg.clock.minute = ck->minute;
	clock_cfg.clock.second = ck->second;
	return 0;
}
