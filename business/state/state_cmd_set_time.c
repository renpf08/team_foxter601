#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

s16 state_set_date_time(REPORT_E cb, void *args)
{
	STATE_E *state = (STATE_E *)args;
	cmd_set_time_t *time = (cmd_set_time_t *)&cmd_get()->set_time;
    clock_t clock;
    app_state state_ble = ble_state_get();

    clock.year = bcd_to_hex(time->year[0])*100 + bcd_to_hex(time->year[1]);
    clock.month = bcd_to_hex(time->month);
    clock.day = bcd_to_hex(time->day);
    clock.hour = bcd_to_hex(time->hour);
    clock.minute = bcd_to_hex(time->minute);
    clock.second = bcd_to_hex(time->second);
    clock.week = bcd_to_hex(time->week);
    
    if(state_ble == app_pairing) {
        *state = BLE_SWITCH;
    } else {
        *state = CLOCK;
    }
    clock_set(&clock);
    print_date_time((u8*)&"set time=", &clock);

	return 0;
}

