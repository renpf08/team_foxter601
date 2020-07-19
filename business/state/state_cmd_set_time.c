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

    clock.year = time->year[1]<<8 | time->year[0];
    clock.month = time->month;
    clock.day = time->day;
    clock.week = time->week;
    clock.hour = time->hour;
    clock.minute = time->minute;
    clock.second = time->second;
    
    if(state_ble == app_pairing) {
        *state = BLE_SWITCH;
    } else {
        *state = CLOCK;
    }
    BLE_SEND_LOG((u8*)time, sizeof(cmd_set_time_t));
	motor_minute_to_position(clock.minute);
	motor_hour_to_position(clock.hour);
    motor_date_to_position(date[clock.day]);
    clock_set(&clock);

	return 0;
}
