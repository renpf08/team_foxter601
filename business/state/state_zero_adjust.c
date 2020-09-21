#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

typedef struct {
	u8 motor_num;
}state_zero_adjust_t;

static state_zero_adjust_t state_zero = {
	.motor_num = 0,
};

static s16 state_zero_adjust_motor_back_zero(u8 motor_num)
{
	switch(motor_num) {
		case hour_motor:
			motor_hour_to_position(HOUR0_0);
			break;
		case minute_motor:
			motor_minute_to_position(MINUTE_0);
			break;
		case activity_motor:
			motor_activity_to_position(ACTIVITY_0);
			break;
		case date_motor:
			motor_date_to_position(DAY_1);
			break;
		case battery_week_motor:
			motor_battery_week_to_position(BAT_PECENT_0);
			break;
		case notify_motor:
			motor_notify_to_position(NOTIFY_NONE);
			break;
		default:
			break;
	}
	return 0;
}

s16 state_zero_adjust(REPORT_E cb, void *args)
{
    STATE_E *state = (STATE_E *)args;
    static STATE_E pre_state = INIT;
    
    if(get_last_state() != ZERO_ADJUST) {
        pre_state = get_last_state(); // auto detect to return to CLOCK state or BLE_CHANGE state
    }
    
	if(KEY_A_B_LONG_PRESS == cb) {
        if(key_sta_ctrl.ab_long_press == 0) {
            key_sta_ctrl.ab_long_press = 1;
    		/*hour back to zero position*/
    		state_zero.motor_num = minute_motor;
    		state_zero_adjust_motor_back_zero(state_zero.motor_num);
        } else if(key_sta_ctrl.ab_long_press == 1) {
            key_sta_ctrl.ab_long_press = 0;
            *state = pre_state;
            motor_restore_position(cb);
        }
	}else if(KEY_M_SHORT_PRESS == cb) {
		/*motor switcch:hour -> minute -> activity -> date -> battery_week ->notify -> hour*/
		state_zero.motor_num++;
		if(max_motor == state_zero.motor_num) {
			state_zero.motor_num = minute_motor;
		}
		state_zero_adjust_motor_back_zero(state_zero.motor_num);
	}else if(KEY_A_SHORT_PRESS == cb) {
		/*motor run positive half step*/
		motor_run_one_step(state_zero.motor_num, pos);
	}else if(KEY_B_SHORT_PRESS == cb){
		/*motor run negtive half step*/
		motor_run_one_step(state_zero.motor_num, neg);
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
