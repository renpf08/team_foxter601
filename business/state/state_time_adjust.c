#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"




s16 state_time_adjust(REPORT_E cb, void *args)
{
	print((u8 *)&"time_adjust", 11);

	if(KEY_A_B_LONG_PRESS == cb) {
		/*minute back to zero position*/
		state_zero.motor_num = minute_motor;
	}else if(KEY_M_SHORT_PRESS == cb) {
		/*motor switcch:minute -> hour ->  date -> battery_week -> minute*/
		state_zero.motor_num++;
		if(max_motor == state_zero.motor_num) {
			state_zero.motor_num = minute_motor;
		}
	}else if(KEY_A_SHORT_PRESS == cb) {
		/*motor run positive half step*/
	
	}else if(KEY_B_SHORT_PRESS == cb){
		/*motor run negtive half step*/
	
	}

	return 0;
}
