#include "ancs_client.h"
#include "adapter/adapter.h"

void ble_switch_on(void)
{
    if((g_app_data.state != app_connected) && 
       (g_app_data.state != app_fast_advertising) && 
       (g_app_data.state != app_slow_advertising))
    {
        AppSetState(app_fast_advertising, 0x0E);
        printf("ble switch on ok\r\n");
    }
    else
    {
        printf("ble switch on no change\r\n"); 
    }
}

void ble_switch_off(void)
{
    if(g_app_data.state == app_connected)
    {
        g_app_data.pairing_remove_button_pressed = FALSE;
        AppSetState(app_disconnecting, 0x0F);
        printf("ble switch off from connected\r\n"); 
    }
    else if((g_app_data.state != app_fast_advertising) || (g_app_data.state != app_slow_advertising))
    {
        g_app_data.pairing_remove_button_pressed = FALSE;
        AppSetState(app_idle, 0x10);
        printf("ble switch off from advertising\r\n"); 
    }
}

