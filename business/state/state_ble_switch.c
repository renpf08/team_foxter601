#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <macros.h>
#include <random.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "../business.h"
#include "state.h"

#define NOTIFY_SWING_INTERVAL   1000

static u16 pair_code = 0;
static s16 discon_vib_timer_id = TIMER_INVALID;
static s16 swing_timer_id = TIMER_INVALID;

static void notify_swing_cb_handler(u16 id)
{
    static bool notify_swing_start = FALSE;	
    app_state cur_state = ble_state_get();
    clock_t *clock = clock_get();

    swing_timer_id = TIMER_INVALID;
    if(cur_state != app_advertising) {
        notify_swing_start = FALSE;
        #if USE_ACTIVITY_NOTIFY
        motor_activity_to_position(NOTIFY_NONE);
        #else
        motor_notify_to_position(NOTIFY_NONE);
        #endif
        return;
    }
        
    if((key_sta_ctrl.ab_long_press == 0) && (key_sta_ctrl.compass_state == 0)) {
        if(notify_swing_start == FALSE) {
            notify_swing_start = TRUE;
            #if USE_ACTIVITY_NOTIFY
            motor_activity_to_position(NOTIFY_COMMING_CALL);
            #else
            motor_notify_to_position(NOTIFY_COMMING_CALL);
            #endif
            //print((u8*)&"forward", 7);
        } else {
            notify_swing_start = FALSE;
            #if USE_ACTIVITY_NOTIFY
            motor_activity_to_position(NOTIFY_NONE);
            #else
            motor_notify_to_position(NOTIFY_NONE);
            #endif
            //print((u8*)&"backward", 8);
        }
        
    	motor_minute_to_position(clock->minute);
    	motor_hour_to_position(clock->hour);
        motor_date_to_position(date[clock->day]);
        motor_week_to_position(clock->week);
//        #if USE_WEEK_FORCE_UPDATE
//        motor_battery_week_to_position(clock->week);
//        #endif
    } else if(notify_swing_start == TRUE) {
        notify_swing_start = FALSE;
        #if USE_ACTIVITY_NOTIFY
        motor_activity_to_position(NOTIFY_NONE);
        #else
        motor_notify_to_position(NOTIFY_NONE);
        #endif
    }

    swing_timer_id = timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);
}
void pair_code_generate(void)
{
    u16 old_pair_code = 0;
    u8 hour;
    u8 minute;
    
    while(1) {
        old_pair_code = pair_code;
        pair_code = Random16();
        while((pair_code == 0) || (pair_code == 0xFFFF)) {
            pair_code = Random16();
        }
        hour = (pair_code>>8)&0x00FF;
        minute = pair_code&0x00FF;
        while(hour >= 12) {
            hour %= 12;
        }
        while(minute >= 12) {
            minute %= 12;
        }
        minute *= 5;
        pair_code = (hour*100+minute);
        if(pair_code != old_pair_code) {
            break;
        }
    }

    LOG_SEND_PAIR_CODE_VARIABLE_DEF(log_send, log_send_pair_code_t, LOG_CMD_SEND, LOG_SEND_PAIR_CODE);
    LOG_SEND_PAIR_CODE_VALUE_SET(log_send.hour_code, hour);
    LOG_SEND_PAIR_CODE_VALUE_SET(log_send.minute_code, minute);
    LOG_SEND_PAIR_CODE_VALUE_SEND(log_send.head);
	
	motor_hour_to_position(hour*5);
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
        key_sta_ctrl.pair_code_disp = 1;
        pair_code_generate();
        ble_state_set(app_pairing);
    #if USE_PAIR_CODE_0000
    } else if((pairing_code == pair_code) || 
              (pairing_code == 0x0000)) {
    #else
    } else if(pairing_code == pair_code) {
    #endif
        //BLE_SEND_LOG((u8*)&"pair matched", 12);
        key_sta_ctrl.pair_code_disp = 0;
        ble_state_set(app_pairing_ok);
        #if USE_PARAM_STORE
        if(pairing_code != 0x0000) 
            nvm_write_pairing_code((u16*)&pair_code, 0);// 0x0000 is test pair code, no need to store in nvm
        #endif
        *state = CLOCK;
    } else if(key_sta_ctrl.pair_code_disp == 1) {
        //BLE_SEND_LOG((u8*)&"pair mis-match", 14);
        pair_code_generate();
        ble_state_set(app_pairing);
        res = 1;
    } else {
        //print((u8*)&"not pair mode", 13);
        *state = CLOCK;
    }

	return res;
}
static void disconect_vib_handler(u16 id)
{
    discon_vib_timer_id = TIMER_INVALID;
    #if USE_UART_PRINT
    trace((u8*)&"vibration", 9);
    #endif
    vib_stop();
    vib_run(5, 0x02);
    key_sta_ctrl.pair_code_disp = 0;
}
static u16 ble_change(void *args)
{
    STATE_E *state_mc = (STATE_E *)args;
    app_state state_ble = ble_state_get();
    static app_state last_state_ble = app_idle;
    
    timer_remove(discon_vib_timer_id);
    discon_vib_timer_id = TIMER_INVALID;
    //if((last_state_ble == app_connected) && ((state_ble != app_connected) || (state_ble == app_idle))) {
    if((last_state_ble == app_connected) && (state_ble != app_connected)) {
        discon_vib_timer_id = timer_event(NOTIFY_SWING_INTERVAL*3, disconect_vib_handler);
    } else if(state_ble == app_connected) {
        vib_stop();
    }
    last_state_ble = state_ble;    
    if(state_ble == app_advertising) { // advertising start
        #if USE_UART_PRINT
        trace((u8*)&"adv swing", 9);
        #endif
        timer_remove(swing_timer_id);
        swing_timer_id = TIMER_INVALID;
        swing_timer_id =timer_event(NOTIFY_SWING_INTERVAL*3, notify_swing_cb_handler);
        return 0;
    } else if(state_ble == app_idle){ // advertising stop
        #if USE_UART_PRINT
        trace((u8*)&"adv stop", 8);
        #endif
    } else if(state_ble == app_connected){ // connected
        #if USE_UART_PRINT
        trace((u8*)&"connect", 7);
        #endif
    } else { // disconnected
        #if USE_UART_PRINT
        trace((u8*)&"disconect", 9);
        #endif
    }
    *state_mc = CLOCK;

    return 0;
}
static u16 ble_switch(void *args)
{
    app_state cur_state = ble_state_get();
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
    } /*else if(cb == REFRESH_STEPS) {
        refresh_step();
    }*/ else if(cb == SET_TIME) {
        sync_time();
    } else if((cb == CHARGE_SWING) || (cb == CHARGE_STOP)) {
        charge_check(cb);
    }

	return res;
}
