#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

void state_run_test_handler(u16 id);


enum{
	run,
	idle,
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
};

void state_run_test_handler(u16 id)
{
	if(pos == run_test.hour_dir) {
		run_test.hour++;
		if(HOUR12_0 == run_test.hour) {
			run_test.hour_dir = neg;
		}
	}

	motor_hour_to_position(run_test.hour);
	motor_minute_to_position(run_test.minute);
	motor_date_to_position(run_test.day);
	motor_notify_to_position(run_test.notify);
	motor_battery_week_to_position(run_test.battery_week);
	motor_activity_to_position(run_test.activity);

	timer_event(1000, state_run_test_handler);
}

s16 state_run_test(REPORT_E cb, void *args)
{
	u8 string[13] = {'s', 't', 'a', 't', 'e', '_', 'r', 'u', 'n', 't', 'e', 's', 't'};
	print(string, 13);

	if(run == run_test.work) {
		run_test.hour = HOUR0_0;
		run_test.minute = MINUTE_0;
		run_test.day = DAY_1;
		run_test.notify = NOTIFY_NONE;
		run_test.battery_week = BAT_PECENT_0;
		run_test.activity = ACTIVITY_0;

		run_test.hour_dir = pos;
		run_test.minute_dir = pos;
		run_test.day_dir = pos;
		run_test.notify_dir = pos;
		run_test.battery_week_dir = pos;
		run_test.activity_dir = pos;

		motor_hour_to_position(HOUR0_0);
		motor_minute_to_position(MINUTE_0);
		motor_date_to_position(DAY_1);
		motor_notify_to_position(NOTIFY_NONE);
		motor_battery_week_to_position(BAT_PECENT_0);
		motor_activity_to_position(ACTIVITY_0);

		timer_event(1000, state_run_test_handler);
		run_test.work = idle;
	}else {

	
	}

	return 0;
}
