#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

void state_run_test_handler(u16 id);
void state_run_test_increase(void);
void state_run_test_exit(u16 id);

enum{
	run,
	idle,
};

enum{
	looping,
	no_loop,
};

typedef struct {
	u8 hour;
	u8 hour_dir;
	u8 minute;
	u8 minute_dir;
	u8 day;
	u8 day_dir;
	u8 notify;
	u8 notify_dir;
	u8 battery_week;
	u8 battery_week_dir;
	u8 activity;
	u8 activity_dir;
	u8 work;
	u8 test_status;
	STATE_E *state;
}run_test_t;

static run_test_t run_test = {
	.hour = HOUR0_0,
	.hour_dir = pos,
	.minute = MINUTE_0,
	.minute_dir = pos,
	.day = DAY_1,
	.day_dir = pos,
	.notify = NOTIFY_NONE,
	.notify_dir = pos,
	.battery_week = BAT_PECENT_0,
	.battery_week_dir = pos,
	.activity = ACTIVITY_0,
	.activity_dir = pos,
	.work = run,
	.test_status = no_loop,
	.state = NULL,
};

void state_run_test_increase(void)
{
	if(pos == run_test.hour_dir) {
		run_test.hour++;
		if(HOUR12_0 == run_test.hour) {
			run_test.hour_dir = neg;
		}
	}else {
		run_test.hour--;
		if(HOUR0_0 == run_test.hour) {
			run_test.hour_dir = pos;
		}
	}

	if(pos == run_test.minute_dir) {
		run_test.minute++;
		if(MINUTE_60 == run_test.minute) {
			run_test.minute_dir = neg;
		}
	}else {
		run_test.minute--;
		if(MINUTE_0 == run_test.minute) {
			run_test.minute_dir = pos;
		}
	}

	if(pos == run_test.day_dir) {
		run_test.day--;
		if(DAY_31 == run_test.day) {
			run_test.day_dir = neg;
		}
	}else {
		run_test.day++;
		if(DAY_1 == run_test.day) {
			run_test.day_dir = pos;
		}
	}

	if(pos == run_test.battery_week_dir) {
		run_test.battery_week--;
		if(SUNDAY == run_test.battery_week) {
			run_test.battery_week_dir = neg;
		}
	}else {
		run_test.battery_week++;
		if(BAT_PECENT_0 == run_test.battery_week) {
			run_test.battery_week_dir = pos;
		}
	}

	if(pos == run_test.notify_dir) {
		run_test.notify++;
		if((NOTIFY_DONE - 1) == run_test.notify) {
			run_test.notify_dir = neg;
		}
	}else {
		run_test.notify--;
		if(NOTIFY_NONE == run_test.notify) {
			run_test.notify_dir = pos;
		}
	}

	if(pos == run_test.activity_dir) {
		run_test.activity++;
		if(ACTIVITY_100 == run_test.activity) {
			run_test.activity_dir = neg;
		}
	}else {
		run_test.activity--;
		if(ACTIVITY_0 == run_test.activity) {
			run_test.activity_dir = pos;
		}
	}
}

void state_run_test_handler(u16 id)
{
	state_run_test_increase();

	motor_hour_one_step(run_test.hour);
	motor_minute_one_step(run_test.minute);	
	motor_date_to_position(run_test.day);
	motor_notify_to_position(run_test.notify);
	motor_battery_week_to_position(run_test.battery_week);
	motor_activity_to_position(run_test.activity);

	if(looping == run_test.test_status) {
		timer_event(1000, state_run_test_handler);
	}
}

void state_run_test_exit(u16 id)
{
	*(run_test.state) = CLOCK;
}

s16 state_run_test(REPORT_E cb, void *args)
{
	u8 string[13] = {'s', 't', 'a', 't', 'e', '_', 'r', 'u', 'n', 't', 'e', 's', 't'};
	print(string, 13);

	if(run == run_test.work) {
		run_test.work = idle;
		run_test.state = (STATE_E *)args;
		if(HOUR12_0 == run_test.hour) {
			run_test.hour_dir = neg;
		}else {
			run_test.hour_dir = pos;
		}

		if(MINUTE_60 == run_test.minute) {
			run_test.minute_dir = neg;
		}else {
			run_test.minute_dir = pos;
		}

		if(DAY_31 == run_test.day) {
			run_test.day_dir = neg;
		}else {
			run_test.day_dir = pos;
		}

		if(SUNDAY == run_test.battery_week) {
			run_test.battery_week_dir = neg;
		}else {
			run_test.battery_week_dir = pos;
		}

		if((NOTIFY_DONE - 1) == run_test.notify) {
			run_test.notify_dir = neg;
		}else {
			run_test.notify_dir = pos;
		}
		
		if(ACTIVITY_100 == run_test.activity) {
			run_test.activity_dir = neg;
		}else {
			run_test.activity_dir = pos;
		}

		run_test.test_status = looping;
		timer_event(1, state_run_test_handler);
	}else {
		run_test.work = run;
		run_test.test_status = no_loop;	
		timer_event(1100, state_run_test_exit);
	}

	return 0;
}

void test_run_test(u16 id)
{
	state_run_test(KEY_A_B_M_LONG_PRESS, NULL);
	timer_event(10000, test_run_test);
}
