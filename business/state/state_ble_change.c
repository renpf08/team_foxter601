#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <macros.h>
#include <random.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "../business.h"
#include "state.h"

#define PAIRING_PROTECT_INTERVAL    100
#define NOTIFY_SWING_INTERVAL   1000
bool notify_swing_start = FALSE;
STATE_E *state = NULL;
STATE_E state_later = STATE_MAX;
pair_code_t pair_code = {0, 0, 0, 0};

static void notify_swing_cb_handler(u16 id)
{
    app_state cur_state = ble_state_get();
    if((cur_state != app_fast_advertising) && (cur_state != app_slow_advertising)) {
        if(notify_swing_start == TRUE) {
            notify_swing_start = FALSE;
        }
        return;
    }

    print((u8*)&"swing", 5);

    if(notify_swing_start == FALSE) {
        notify_swing_start = TRUE;
        motor_notify_to_position(NOTIFY_COMMING_CALL);
    } else {
        notify_swing_start = FALSE;
        motor_notify_to_position(NOTIFY_NONE);
    }
    timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);
}
static void pairing_protect_cb_handler(u16 id)
{
    *state = state_later;
}
void pair_code_generate(void)
{
    u16 old_pair_code = 0;
    u8 bcd_hour = 0;
    u8 bcd_minute = 0;
    
    while(1) {
        old_pair_code = pair_code.pair_code;
        pair_code.pair_code = Random16();
        while((pair_code.pair_code == 0) || (pair_code.pair_code == 0xFFFF)) {
            pair_code.pair_code = Random16();
        }
        pair_code.hour = (pair_code.pair_code>>8)&0x00FF;
        pair_code.minute = pair_code.pair_code&0x00FF;
        while(pair_code.hour >= 12) {
            pair_code.hour %= 12;
        }
        while(pair_code.minute >= 12) {
            pair_code.minute %= 12;
        }
        pair_code.minute *= 5;
        bcd_hour = hex_to_bcd(pair_code.hour);
        bcd_minute = hex_to_bcd(pair_code.minute);
        pair_code.pair_code = (bcd_hour<<8)|bcd_minute;
        if(pair_code.pair_code != old_pair_code) {
            break;
        }
    }
    print_str_hex((u8*)&"gen pair code=0x", pair_code.pair_code);
    //print_str_dec((u8*)&"hour=", pair_code.hour);
    //print_str_dec((u8*)&"minute=", pair_code.minute);
	motor_hour_to_position(pair_code.hour);
	motor_minute_to_position(pair_code.minute);
}
s16 state_ble_pairing(REPORT_E cb, void *args)
{

    u8* code = cmd_get()->pair_code.code;
    u16 pairing_code = (code[0]<<8)|code[1];
    print_str_hex((u8*)&"recv pairing code=0x", pairing_code);
    
    state = (STATE_E *)args;
    *state = STATE_MAX;
    if(pairing_code == 0xFFFF) {
        print((u8*)&"enter pairing mode", 18);
        pair_code.pair_bgn = 1;
        pair_code_generate();
        state_later = PAIRING_INITIATE;//PAIRING_MATCHING;
    } else if(pairing_code == pair_code.pair_code) {
        state_later = CLOCK;
        pair_code.pair_bgn = 0;
        print((u8*)&"pairing match", 13);
    } else if(pair_code.pair_bgn == 1) {
        state_later = PAIRING_INITIATE;
        print((u8*)&"pairing mis-match", 17);
        pair_code_generate();
    } else {
        *state = CLOCK;
        print((u8*)&"not pairing mode", 16);
        return 0;
    }
    timer_event(PAIRING_PROTECT_INTERVAL, pairing_protect_cb_handler);

	return 0;
}
s16 state_ble_stop_advertise(REPORT_E cb, void *args)
{
    print((u8*)&"adv stop", 8);
    pair_code.pair_bgn = 0;
    motor_notify_to_position(NOTIFY_NONE);

	return 0;
}
s16 state_ble_advertise(REPORT_E cb, void *args)
{
    print((u8*)&"adv start", 9);
    pair_code.pair_bgn = 0;
    timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);

	return 0;
}
s16 state_ble_disconnect(REPORT_E cb, void *args)
{
    print((u8*)&"disconnect", 10);
    pair_code.pair_bgn = 0;

	return 0;
}
s16 state_ble_connect(REPORT_E cb, void *args)
{
    print((u8*)&"connect", 7);
    motor_notify_to_position(NOTIFY_NONE);

	return 0;
}
pair_code_t *pair_code_get(void)
{
    return &pair_code;
}

