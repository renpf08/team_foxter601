#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

static void sport_activity_calc(void)
{
    u32 target_steps = cmd_get()->user_info.target_steps;
    u32 current_steps = sport_get()->StepCounts;
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
    s16 res = 0;
	clock_t *clock = clock_get();
    cmd_params_t* params = cmd_get_params();
    SPORT_INFO_T* sport_info = NULL;
    his_data_t data;
    
    MemSet(&data, 0, sizeof(his_data_t));
    switch(cb)
    {
    case READ_STEPS_TARGET:
        params->clock = clock;
        break;
    case READ_STEPS_CURRENT:
    case REFRESH_STEPS:
        sport_info = sport_get();
        data.year = clock->year;
        data.month = clock->month;
        data.day = clock->day;
        data.steps = sport_info->StepCounts;
        #if USE_DEV_CALORIE
        data.colorie = sport_info->Calorie;
        data.acute = sport_info->AcuteSportTimeCounts;
        #endif
        params->data = &data;
        break;
    default:
        break;
    }
    cmd_set_params(params);
    sport_activity_calc();

	*state = CLOCK;
	return res;
}


