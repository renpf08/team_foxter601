#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

typedef enum {
	state_battery,
	state_week,
}STATE_BATTERY_WEEK_E;

typedef struct {
	u8 cur_state;
}state_battery_week_t;

static state_battery_week_t state_battery_week = {
	.cur_state = state_battery,
};

s16 state_battery_week_switch(REPORT_E cb, void *args)
{
	//clock_t * clk = NULL;
	//u8 battery_level = BAT_PECENT_0;
	STATE_E *state = (STATE_E *)args;

	print((u8 *)&"battery_week", 12);

	if(state_week == state_battery_week.cur_state) {
		state_battery_week.cur_state = state_battery;
		/*get battery level*/
		//battery_level = battery_percent_read();
		motor_battery_week_to_position(BAT_PECENT_0);
	}else {
		state_battery_week.cur_state = state_week;
		/*get week*/
		//clk = clock_get();
		//motor_battery_week_to_position(clk->week);
		motor_battery_week_to_position(SUNDAY);
	}

	*state = CLOCK;
	return 0;
}


static adapter_callback battery_week_cb = NULL;

static void battery_week_test_handler(u16 id)
{
	if(NULL != battery_week_cb) {
		battery_week_cb(KEY_M_SHORT_PRESS, NULL);
		timer_event(5000, battery_week_test_handler);
	}
}

void battery_week_test(adapter_callback cb)
{
	battery_week_cb = cb;
	timer_event(5000, battery_week_test_handler);
}
