#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

static u8 motor_num = 0;
static u8 motor_tmp_pos[max_motor] = {0,0,0,0,0,0};
typedef struct {
    u8 rotate_speed;
    u8 backward_pos;
    u8 forward_pos;
} trig_range_t;

static s16 motor_ctrl_swing(void *args)
{
    static u8 index = 0;
    motor_ctrl_queue_t queue_param = {.user = QUEUE_USER_MOTOR_TRIG};
    trig_range_t trig_range[max_motor] = {
            [hour_motor]            = {10, HOUR0_0,        HOUR0_2},
            [minute_motor]          = {10, MINUTE_0,       MINUTE_3},
            [battery_week_motor]    = {10, BAT_PECENT_0,   BAT_PECENT_40},
            [date_motor]            = {10, DAY_1,          DAY_5},
            [activity_motor]        = {25, ACTIVITY_0,     ACTIVITY_10},
            [notify_motor]          = {25, NOTIFY_NONE,    NOTIFY_EMAIL},
    };
            
    MemSet(&motor_manager.motor_run_info, 0, sizeof(motor_run_info_t));    
    motor_manager.motor_run_info.num = motor_num;
    queue_param.intervel = trig_range[motor_num].rotate_speed;
    queue_param.mask = (1<<motor_num);
    queue_param.dest[motor_num] = trig_range[motor_num].forward_pos;
    queue_param.index = index++;
    motor_manager.motor_run_info.target_pos = trig_range[motor_num].forward_pos;
    motor_ctrl_enqueue(&queue_param);
    queue_param.dest[motor_num] = trig_range[motor_num].backward_pos;
    queue_param.index = index++;
    motor_manager.motor_run_info.target_pos = trig_range[motor_num].backward_pos;
    motor_ctrl_enqueue(&queue_param);

    return 0;
}
s16 state_zero_adjust(REPORT_E cb, void *args)
{
    motor_ctrl_queue_t queue_param = {.user = QUEUE_USER_ZERO_ADJUST, .intervel = 10, .mask = MOTOR_MASK_ALL};
    static u8 turn = 0;
//	clock_t *clock = clock_get();
    STATE_E *state = args;

	if(KEY_A_B_LONG_PRESS == cb) {
        if(turn == 0) {
    		motor_num = minute_motor;
            MemCopy(motor_tmp_pos, adapter_ctrl.motor_dst, max_motor*sizeof(u8));
            MemCopy(queue_param.dest, adapter_ctrl.motor_zero, max_motor*sizeof(u8));
            motor_ctrl_swing(NULL);
        } else {
            *state = CLOCK;
            MemCopy(queue_param.dest, motor_tmp_pos, max_motor*sizeof(u8));
//            queue_param.dest[minute_motor] = clock->minute;
//            queue_param.dest[hour_motor] = clock->hour;
//            queue_param.dest[date_motor] = adapter_ctrl.date[clock->day];
//            queue_param.dest[battery_week_motor] = get_battery_week_pos(adapter_ctrl.current_bat_week_sta);
//            queue_param.dest[activity_motor] = adapter_ctrl.activity;
//            queue_param.dest[notify_motor] = NOTIFY_NONE;
        }
        turn = (turn==0)?1:0;
	}else if(KEY_M_SHORT_PRESS == cb) {
		motor_num = (motor_num+1)%max_motor;
        queue_param.mask = MOTOR_MASK_TRIG_SWING;
        motor_ctrl_swing(NULL);
	}else if((KEY_A_SHORT_PRESS == cb) || (KEY_B_SHORT_PRESS == cb)) {
        queue_param.mask = MOTOR_MASK_ZERO_ADJUST;
        queue_param.cb_params[0] = motor_num;
        queue_param.cb_params[1] = (cb==KEY_A_SHORT_PRESS)?pos:neg;
    }
    motor_ctrl_enqueue(&queue_param);
	
	return 0;
}

#if 0
u8 test[] = {KEY_A_B_LONG_PRESS, KEY_A_SHORT_PRESS, KEY_A_SHORT_PRESS, KEY_A_SHORT_PRESS,
			 KEY_B_SHORT_PRESS, KEY_B_SHORT_PRESS, KEY_B_SHORT_PRESS, KEY_M_SHORT_PRESS};
void zero_adjust_test(u16 id)
{
	static u8 cnt = 0;
	if(cnt < sizeof(test)) {
		state_zero_adjust(test[cnt], NULL);
		cnt++;
	}else {
		cnt = 1;
	}
	timer_event(1000, zero_adjust_test);
}
#endif
