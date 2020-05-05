#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

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
			motor_notify_to_position(0);
			break;
		default:
			break;
	}
	return 0;
}

s16 state_zero_adjust(REPORT_E cb, void *args)
{
	if(KEY_A_B_LONG_PRESS == cb) {
		/*hour back to zero position*/
		state_zero.motor_num = 0;
		state_zero_adjust_motor_back_zero(state_zero.motor_num);
	}else if(KEY_M_SHORT_PRESS == cb) {
		/*motor switcch:hour -> minute -> activity -> date -> battery_week ->notify -> hour*/
		state_zero.motor_num++;
		if(6 == state_zero.motor_num) {
			state_zero.motor_num = 0;
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