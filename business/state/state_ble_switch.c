#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <macros.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

#define NOTIFY_SWING_INTERVAL   1000
typedef struct {
    volatile app_state last_state;
    volatile bool notify_swing_start;
    volatile bool notify_swing_en;
}ble_sta_ctrl_t;
ble_sta_ctrl_t ble_sta_ctrl = {
    .last_state = app_init,
    .notify_swing_start = false,
    .notify_swing_en = false,
};

typedef u8 (* ble_state_handler)(app_state cur_state);
typedef struct  {
	app_state state;
	ble_state_handler handler;
}ble_state_t;
static u8 ble_state_unuse(app_state cur_state);
static u8 ble_state_dvertising(app_state cur_state);
static u8 ble_state_connected(app_state cur_state);
static u8 ble_state_disconnecting(app_state cur_state);
static const ble_state_t ble_state_list[] =
{
    {app_init, ble_state_unuse},
    {app_fast_advertising, ble_state_dvertising},
    {app_slow_advertising, ble_state_dvertising},
    {app_connected, ble_state_connected},
    {app_disconnecting, ble_state_disconnecting},
    {app_idle, ble_state_unuse},
    {app_state_unknown, ble_state_unuse}
};

static void notify_swing_cb_handler(u16 id)
{
    if(ble_sta_ctrl.notify_swing_en == FALSE) {
        return;
    }
    print((u8*)&"swing", 5);
    
    if(ble_sta_ctrl.notify_swing_start == FALSE) {
        ble_sta_ctrl.notify_swing_start = TRUE;
        motor_notify_to_position(NOTIFY_COMMING_CALL);
    } else {
        ble_sta_ctrl.notify_swing_start = FALSE;
        motor_notify_to_position(NOTIFY_NONE);
    }
    timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);
}
static void notify_swing_back(app_state cur_state)
{
    if(ble_sta_ctrl.notify_swing_start == TRUE) {
        motor_notify_to_position(NOTIFY_NONE);
    }
    ble_sta_ctrl.notify_swing_en = FALSE;
    ble_sta_ctrl.notify_swing_start = FALSE;
    ble_sta_ctrl.last_state = cur_state;
}
static u8 ble_state_unuse(app_state cur_state)
{
    print((u8*)&"unuse", 5);
    notify_swing_back(cur_state);
    
    return 0;
}
static u8 ble_state_dvertising(app_state cur_state)
{
    print((u8*)&"dvertising", 10);
    ble_sta_ctrl.last_state = cur_state;
    ble_sta_ctrl.notify_swing_en = TRUE;
    timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);
    
    return 0;
}
static u8 ble_state_connected(app_state cur_state)
{
    print((u8*)&"connected", 9);
    notify_swing_back(cur_state);
    
    return 0;
}
static u8 ble_state_disconnecting(app_state cur_state)
{
    print((u8*)&"disconnecting", 13);
    notify_swing_back(cur_state);
    
    return 0;
}
s16 state_ble_state(REPORT_E cb, void *args)
{
    //STATE_E *state = (STATE_E *)args;
	u8 i = 0;
    u8 string[13] = {'s', 't', 'a', 't', 'e', '_', 'b', 'l', 'e', '_', 's', 't', 'a'};
    print(string, 13);

    app_state ble_state = ble_state_get();
    while(ble_state_list[i].state != app_state_unknown)
    {
        if(ble_state_list[i].state == ble_state)
        {
            ble_state_list[i].handler(ble_state);
            break;
        }
        i++;
    }

    //*state = CLOCK;
    return 0;
}
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