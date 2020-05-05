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
	.cur_state = state_week,
};

s16 state_battery_week_switch(REPORT_E cb, void *args)
{
	clock_t * clk = NULL;
	u8 battery_level = BAT_PECENT_0;
	STATE_E *state = (STATE_E *)args;

	u8 string[12] = {'s', 't', 'a', 't', 'e', '_', 'b', 'a', 't', 't', 'r', 'y'};
	print(string, 12);

	if(state_week == state_battery_week.cur_state) {
		state_battery_week.cur_state = state_battery;
		/*get battery level*/
		battery_level = battery_percent_read();
		motor_battery_week_to_position(battery_level);
	}else {
		state_battery_week.cur_state = state_week;
		/*get week*/
		clk = clock_get();
		motor_battery_week_to_position(clk->week);
	}

	*state = CLOCK;
	return 0;
}
