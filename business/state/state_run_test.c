#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>
#include <random.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

u8 run_enable = 0;
u8 motor_rdc[max_motor] = {0,0,0,0,0,0};

static void motor_run_state_calc(u8 motor_num, u8 min_val, u8 max_val)
{
    if(motor_run.random_val[motor_num] <= min_val) {
        motor_run.motor_dirc[motor_num] = pos;
        motor_run.calc_dirc[motor_num] = 1;
        motor_run.random_val[motor_num] = Random16()%max_val;
        if(motor_run.random_val[motor_num] > (max_val-motor_run.motor_offset[motor_num])) {
            motor_run.random_val[motor_num] = (max_val-motor_run.motor_offset[motor_num]);
        }
    } else if (motor_run.random_val[motor_num] >= max_val) {
        motor_run.motor_dirc[motor_num] = neg;
        motor_run.calc_dirc[motor_num] = -1;
        motor_run.random_val[motor_num] = Random16()%max_val;
        if(motor_run.random_val[motor_num] > motor_run.motor_offset[motor_num]) {
            motor_run.random_val[motor_num] = motor_run.motor_offset[motor_num];
        }
    }
    motor_run.random_val[motor_num] += motor_run.calc_dirc[motor_num];
    motor_run.motor_offset[motor_num] += motor_run.calc_dirc[motor_num];
}
static void state_run_test_handler(u16 id)
{
    motor_run_state_calc(minute_motor, MINUTE_0, MINUTE_60);
    motor_run_state_calc(hour_motor, MINUTE_0, MINUTE_60);
    motor_run_state_calc(activity_motor, ACTIVITY_0, ACTIVITY_100);
    motor_run_state_calc(date_motor, DAY_31, DAY_1);
    motor_run_state_calc(battery_week_motor, SUNDAY, BAT_PECENT_0);
    motor_run_state_calc(notify_motor, NOTIFY_NONE, NOTIFY_DONE);
    motor_run_one_step_handler(0);
	if(run_enable == 1) {
		timer_event(40, state_run_test_handler);
	}
}

static void motor_roll_back_handler(u16 id)
{
    if(motor_check_idle() == 0) {
        state_run_test_handler(0);
        return;
    }
    timer_event(100, motor_roll_back_handler);
}

s16 state_run_test(REPORT_E cb, void *args)
{
	STATE_E *state = (STATE_E *)args;
    
    if(run_enable == 0)
    {
        run_enable = 1;
        MemCopy(motor_rdc, adapter_ctrl.motor_dst, max_motor*sizeof(u8));
        MemCopy(adapter_ctrl.motor_dst, adapter_ctrl.motor_zero, max_motor*sizeof(u8));
        motor_set_position(MOTOR_MASK_ALL);
        motor_roll_back_handler(0);
    } else {
        run_enable = 0;
        MemCopy(adapter_ctrl.motor_dst, motor_rdc, max_motor*sizeof(u8));
        motor_set_position(MOTOR_MASK_ALL);
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
