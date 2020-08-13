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

static void motor_run_state_calc(u8 motor_num);
static void state_run_test_handler(u16 id);

static s16 motor_recover(void *args)
{
    motor_manager.run_test_mode = 0;
    run_enable = 0;
    *state = CLOCK;
    return 0;
}
static s16 motor_run(void *args)
{
    state_run_test_handler(0);
    return 0;
}
static s16 motor_check(void *args)
{
    state_run_test_handler(0);
    return 0;
}
static void motor_run_state_calc(u8 motor_num)
{
    if(motor_manager.step_cnt[motor_num] <= motor_manager.status[motor_num].run_range.min) {
        if(run_enable == 2) { // back to zero position
            run_quit[motor_num] = 1;
            motor_manager.status[motor_num].run_flag = 0;
            motor_manager.status[motor_num].run_direc = none;
            return;
        }
        if((motor_num == battery_week_motor) || (motor_num == date_motor)) {
            motor_manager.status[motor_num].run_direc = neg;
        } else {
            motor_manager.status[motor_num].run_direc = pos;
        }
        motor_manager.calc_dirc[motor_num] = 1;
    } else if(motor_manager.step_cnt[motor_num] >= motor_manager.status[motor_num].run_range.max) {
        if((run_enable == 2) && ((motor_num == minute_motor) ||(motor_num == hour_motor))) { // back to zero position
            run_quit[motor_num] = 1;
            motor_manager.status[motor_num].run_flag = 0;
            motor_manager.status[motor_num].run_direc = none;
            return;
        }
        if((motor_num == battery_week_motor) || (motor_num == date_motor)) {
            motor_manager.status[motor_num].run_direc = pos;
        } else {
            motor_manager.status[motor_num].run_direc = neg;
        }
        motor_manager.calc_dirc[motor_num] = -1;
    }
    motor_manager.status[motor_num].run_flag = 1;
    motor_manager.step_cnt[motor_num]+= motor_manager.calc_dirc[motor_num];
}
static void state_run_test_handler(u16 id)
{
    u8 i = 0;
    motor_queue_t queue_param = {.user = QUEUE_USER_RUN_HANDLER, .intervel = 10, .mask = MOTOR_MASK_ALL, .cb = motor_recover};

    if(motor_check_idle() == 0) {
        for(i = 0; i < max_motor; i++) {
//            if(motor_manager.skip_cnt[i] < (motor_manager.skip_total[i])*2) {
//                motor_manager.skip_cnt[i]++;
//            } else {
//                motor_manager.skip_cnt[i] = 0;
                motor_run_state_calc(i);
            //}
        }
        for(i = 0; i < max_motor; i++) {
            if(run_quit[i] != 1) {
                break;
            }
        }
        if(i == max_motor) {
            MemCopy(queue_param.dest, motor_rdc, max_motor*sizeof(u8));
            motor_params_enqueue(&queue_param);
            return; // all motor has back to zero position
        } else {
            queue_param.intervel = 5;
            queue_param.mask = MOTOR_MASK_RUN_TEST;
            queue_param.cb = motor_check;
            motor_params_enqueue(&queue_param);
        }
    }
}
s16 state_run_test(REPORT_E cb, void *args)
{
    state = args;
    motor_queue_t queue_param = {.user = QUEUE_USER_RUN_TEST, .intervel = 10, .mask = MOTOR_MASK_ALL, .cb = motor_run};

    if(run_enable == 0) {
        run_enable = 1;
        motor_manager.run_test_mode = 1;
        MemSet(run_quit, 0, max_motor*sizeof(u8));
        MemCopy(motor_rdc, adapter_ctrl.motor_dst, max_motor*sizeof(u8));
        MemCopy(queue_param.dest, adapter_ctrl.motor_zero, max_motor*sizeof(u8));
        motor_params_enqueue(&queue_param);
    } else {
        run_enable = 2;
    }
	return 0;
}

