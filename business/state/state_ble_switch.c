#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

#define CLOCK_HAND_SWING_INTERVAL   1000
bool is_adv_state = FALSE;

static void clock_hand_swing_cb_handler(u16 id)
{
    static u8 init_state = TRUE;
    
    if(is_adv_state == TRUE) {
        if(init_state == TRUE) {
            motor_notify_to_position(NOTIFY_DONE);
        } else {
            motor_notify_to_position(NOTIFY_NONE);
        }
        init_state = init_state?FALSE:TRUE;
	    get_driver()->timer->timer_start(CLOCK_HAND_SWING_INTERVAL, clock_hand_swing_cb_handler);
    } else {
        if(init_state == FALSE) {
            motor_notify_to_position(NOTIFY_NONE);
        }
        init_state = TRUE;
    }
}
static void ble_is_advertising(bool is_adv)
{
    is_adv_state = is_adv;
    get_driver()->timer->timer_start(CLOCK_HAND_SWING_INTERVAL, clock_hand_swing_cb_handler);
}
s16 state_ble_switch(REPORT_E cb, void *args)
{
	STATE_E *state = (STATE_E *)args;
    
	u8 string[13] = {'s', 't', 'a', 't', 'e', '_', 'b', 'l', 'e', '_', 's', 'w', 't'};
	print(string, 13);

    ble_is_advertising(ble_switch_get());

	*state = CLOCK;
	return 0;
}
