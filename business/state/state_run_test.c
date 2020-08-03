#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>
#include <random.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

STATE_E *state;
u8 run_enable = 0;
u8 run_quit[max_motor] = {0,0,0,0,0,0};
u8 motor_rdc[max_motor] = {0,0,0,0,0,0};

static void motor_reset_handler(u16 id);
static void motor_recover_handler(u16 id);
static void motor_run_state_calc(u8 motor_num);
static void state_run_test_handler(u16 id);

static void motor_reset_handler(u16 id)
{
    if(motor_check_idle() == 0) {
		timer_event(10, state_run_test_handler);
        return;
    }
    timer_event(100, motor_reset_handler);
}

static void motor_recover_handler(u16 id)
{
    if(motor_check_idle() == 0) {
        motor_run.test_mode = 0;
        *state = CLOCK;
        return;
    }
    timer_event(100, motor_reset_handler);
}

static void motor_run_state_calc(u8 motor_num)
{
    if(motor_run.step_cnt[motor_num] <= motor_run.motor_range[motor_num].min) {
        if(run_enable == 0) { // back to zero position
            run_quit[motor_num] = 1;
            motor_run.motor_flag[motor_num] = 0;
            motor_run.motor_dirc[motor_num] = none;
            return;
        }
        if((motor_num == battery_week_motor) || (motor_num == date_motor)) {
            motor_run.motor_dirc[motor_num] = neg;
        } else {
            motor_run.motor_dirc[motor_num] = pos;
        }
        motor_run.calc_dirc[motor_num] = 1;
    } else if(motor_run.step_cnt[motor_num] >= motor_run.motor_range[motor_num].max) {
        if((motor_num == battery_week_motor) || (motor_num == date_motor)) {
            motor_run.motor_dirc[motor_num] = pos;
        } else {
            motor_run.motor_dirc[motor_num] = neg;
        }
        motor_run.calc_dirc[motor_num] = -1;
    }
    if(motor_run.skip_cnt[motor_num] == 0) {
        motor_run.step_cnt[motor_num]+= motor_run.calc_dirc[motor_num];
    }
}
static void state_run_test_handler(u16 id)
{
    u8 i = activity_motor;
    
    MemSet(motor_run.motor_flag, 1, max_motor*sizeof(u8));
    for(i = 0; i < max_motor; i++) {
        if(motor_run.skip_cnt[i] < (motor_run.skip_total[i])*2) {
            motor_run.skip_cnt[i]++;
            motor_run.motor_flag[i] = 0;
        } else if(motor_run.skip_cnt[i] != 0) {
            motor_run.skip_cnt[i] = 0;
        }
    }
    for(i = 0; i < max_motor; i++) {
        motor_run_state_calc(i);
    }
    for(i = 0; i < max_motor; i++) {
        if(run_quit[i] != 1) {
            break;
        }
    }
    if(i == max_motor) {
        MemCopy(adapter_ctrl.motor_dst, motor_rdc, max_motor*sizeof(u8));
        motor_set_position(MOTOR_MASK_ALL);
        timer_event(1, motor_recover_handler);
        return; // all motor has back to zero position
    }
    motor_run.timer_interval = 5;
    timer_event(1, motor_check_run);
	timer_event(40, state_run_test_handler);
}

s16 state_run_test(REPORT_E cb, void *args)
{
    
    if(run_enable == 0)
    {
        run_enable = 1;
        motor_run.test_mode = 1;
        MemSet(run_quit, 0, max_motor*sizeof(u8));
        MemCopy(motor_rdc, adapter_ctrl.motor_dst, max_motor*sizeof(u8));
        MemCopy(adapter_ctrl.motor_dst, adapter_ctrl.motor_zero, max_motor*sizeof(u8));
        motor_set_position(MOTOR_MASK_ALL);
        timer_event(1, motor_reset_handler);
    } else {
        run_enable = 0;
        state = args;
    }
	return 0;
}

