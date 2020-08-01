#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

typedef struct {
    u8 start;
    u8 end;
} motor_test_t;

u8 motor_rdc[max_motor] = {0,0,0,0,0,0};
u8 run_flag = 0;
motor_test_t motor_test[max_motor] = {
    [minute_motor] = {MINUTE_0, MINUTE_60},
    [hour_motor] = {HOUR0_0, HOUR12_0},
    [activity_motor] = {ACTIVITY_0, ACTIVITY_100},
    [date_motor] = {DAY_31, DAY_1},
    [battery_week_motor] = {SUNDAY, BAT_PECENT_0},
    [notify_motor] = {NOTIFY_NONE, NOTIFY_DONE},
};

static void state_run_test_handler(u16 id)
{
    static s8 inc[max_motor] = {1,1,1,1,1,1};
    u8 i = 0;

    for(i = 0; i < max_motor; i++) {
        if(adapter_ctrl.motor_dst[i] == motor_test[i].start) inc[i] = 1;
        else if(adapter_ctrl.motor_dst[i] == motor_test[i].end) inc[i] = -1;
        adapter_ctrl.motor_dst[i] += inc[i];
    }
    motor_set_position(MOTOR_MASK_ALL);

	if(run_flag == 1) {
		timer_event(1000, state_run_test_handler);
	}
}

s16 state_run_test(REPORT_E cb, void *args)
{
	STATE_E *state = (STATE_E *)args;
    
    if(run_flag == 0)
    {
        run_flag = 1;
        MemCopy(motor_rdc, adapter_ctrl.motor_dst, max_motor*sizeof(u8));
        state_run_test_handler(0);
    } else {
        run_flag = 0;
        MemCopy(adapter_ctrl.motor_dst, motor_rdc, max_motor*sizeof(u8));
        *state = CLOCK;
    }
	return 0;
}

#if 0
void test_run_test(u16 id)
{
	motor_battery_week_to_position(SUNDAY);
	//state_run_test(KEY_A_B_M_LONG_PRESS, NULL);
	//timer_event(10000, test_run_test);
}
#endif
