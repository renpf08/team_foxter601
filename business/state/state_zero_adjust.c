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

static void motor_trig_handler(void)
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
    motor_manager.motor_run_info.num = adapter_ctrl.current_motor.num;
    queue_param.intervel = trig_range[adapter_ctrl.current_motor.num].rotate_speed;
    queue_param.mask = (1<<adapter_ctrl.current_motor.num);
    queue_param.dest[adapter_ctrl.current_motor.num] = trig_range[adapter_ctrl.current_motor.num].forward_pos;
    queue_param.index = index++;
    motor_params_enqueue(&queue_param);
    queue_param.dest[adapter_ctrl.current_motor.num] = trig_range[adapter_ctrl.current_motor.num].backward_pos;
    queue_param.index = index++;
    motor_params_enqueue(&queue_param);
}
s16 state_zero_adjust(REPORT_E cb, void *args)
{
    motor_queue_t queue_param = {.user = QUEUE_USER_ZERO_ADJUST, .intervel = 10, .mask = MOTOR_MASK_ALL};

	if(KEY_A_B_LONG_PRESS == cb) {
		adapter_ctrl.current_motor.num = minute_motor;
        MemCopy(queue_param.dest, adapter_ctrl.motor_zero, max_motor*sizeof(u8));
        motor_params_enqueue(&queue_param);
	}else if(KEY_M_SHORT_PRESS == cb) {
		/*motor switcch:hour -> minute -> activity -> date -> battery_week ->notify -> hour*/
		adapter_ctrl.current_motor.num = (adapter_ctrl.current_motor.num+1)%max_motor;
	}else if(KEY_A_SHORT_PRESS == cb) {
        adapter_ctrl.current_motor.pos = pos;
	}else if(KEY_B_SHORT_PRESS == cb){
        adapter_ctrl.current_motor.pos = neg;
	}

    if((KEY_A_B_LONG_PRESS == cb) || (KEY_M_SHORT_PRESS == cb)) {
        motor_trig_handler();
    } else if((KEY_A_SHORT_PRESS == cb) || (KEY_B_SHORT_PRESS == cb)) {
        motor_run_one_step(adapter_ctrl.current_motor.num, adapter_ctrl.current_motor.pos);
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
