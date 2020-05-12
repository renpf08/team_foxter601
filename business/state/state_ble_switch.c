#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

s16 state_ble_switch(REPORT_E cb, void *args)
{
	u8 string[13] = {'s', 't', 'a', 't', 'e', '_', 'b', 'l', 'e', '_', 's', 'w', 't'};
	print(string, 13);

	
	return 0;
}

void ble_switch_test(u16 id)
{
	static u8 clock_swing = 0;
	if(0 == clock_swing) {
		clock_swing = 1;
		motor_notify_to_position(NOTIFY_COMMING_CALL);
	}else if(1 == clock_swing) {
		clock_swing = 0;
		motor_notify_to_position(NOTIFY_NONE);
	}
	timer_event(700, ble_switch_test);
}
