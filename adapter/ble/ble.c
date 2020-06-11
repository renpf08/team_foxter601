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
        AppSetState(app_fast_advertising);
    } else {
    }
}
void ble_switch_off(void)
{
    if(g_app_data.state == app_connected) {
        g_app_data.pairing_remove_button_pressed = FALSE;
        AppSetState(app_disconnecting);
    } else if((g_app_data.state != app_fast_advertising) || (g_app_data.state != app_slow_advertising)) {
        g_app_data.pairing_remove_button_pressed = FALSE;
        AppSetState(app_idle);
    }
}
void ble_state_set(app_state cur_state)
{
    if(cur_state == app_pairing) {
        ble_last_state = cur_state;
        return;
    } else if (cur_state == app_pairing_ok) {
        ble_last_state = app_connected;
        return;
    }
    if((cur_state == app_fast_advertising) || (cur_state == app_slow_advertising)) {
        cur_state = app_advertising;
    }
    if(ble_last_state == cur_state) {
        //print((u8*)&"ble no change", 13);
        return;
    }
    ble_last_state = cur_state;
    ble_switch_cb(BLE_CHANGE, NULL);
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
