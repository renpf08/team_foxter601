#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <macros.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

#define NOTIFY_SWING_INTERVAL   1000
bool is_adv_state = FALSE;

static void notify_swing_cb_handler(u16 id)
{
    static u8 init_state = TRUE;
    
    if(is_adv_state == TRUE) {
        if(init_state == TRUE) {
            init_state = FALSE;
            motor_notify_to_position(NOTIFY_COMMING_CALL);
        } else {
            init_state = TRUE;
            motor_notify_to_position(NOTIFY_NONE);
        }
	    timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);
    } else {
        if(init_state == FALSE) {
            motor_notify_to_position(NOTIFY_NONE);
        }
        init_state = TRUE;
    }
}
s16 state_ble_state(REPORT_E cb, void *args)
{
    //STATE_E *state = (STATE_E *)args;
    
    u8 string[13] = {'s', 't', 'a', 't', 'e', '_', 'b', 'l', 'e', '_', 's', 't', 'a'};
    print(string, 13);

    is_adv_state = ble_switch_get();
    timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);

    //*state = CLOCK;
    return 0;
}
s16 state_ble_switch(REPORT_E cb, void *args)
{
	//STATE_E *state = (STATE_E *)args;
    
	u8 string[13] = {'s', 't', 'a', 't', 'e', '_', 'b', 'l', 'e', '_', 's', 'w', 't'};
	print(string, 13);

    bool is_adv = ble_switch_get();
    if(is_adv == TRUE) {
        ble_switch_off();
    } else {
        ble_switch_on();
    }

	//*state = CLOCK;
	return 0;
}