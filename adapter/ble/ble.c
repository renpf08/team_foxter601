#include "ancs_client.h"
#include "adapter/adapter.h"
#include <macros.h>

static app_state ble_last_state = app_init;
static adapter_callback ble_switch_cb = NULL;

s16 ble_switch_init(adapter_callback cb);

void ble_switch_on(void)
{
    if((g_app_data.state != app_connected) && 
       (g_app_data.state != app_fast_advertising) && 
       (g_app_data.state != app_slow_advertising)) {
        AppSetState(app_fast_advertising, 0x0E);
    } else {
    }
}
void ble_switch_off(void)
{
    if(g_app_data.state == app_connected) {
        g_app_data.pairing_remove_button_pressed = FALSE;
        AppSetState(app_disconnecting, 0x0F);
    } else if((g_app_data.state != app_fast_advertising) || (g_app_data.state != app_slow_advertising)) {
        g_app_data.pairing_remove_button_pressed = FALSE;
        AppSetState(app_idle, 0x10);
    }
}
void ble_state_set(app_state cur_state)
{
    if((cur_state == app_fast_advertising) || (cur_state == app_slow_advertising)) {
        if((ble_last_state == app_fast_advertising) || (ble_last_state == app_slow_advertising)) {
            print((u8*)&"adv to adv", 10);
            return;
        }
    }
    if(ble_last_state == cur_state) {
        print((u8*)&"ble no change", 13);
        return; // important!!
    }

    if((cur_state == app_fast_advertising) || (cur_state == app_slow_advertising)) {
        //print((u8*)&"ble adv", 7);
        ble_switch_cb(BLE_ADVERTISE, NULL);
    } else if(cur_state == app_idle){
        //print((u8*)&"ble idle", 8);
        ble_switch_cb(BLE_STOP_ADVERTISE, NULL);
    } else if(cur_state == app_connected){
        //print((u8*)&"ble con", 7);
        ble_switch_cb(BLE_CONNECT, NULL);
    } else {
        print((u8*)&"ble discon", 10);
        ble_switch_cb(BLE_DISCONNECT, NULL);
    }
    ble_last_state = cur_state;
}

app_state ble_state_get(void)
{
    return ble_last_state;
}

s16 ble_switch_init(adapter_callback cb)
{
	ble_switch_cb = cb;
	return 0;
}

