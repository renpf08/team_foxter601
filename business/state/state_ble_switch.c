#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <macros.h>
#include <random.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "../business.h"
#include "state.h"

#define NOTIFY_SWING_INTERVAL   1000

#define SWING_MOTOR_NAME activity_motor
#define SWING_MOTOR_MASK MOTOR_MASK_ACTIVITY

typedef struct {
    u16 pair_code;
    u8 pair_bgn;
} pair_ctrl_t;

pair_ctrl_t pair_code = {0, 0};
static u8 swing_ongoing = 0;

static void notify_swing_cb_handler(u16 id)
{
    clock_t *clock = clock_get();
    static u8 minute = 0;
    static u8 swing_state = NOTIFY_NONE;
    motor_ctrl_queue_t queue_param = {.user = QUEUE_USER_BLE_SWING, .intervel = 40, .mask = SWING_MOTOR_MASK};

    swing_ongoing = 0;
    if(ble_state_get() != app_advertising) {
        queue_param.dest[SWING_MOTOR_NAME] = NOTIFY_NONE;
        motor_ctrl_enqueue(&queue_param);
        return;
    }
    swing_ongoing = 1;

    swing_state = (swing_state == NOTIFY_NONE)?NOTIFY_COMMING_CALL:NOTIFY_NONE;
    queue_param.dest[SWING_MOTOR_NAME] = swing_state;
    motor_ctrl_enqueue(&queue_param);
    if(minute != clock->minute) {
        minute = clock->minute;
        motor_ctrl_refresh(clock, (MOTOR_MASK_HOUR|MOTOR_MASK_MINUTE|MOTOR_MASK_DATE|MOTOR_MASK_BAT_WEEK), QUEUE_USER_BLE_SWING, NULL);
    }
    
    timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);
}
void pair_code_generate(void)
{
    u16 old_pair_code = 0;
    u8 hour;
    u8 minute;
    motor_ctrl_queue_t queue_param = {.user = QUEUE_USER_PAIR_CODE, .intervel = 10, .mask = (MOTOR_MASK_HOUR|MOTOR_MASK_MINUTE|MOTOR_MASK_NOTIFY)};
    
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
	
    queue_param.dest[minute_motor] = minute;
    queue_param.dest[hour_motor] = hour;
    motor_ctrl_enqueue(&queue_param);
}
static s16 ble_pair(void *args)
{
    s16 res = 0;
    STATE_E *state = (STATE_E *)args;
    u8* code = cmd_get()->pair_code.code;
    u16 pairing_code = (code[1]<<8)|code[0];

    if(pairing_code == 0xFFFF) {
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
        *state = CLOCK;
    }

	return res;
}
static u16 ble_change(void *args)
{
    STATE_E *state_mc = (STATE_E *)args;
    app_state state_ble = ble_state_get();
    motor_ctrl_queue_t queue_param = {.user = QUEUE_USER_BLE_CHANGE, .intervel = 40, .mask = SWING_MOTOR_MASK};
    
    queue_param.dest[SWING_MOTOR_NAME] = NOTIFY_NONE;
    if(state_ble == app_advertising) { // advertising start
        timer_event(1, notify_swing_cb_handler);
        return 0;
    } else if((state_ble == app_idle) || (state_ble == app_connected)){ // advertising stop & connected
        if(swing_ongoing == 1) {
            swing_ongoing = 0;
            motor_ctrl_enqueue(&queue_param);
        }
    } else { // disconnected
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
        ble_switch_off();
    } else {
        ble_switch_on();
    }

    return 0;
}
s16 state_ble_switch(REPORT_E cb, void *args)
{
    s16 res = 0;
    
    if(cb == KEY_M_LONG_PRESS) {
        res = ble_switch(args);
    } else if(cb == BLE_CHANGE) {
        res = ble_change(args);
    } else if(cb == BLE_PAIR) {
        res = ble_pair(args);
    } else if(cb == REFRESH_STEPS) {
        refresh_step();
    } else if(cb == SET_TIME) {
        sync_time();
    }

	return res;
}
