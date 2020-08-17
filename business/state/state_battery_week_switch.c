#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

s16 state_battery_week_switch(REPORT_E cb, void *args)
{
//	clock_t * clk = NULL;
//	u8 battery_level = BAT_PECENT_0;
	STATE_E *state = (STATE_E *)args;
    motor_ctrl_queue_t queue_param = {.user = QUEUE_USER_BATWEEK_SWITCH, .intervel = 10, .mask = MOTOR_MASK_BAT_WEEK};

	//print((u8 *)&"battery_week", 12);
	if(state_week == adapter_ctrl.current_bat_week_sta) {
		adapter_ctrl.current_bat_week_sta = state_battery;
	}else {
		adapter_ctrl.current_bat_week_sta = state_week;
	}
    queue_param.dest[battery_week_motor] = get_battery_week_pos(adapter_ctrl.current_bat_week_sta);
    motor_ctrl_enqueue(&queue_param);

	*state = CLOCK;
	return 0;
}

#if 0
static adapter_callback battery_week_cb = NULL;
static void battery_week_test_handler(u16 id)
{
	if(NULL != battery_week_cb) {
		battery_week_cb(KEY_M_SHORT_PRESS, NULL);
		timer_event(10000, battery_week_test_handler);
	}
}

void battery_week_test(adapter_callback cb)
{
	battery_week_cb = cb;
	battery_week_test_handler(0);
}
#endif
