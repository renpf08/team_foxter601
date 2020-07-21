#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

static void sport_activity_calc(void)
{
    u32 target_steps = cmd_get()->user_info.target_steps;
    u32 current_steps = step_get();
    u8 activity = 40; // total 40 grids

    if(current_steps < target_steps) {
        activity = (current_steps*40)/target_steps;
    } else if(target_steps == 0) {
        activity = 0;
    }
    motor_activity_to_position(activity);
    cmd_resp(CMD_SYNC_DATA, 0, (u8*)&"\xF5\xFA");
}
s16 state_step_get(REPORT_E cb, void *args)
{
    STATE_E *state = (STATE_E *)args;
	clock_t *clock = clock_get();
    cmd_params_t params;
    
    params.clock->year = clock->year;
    params.clock->month = clock->month;
    params.clock->day = clock->day;
    params.clock->hour = clock->hour;
    params.clock->minute = clock->minute;
    params.clock->second = clock->second;
    params.clock->week = clock->week;
    params.days = 0;
    params.steps = step_get();
        
    cmd_set_params(&params);
    sport_activity_calc();

	*state = CLOCK;
	return 0;
}


