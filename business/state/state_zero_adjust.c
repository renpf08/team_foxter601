#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

typedef struct {
    u8 rotate_speed;
    u8 backward_pos;
    u8 forward_pos;
} trig_range_t;
static void motor_trig_handler(zero_adjust_t *zero_adjust)
{
    static u8 index = 0;
    motor_queue_t queue_param = {.user = QUEUE_USER_MOTOR_TRIG};
    trig_range_t trig_range[max_motor] = {
            [hour_motor]            = {10, HOUR0_0,        HOUR0_2},
            [minute_motor]          = {10, MINUTE_0,       MINUTE_3},
            [battery_week_motor]    = {10, BAT_PECENT_0,   BAT_PECENT_40},
            [date_motor]            = {10, DAY_1,          DAY_5},
            [activity_motor]        = {25, ACTIVITY_0,     ACTIVITY_10},
            [notify_motor]          = {25, NOTIFY_NONE,    NOTIFY_EMAIL},
    };
            
    MemSet(&motor_manager.motor_run_info, 0, sizeof(motor_run_info_t));    
    motor_manager.motor_run_info.num = zero_adjust->motor_num;
    queue_param.intervel = trig_range[zero_adjust->motor_num].rotate_speed;
    queue_param.mask = (1<<zero_adjust->motor_num);
    queue_param.dest[zero_adjust->motor_num] = trig_range[zero_adjust->motor_num].forward_pos;
    queue_param.index = index++;
    motor_params_enqueue(&queue_param);
    queue_param.dest[zero_adjust->motor_num] = trig_range[zero_adjust->motor_num].backward_pos;
    queue_param.index = index++;
    motor_params_enqueue(&queue_param);
}
s16 state_zero_adjust(REPORT_E cb, void *args)
{
    motor_queue_t queue_param = {.user = QUEUE_USER_ZERO_ADJUST, .intervel = 10, .mask = MOTOR_MASK_ALL};
    static zero_adjust_t zero_adjust = {0,0};

	if(KEY_A_B_LONG_PRESS == cb) {
		zero_adjust.motor_num = minute_motor;
        MemCopy(queue_param.dest, adapter_ctrl.motor_zero, max_motor*sizeof(u8));
        motor_params_enqueue(&queue_param);
	}else if(KEY_M_SHORT_PRESS == cb) {
		/*motor switcch:hour -> minute -> activity -> date -> battery_week ->notify -> hour*/
		zero_adjust.motor_num = (zero_adjust.motor_num+1)%max_motor;
	}else if(KEY_A_SHORT_PRESS == cb) {
        zero_adjust.motor_pos = pos;
	}else if(KEY_B_SHORT_PRESS == cb){
        zero_adjust.motor_pos = neg;
	}

    if((KEY_A_B_LONG_PRESS == cb) || (KEY_M_SHORT_PRESS == cb)) {
        motor_trig_handler(&zero_adjust);
    } else if((KEY_A_SHORT_PRESS == cb) || (KEY_B_SHORT_PRESS == cb)) {
        queue_param.mask = MOTOR_MASK_ZERO_ADJUST;
        queue_param.cb_params[0] = zero_adjust.motor_num;
        queue_param.cb_params[1] = zero_adjust.motor_pos;
        motor_params_enqueue(&queue_param);
    }
	
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
