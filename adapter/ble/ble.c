#include "ancs_client.h"
#include "adapter/adapter.h"
#include <macros.h>

static bool is_adv_state = FALSE;
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
void ble_switch_set(bool cur_state)
{
    if(is_adv_state == cur_state) {
        return; // important!!
    }
    is_adv_state = cur_state;
    ble_switch_cb(BLE_CHANGE, NULL);
}

bool ble_switch_get(void)
{
    return is_adv_state;
}

s16 ble_switch_init(adapter_callback cb)
{
	ble_switch_cb = cb;
	return 0;
}

