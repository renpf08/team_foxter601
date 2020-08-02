#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

s16 state_zero_adjust(REPORT_E cb, void *args)
{	
//    u8 msg[3] = {CMD_TEST_SEND, 0x02, 0};
//    if(adapter_ctrl.zero_adjust_mode.press == 1) {
//        msg[2] = 0x01;
//        BLE_SEND_LOG(msg, 3);
//        return 1;
//    } else if(adapter_ctrl.zero_adjust_mode.run == 1) {
//        msg[2] = 0x02;
//        BLE_SEND_LOG(msg, 3);
//        return 1;
//    }
//    adapter_ctrl.zero_adjust_mode.press = 1;
	if(KEY_A_B_LONG_PRESS == cb) {
		/*hour back to zero position*/
		adapter_ctrl.current_motor_num = minute_motor;
        MemCopy(adapter_ctrl.motor_dst, adapter_ctrl.motor_zero, max_motor*sizeof(u8));
        motor_run.timer_interval = 20;
        motor_set_position(MOTOR_MASK_ALL|MOTOR_MASK_TRIG);
	}else if(KEY_M_SHORT_PRESS == cb) {
		/*motor switcch:hour -> minute -> activity -> date -> battery_week ->notify -> hour*/
		adapter_ctrl.current_motor_num++;
		if(max_motor == adapter_ctrl.current_motor_num) {
			adapter_ctrl.current_motor_num = minute_motor;
		}
        motor_run.timer_interval = 20;
        motor_set_position(MOTOR_MASK_TRIG);
		//state_zero_adjust_motor_back_zero(motor_num);
	}else if(KEY_A_SHORT_PRESS == cb) {
		/*motor run positive half step*/
		motor_run_one_step(adapter_ctrl.current_motor_num, pos);
	}else if(KEY_B_SHORT_PRESS == cb){
		/*motor run negtive half step*/
		motor_run_one_step(adapter_ctrl.current_motor_num, neg);
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
