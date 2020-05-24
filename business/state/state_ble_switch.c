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
bool notify_swing_start = FALSE;
pair_code_t pair_code = {0, 0, 0, 0};

static u8 day2[] = {DAY_0,
	DAY_1, DAY_2, DAY_3, DAY_4, DAY_5,
	DAY_6, DAY_7, DAY_8, DAY_9, DAY_10,
	DAY_11, DAY_12, DAY_13, DAY_14, DAY_15,
	DAY_16, DAY_17, DAY_18, DAY_19, DAY_20,
	DAY_21, DAY_22, DAY_23, DAY_24, DAY_25,
	DAY_26, DAY_27,	DAY_28,	DAY_29, DAY_30,
	DAY_31};
static void clock_refresh(void)
{
    static clock_t last_clk = {0,0,0,0,0,0,0};
    
    clock_t *clock = clock_get();
    if((last_clk.day != clock->day) || (last_clk.hour != clock->hour) || (last_clk.minute != clock->minute)) {
    	motor_minute_to_position(clock->minute);
    	motor_hour_to_position(clock->hour);
        motor_date_to_position(day2[clock->day]);
    }
    last_clk.day = clock->day;
    last_clk.hour = clock->hour;
    last_clk.minute = clock->minute;
}
static void connect_clock_cb_handler(u16 id)
{
    if((ble_state_get() != app_connected) || (pair_code.pair_bgn == 1)) {
        return;
    }
    clock_refresh();
    timer_event(CLOCK_REFRESH_INTERVAL, connect_clock_cb_handler);
    print((u8*)&"connect clock", 13);
}
static void notify_swing_cb_handler(u16 id)
{
    app_state cur_state = ble_state_get();
    if((cur_state != app_fast_advertising) && (cur_state != app_slow_advertising)) {
        if(notify_swing_start == TRUE) {
            notify_swing_start = FALSE;
            motor_notify_to_position(NOTIFY_NONE);
        }
        return;
    }

    if(notify_swing_start == FALSE) {
        notify_swing_start = TRUE;
        motor_notify_to_position(NOTIFY_COMMING_CALL);
    } else {
        notify_swing_start = FALSE;
        motor_notify_to_position(NOTIFY_NONE);
    }
    clock_refresh();
    timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);
    print((u8*)&"swing clock", 11);
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
    STATE_E *state = (STATE_E *)args;
    u8* code = cmd_get()->pair_code.code;
    u16 pairing_code = (code[0]<<8)|code[1];
    //print_str_hex((u8*)&"recv pairing code=0x", pairing_code);
    
    if(pairing_code == 0xFFFF) {
        print((u8*)&"enter pairing mode", 18);
        pair_code.pair_bgn = 1;
        pair_code_generate();
        *state = PAIRING_INITIATE;
    } else if(pairing_code == pair_code.pair_code) {
        *state = PAIRING_MATCHING;
        pair_code.pair_bgn = 0;
        print((u8*)&"pairing match", 13);
    } else if(pair_code.pair_bgn == 1) {
        *state = PAIRING_INITIATE;
        print((u8*)&"pairing mis-match", 17);
        pair_code_generate();
    } else {
        print((u8*)&"not pairing mode", 16);
        return 0;
    }

	return 0;
}
s16 state_ble_stop_advertise(REPORT_E cb, void *args)
{
    //print((u8*)&"adv stop", 8);
    pair_code.pair_bgn = 0;
    motor_notify_to_position(NOTIFY_NONE);

	return 0;
}
s16 state_ble_advertise(REPORT_E cb, void *args)
{
    //print((u8*)&"adv start", 9);
    pair_code.pair_bgn = 0;
    timer_event(NOTIFY_SWING_INTERVAL, notify_swing_cb_handler);

	return 0;
}
s16 state_ble_disconnect(REPORT_E cb, void *args)
{
    //print((u8*)&"disconnect", 10);
    pair_code.pair_bgn = 0;

	return 0;
}
s16 state_ble_connect(REPORT_E cb, void *args)
{
    //print((u8*)&"connect", 7);
    pair_code.pair_bgn = 0;
    motor_notify_to_position(NOTIFY_NONE);
    timer_event(CLOCK_REFRESH_INTERVAL, connect_clock_cb_handler);

	return 0;
}
s16 state_ble_switch(REPORT_E cb, void *args)
{
    app_state cur_state = ble_state_get();
    if((cur_state == app_fast_advertising) || (cur_state == app_slow_advertising) || (cur_state == app_connected)) {
        print((u8*)&"switch off", 10);
        ble_switch_off();
    } else {
        print((u8*)&"switch on", 9);
        ble_switch_on();
    }

	return 0;
}
pair_code_t *pair_code_get(void)
{
    return &pair_code;
}

#if 0
#define BLE_STATE_FILL(init, event, next, function) {\
						.init_state = init,\
						.ev = event,\
						.next_state = next,\
						.func = function,\
					}

typedef enum {
	BLE_STATE_INIT,
	BLE_STATE_SWITCH,
	BLE_STATE_CHANGE,
	BLE_STATE_PAIRING,
	BLE_STATE_MAX,
}BLE_STATE_E;

typedef enum {
	BLE_REPORT_INIT,
	BLE_REPORT_SWITCH,
	BLE_REPORT_CHANGE,
	BLE_REPORT_PAIRING,
	BLE_REPORT_MAX,
}BLE_REPORT_E;

typedef struct {
	STATE_E now;
}ble_state_t;

static ble_state_t ble_state = {
	.ble_state_now = BLE_STATE_INIT,
};

static state_t ble_state_mc[] = {
	BLE_STATE_FILL(CLOCK,               BLE_ADVERTISE,          BLE_ADVERTISING,        state_ble_advertise),
	BLE_STATE_FILL(CLOCK,               BLE_CONNECT,            BLE_CONNECTED,          state_ble_connect),
	BLE_STATE_FILL(BLE_SWITCH,          BLE_ADVERTISE,          BLE_ADVERTISING,        state_ble_advertise),
	BLE_STATE_FILL(BLE_SWITCH,          BLE_STOP_ADVERTISE,     BLE_STOP_ADVERTISING,   state_ble_stop_advertise),
	BLE_STATE_FILL(BLE_ADVERTISING,     BLE_CONNECT,            BLE_CONNECTED,          state_ble_connect),
	BLE_STATE_FILL(BLE_ADVERTISING,     KEY_M_LONG_PRESS,       BLE_SWITCH,             state_ble_switch),
	BLE_STATE_FILL(BLE_CONNECTED,       PAIRING_PROC,           PAIRING_INITIATE,       state_ble_pairing),
	BLE_STATE_FILL(BLE_CONNECTED,       KEY_M_LONG_PRESS,       BLE_SWITCH,             state_ble_switch),
	BLE_STATE_FILL(BLE_CONNECTED,       BLE_ADVERTISE,          BLE_ADVERTISING,        state_ble_advertise), //app disconnect
	BLE_STATE_FILL(PAIRING_INITIATE,    PAIRING_PROC,           PAIRING_MATCHING,       state_ble_pairing),
	BLE_STATE_FILL(PAIRING_INITIATE,    BLE_ADVERTISE,          BLE_ADVERTISING,        state_ble_advertise), //app disconnect
	BLE_STATE_FILL(PAIRING_INITIATE,    KEY_M_LONG_PRESS,       BLE_SWITCH,             state_ble_switch),
	BLE_STATE_FILL(PAIRING_MATCHING,    CLOCK_1_MINUTE,         CLOCK,                  state_clock),
	BLE_STATE_FILL(BLE_STOP_ADVERTISING,CLOCK_1_MINUTE,         CLOCK,                  state_clock),
};

static s16 state_ble_switch_cb_handler(REPORT_E cb, void *args)
{
	u16 i = 0;

    u8 cb_buf[11] = {"cb=xx,st=xx"};
    cb_buf[3] = (cb/10)+'0';
    cb_buf[4] = (cb%10)+'0';
    cb_buf[9] = (ble_state.now/10)+'0';
    cb_buf[10] = (ble_state.now%10)+'0';
    print(cb_buf, 11);/**/
    

	for(i = 0; i < sizeof(ble_state_mc)/sizeof(state_t); i++) {
		if((ble_state_mc[i].init_state == ble_state.now) && 
			(ble_state_mc[i].ev == cb)) {
			ble_state.now = ble_state_mc[i].next_state;
			ble_state_mc[i].func(cb, &ble_state.now);
            break;
		}
	}
	return 0;
}

#endif

