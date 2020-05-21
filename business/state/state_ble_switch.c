#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <macros.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

s16 state_ble_switch(REPORT_E cb, void *args)
{
	//STATE_E *state = (STATE_E *)args;
    
	u8 string[13] = {'s', 't', 'a', 't', 'e', '_', 'b', 'l', 'e', '_', 's', 'w', 't'};
	print(string, 13);

    app_state cur_state = ble_state_get();
    if((cur_state == app_fast_advertising) || (cur_state == app_slow_advertising) || (cur_state == app_connected)) {
        print((u8*)&"off", 3);
        ble_switch_off();
    } else {
        print((u8*)&"on", 2);
        ble_switch_on();
    }

	//*state = CLOCK;
	return 0;
}