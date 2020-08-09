#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <macros.h>
#include <random.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "../business.h"
#include "state.h"

#define NOTIFY_SWING_INTERVAL   1000

typedef struct {
    u16 pair_code;
    u8 pair_bgn;
} pair_ctrl_t;

pair_ctrl_t pair_code = {0, 0};
#if USE_NO_SWING
static bool swing_en = FALSE;
#endif

static void notify_swing_cb_handler(u16 id)
{
    static bool notify_swing_start = FALSE;	

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
        //print((u8*)&"forward", 7);
    } else {
        notify_swing_start = FALSE;
        motor_notify_to_position(NOTIFY_NONE);
        //print((u8*)&"backward", 8);
    }
    
	motor_minute_to_position(clock->minute);
	motor_hour_to_position(clock->hour);
    motor_date_to_position(date[clock->day]);

    timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);
}
void pair_code_generate(void)
{
    u16 old_pair_code = 0;
    u8 hour;
    u8 minute;
    
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
        pair_code.pair_code = (hour*100+minute);
        if(pair_code.pair_code != old_pair_code) {
            break;
        }
    }

    u8 test_buf[4] = {CMD_TEST_SEND, 0, 0, 0};
    test_buf[2] = hour;
    test_buf[3] = minute;
    BLE_SEND_LOG((u8*)&test_buf, 4);
	
	motor_hour_to_position(hour);
	motor_minute_to_position(minute);
}
static s16 ble_pair(void *args)
{
    s16 res = 0;
    STATE_E *state = (STATE_E *)args;
    u8* code = cmd_get()->pair_code.code;
    u16 pairing_code = (code[1]<<8)|code[0];
    //print_str_hex((u8*)&"recv code=0x", pairing_code);

	//print((u8 *)&"recv_code:", 10);
	//print((u8 *)&pairing_code, 2);

    if(pairing_code == 0xFFFF) {
        //print((u8*)&"enter pair mode", 15);
        pair_code.pair_bgn = 1;
        pair_code_generate();
        ble_state_set(app_pairing);
    #if USE_PAIR_CODE_0000
    } else if((pairing_code == pair_code.pair_code) || 
              (pairing_code == 0x0000)) {
    #else
    } else if(pairing_code == pair_code.pair_code) {
    #endif
        BLE_SEND_LOG((u8*)&"pair matched", 12);
        pair_code.pair_bgn = 0;
        ble_state_set(app_pairing_ok);
        #if USE_PARAM_STORE
        if(pairing_code != 0x0000) 
            nvm_write_pairing_code((u16*)&pair_code.pair_code, 0);// 0x0000 is test pair code, no need to store in nvm
        #endif
        *state = CLOCK;
    } else if(pair_code.pair_bgn == 1) {
        BLE_SEND_LOG((u8*)&"pair mis-match", 14);
        pair_code_generate();
        ble_state_set(app_pairing);
        res = 1;
    } else {
        //print((u8*)&"not pair mode", 13);
        *state = CLOCK;
    }

	return res;
}
static u16 ble_change(void *args)
{
    STATE_E *state_mc = (STATE_E *)args;
    app_state state_ble = ble_state_get();
    
    if(state_ble == app_advertising) { // advertising start
        #if USE_NO_SWING
        if(swing_en == TRUE) {
            #if USE_UART_PRINT
            print((u8*)&"adv swing", 9);
            #endif
            timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);
            return 0;
        } else {
            #if USE_UART_PRINT
            print((u8*)&"adv start", 9);
            #endif
        }
        #else//USE_NO_SWING
        #if USE_UART_PRINT
        print((u8*)&"adv swing", 9);
        #endif
        timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);
        return 0;
        #endif//USE_NO_SWING
    } else if(state_ble == app_idle){ // advertising stop
        #if USE_UART_PRINT
        print((u8*)&"adv stop", 8);
        #endif
        motor_notify_to_position(NOTIFY_NONE);
    } else if(state_ble == app_connected){ // connected
        #if USE_UART_PRINT
        print((u8*)&"connect", 7);
        #endif
        #if USE_NO_SWING
        if(swing_en == FALSE) swing_en = TRUE;
        #endif
        motor_notify_to_position(NOTIFY_NONE);
    } else { // disconnected
        #if USE_UART_PRINT
        print((u8*)&"disconect", 9);
        #endif
    }
    *state_mc = CLOCK;

    return 0;
}
static u16 ble_switch(void *args)
{
    app_state cur_state = ble_state_get();
    #if USE_NO_SWING
    if(swing_en == FALSE) { // first KEY_M_LONG_PRESS
        swing_en = TRUE;
		timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);
		return 0;
		#if 0
        if(cur_state == app_advertising) {
            ble_change(NULL);
            return 0;
        }
		#endif
    }
    #endif
    if((cur_state == app_advertising) || 
        (cur_state == app_connected) || 
        (cur_state == app_pairing) || 
        (cur_state == app_pairing_ok)) {
        #if USE_UART_PRINT
        print((u8*)&"switch off", 10);
        #endif
        ble_switch_off();
    } else {
        #if USE_UART_PRINT
        print((u8*)&"switch on", 9);
        #endif
        ble_switch_on();
    }

    return 0;
}
s16 state_ble_switch(REPORT_E cb, void *args)
{
    s16 res = 0;
    
    if(cb == KEY_M_LONG_PRESS) {
        #if USE_UART_PRINT
        print((u8*)&"key press", 9);
        #endif
        res = ble_switch(args);
    } else if(cb == BLE_CHANGE) {
        #if USE_UART_PRINT
        print((u8*)&"ble change", 10);
        #endif
        res = ble_change(args);
    } else if(cb == BLE_PAIR) {
        //print((u8*)&"cmd pair", 8);
        res = ble_pair(args);
    } else if(cb == REFRESH_STEPS) {
        refresh_step();
    } else if(cb == SET_TIME) {
        sync_time();
    }

	return res;
}
