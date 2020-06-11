#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <macros.h>
#include <random.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "../business.h"
#include "state.h"

#define NOTIFY_SWING_INTERVAL   1000
#define CLOCK_REFRESH_INTERVAL  1000

typedef struct {
    u16 pair_code;
    u8 pair_bgn;
} pair_ctrl_t;

pair_ctrl_t pair_code = {0, 0};
static bool swing_en = FALSE;

static void notify_swing_cb_handler(u16 id)
{
    static bool notify_swing_start = FALSE;
	u8 day2[] = {DAY_0,
	DAY_1, DAY_2, DAY_3, DAY_4, DAY_5,
	DAY_6, DAY_7, DAY_8, DAY_9, DAY_10,
	DAY_11, DAY_12, DAY_13, DAY_14, DAY_15,
	DAY_16, DAY_17, DAY_18, DAY_19, DAY_20,
	DAY_21, DAY_22, DAY_23, DAY_24, DAY_25,
	DAY_26, DAY_27,	DAY_28,	DAY_29, DAY_30,
	DAY_31};
	
    app_state cur_state = ble_state_get();
    clock_t *clock = clock_get();
    
    if(cur_state != app_advertising) {
        if(notify_swing_start == TRUE) {
            notify_swing_start = FALSE;
            motor_notify_to_position(NOTIFY_NONE);
        }
        return;
    }

    if(notify_swing_start == FALSE) {
        notify_swing_start = TRUE;
        motor_notify_to_position(NOTIFY_COMMING_CALL);
        print((u8*)&"forward", 7);
    } else {
        notify_swing_start = FALSE;
        motor_notify_to_position(NOTIFY_NONE);
        print((u8*)&"backward", 8);
    }
    
	motor_minute_to_position(clock->minute);
	motor_hour_to_position(clock->hour);
    motor_date_to_position(day2[clock->day]);
    
    timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);
}

void pair_code_generate(void)
{
    u16 old_pair_code = 0;
	u8 hour;
    u8 minute;
    u8 bcd_hour = 0;
    u8 bcd_minute = 0;
    
    while(1) {
        old_pair_code = pair_code.pair_code;
        pair_code.pair_code = Random16();
        while((pair_code.pair_code == 0) || (pair_code.pair_code == 0xFFFF)) {
            pair_code.pair_code = Random16();
        }
        hour = (pair_code.pair_code>>8)&0x00FF;
        minute = pair_code.pair_code&0x00FF;
        while(hour >= 12) {
            hour %= 12;
        }
        while(minute >= 12) {
            minute %= 12;
        }
        minute *= 5;
        bcd_hour = hex_to_bcd(hour);
        bcd_minute = hex_to_bcd(minute);
        pair_code.pair_code = (bcd_hour<<8)|bcd_minute;
        if(pair_code.pair_code != old_pair_code) {
            break;
        }
    }
    //print_str_hex((u8*)&"pair code=0x", pair_code.pair_code);
	print((u8 *)&"pair_hour:", 10);
	print(&hour, 1);
	print((u8 *)&"pair_min:", 9);
	print(&minute, 1);
	
	motor_hour_to_position(hour);
	motor_minute_to_position(minute);
}

static s16 ble_pair(void *args)
{
    STATE_E *state = (STATE_E *)args;
    u8* code = cmd_get()->pair_code.code;
    u16 pairing_code = (code[0]<<8)|code[1];
    //print_str_hex((u8*)&"recv code=0x", pairing_code);

	print((u8 *)&"recv_code:", 10);
	print((u8 *)&pairing_code, 2);
	
    if(pairing_code == 0xFFFF) {
        print((u8*)&"enter pair mode", 15);
        pair_code.pair_bgn = 1;
        pair_code_generate();
        ble_state_set(app_pairing);
    } else if(pairing_code == pair_code.pair_code) {
        print((u8*)&"pair matched", 12);
        pair_code.pair_bgn = 0;
        ble_state_set(app_pairing_ok);
        *state = CLOCK;
    } else if(pair_code.pair_bgn == 1) {
        print((u8*)&"pair mis-match", 14);
        pair_code_generate();
        ble_state_set(app_pairing);
    } else {
        print((u8*)&"not pair mode", 13);
        *state = CLOCK;
        return 0;
    }

	return 0;
}

static u16 ble_change(void *args)
{
    STATE_E *state_mc = (STATE_E *)args;
    app_state state_ble = ble_state_get();
    
    if((state_ble == app_advertising) && (swing_en == TRUE)) { // advertising start
        print((u8*)&"adv start", 9);
        timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);
    } else if(state_ble == app_idle){ // advertising stop
        print((u8*)&"adv stop", 8);
        *state_mc = CLOCK;
        motor_notify_to_position(NOTIFY_NONE);
    } else if(state_ble == app_connected){ // connected
        print((u8*)&"connect", 7);
        if(swing_en == FALSE) swing_en = TRUE;
        motor_notify_to_position(NOTIFY_NONE);
        *state_mc = CLOCK;
    } else { // disconnected
        print((u8*)&"disconect", 9);
        *state_mc = CLOCK;
    }

    return 0;
}

static u16 ble_switch(void *args)
{
    app_state cur_state = ble_state_get();
    if(swing_en == FALSE) { // first KEY_M_LONG_PRESS
        swing_en = TRUE;
        if(cur_state == app_advertising) {
            ble_change(NULL);
            return 0;
        }
    }
    if((cur_state == app_advertising) || (cur_state == app_connected)) {
        print((u8*)&"switch off", 10);
        ble_switch_off();
    } else {
        print((u8*)&"switch on", 9);
        ble_switch_on();
    }

    return 0;
}

s16 state_ble_switch(REPORT_E cb, void *args)
{
    if(cb == KEY_M_LONG_PRESS) {
        //print((u8*)&"key press", 9);
        ble_switch(args);
    } else if(cb == BLE_CHANGE) {
        //print((u8*)&"ble change", 10);
        ble_change(args);
    } else if(cb == BLE_PAIR) {
        //print((u8*)&"cmd pair", 8);
        ble_pair(args);
    }

	return 0;
}
