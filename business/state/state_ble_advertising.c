#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <macros.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

#define NOTIFY_SWING_INTERVAL   1000

static bool notify_swing_start = FALSE;

static void notify_swing_cb_handler(u16 id)
{
    app_state cur_state = ble_state_get();
    if((cur_state != app_fast_advertising) && (cur_state != app_slow_advertising)) {
        if(notify_swing_start == TRUE) {
            notify_swing_start = FALSE;
        }
        return;
    }
    print((u8*)&"swing", 5);
    
    if(notify_swing_start == FALSE) {
        notify_swing_start = TRUE;
        motor_notify_to_position(NOTIFY_COMMING_CALL);
    } else {
        notify_swing_start = FALSE;
        motor_notify_to_position(NOTIFY_NONE);
    }
    timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);
}
s16 state_ble_advertise(REPORT_E cb, void *args)
{
	u8 string[13] = {'s', 't', 'a', 't', 'e', '_', 'b', 'l', 'e', '_', 'a', 'd', 'v'};
	print(string, 13);

    timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);

	return 0;
}
