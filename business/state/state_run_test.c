#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>
#include <random.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

u8 run_enable = 0;
u8 motor_rdc[max_motor] = {0,0,0,0,0,0};

static void motor_run_state_calc(u8 motor_num)
{
    if(motor_run.random_val[motor_num] <= motor_run.motor_range[motor_num].min) {
        motor_run.motor_dirc[motor_num] = pos;
        motor_run.calc_dirc[motor_num] = 1;
        motor_run.random_val[motor_num] = Random16()%motor_run.motor_range[motor_num].max;
        if(motor_run.random_val[motor_num] > (motor_run.motor_range[motor_num].max-motor_run.motor_offset[motor_num])) {
            motor_run.random_val[motor_num] = (motor_run.motor_range[motor_num].max-motor_run.motor_offset[motor_num]);
        }
    } else if (motor_run.random_val[motor_num] >= motor_run.motor_range[motor_num].max) {
        motor_run.motor_dirc[motor_num] = neg;
        motor_run.calc_dirc[motor_num] = -1;
        motor_run.random_val[motor_num] = Random16()%motor_run.motor_range[motor_num].max;
        if(motor_run.random_val[motor_num] > motor_run.motor_offset[motor_num]) {
            motor_run.random_val[motor_num] = motor_run.motor_offset[motor_num];
        }
    }
    motor_run.random_val[motor_num] += motor_run.calc_dirc[motor_num];
    motor_run.motor_offset[motor_num] += motor_run.calc_dirc[motor_num];
}
static void state_run_test_handler(u16 id)
{
    u8 i = 0;
    
    for(i = 0; i < max_motor; i++) {
        motor_run_state_calc(i);
    }
    MemSet(motor_run.motor_flag, 1, max_motor*sizeof(u8));
    motor_run.timer_interval = 5;
    timer_event(1, motor_check_run);
	if(run_enable == 1) {
		timer_event(40, state_run_test_handler);
	}
}

static void motor_roll_back_handler(u16 id)
{
    if(motor_check_idle() == 0) {
		timer_event(1, state_run_test_handler);
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
