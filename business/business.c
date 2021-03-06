#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "business.h"
#include "state/state.h"
#include "../log.h"

#define STATE_FILL(init, event, next, function) {\
						.init_state = init,\
						.ev = event,\
						.next_state = next,\
						.func = function,\
					}

typedef struct {
	STATE_E state_now;
}business_t;

static business_t business = {
	.state_now = INIT,
};

static state_t state[] = {
	STATE_FILL(CLOCK,               CLOCK_1_MINUTE,     	CLOCK,                  state_clock),
	STATE_FILL(CLOCK,               KEY_A_SHORT_PRESS,     	CLOCK,                  state_clock),
	//STATE_FILL(CLOCK,               KEY_M_SHORT_PRESS,     	CLOCK,                  state_clock),
	STATE_FILL(CLOCK,               SET_TIME,               CLOCK,                  state_clock),
//	STATE_FILL(CLOCK,               REFRESH_STEPS,          CLOCK,                  state_clock),
	STATE_FILL(CLOCK,               READ_HISDAYS,           CLOCK,                  state_clock),
	STATE_FILL(CLOCK,               READ_HISDATA,           CLOCK,                  state_clock),
	STATE_FILL(CLOCK,               READ_CURDATA,           CLOCK,                  state_clock),
	#if USE_PARAM_STORE
	STATE_FILL(CLOCK,               WRITE_USER_INFO,        CLOCK,                  state_clock),
	STATE_FILL(CLOCK,               WRITE_ALARM_CLOCK,      CLOCK,                  state_clock),
	#endif
	STATE_FILL(CLOCK,               CHARGE_SWING,           CLOCK,                  state_clock),
	STATE_FILL(CLOCK,               CHARGE_STOP,            CLOCK,                  state_clock),
	/*battery mode*/
	//STATE_FILL(CLOCK,       BATTERY_LOW,        	LOW_VOLTAGE, 	state_low_voltage),
	//STATE_FILL(LOW_VOLTAGE, BATTERY_NORMAL,     	CLOCK,       	state_clock),
	/*zero adjust mode*/
	STATE_FILL(CLOCK,               KEY_A_B_LONG_PRESS, 	ZERO_ADJUST,            state_zero_adjust),
	STATE_FILL(BLE_SWITCH,          KEY_A_B_LONG_PRESS, 	ZERO_ADJUST,            state_zero_adjust),
	STATE_FILL(ZERO_ADJUST,         KEY_A_SHORT_PRESS,  	ZERO_ADJUST,            state_zero_adjust),
	STATE_FILL(ZERO_ADJUST,         KEY_B_SHORT_PRESS,  	ZERO_ADJUST,            state_zero_adjust),
	STATE_FILL(ZERO_ADJUST,         KEY_M_SHORT_PRESS,		ZERO_ADJUST,            state_zero_adjust),
	STATE_FILL(ZERO_ADJUST,         KEY_A_B_LONG_PRESS, 	ZERO_ADJUST,            state_zero_adjust),
	/*ble switch open*/	
	STATE_FILL(CLOCK,               KEY_M_LONG_PRESS,   	BLE_SWITCH,             state_ble_switch),
	STATE_FILL(CLOCK,               BLE_CHANGE,   	        BLE_SWITCH,             state_ble_switch),
	STATE_FILL(CLOCK,               BLE_PAIR,               BLE_SWITCH,             state_ble_switch),
	STATE_FILL(BLE_SWITCH,          KEY_M_LONG_PRESS,   	BLE_SWITCH,             state_ble_switch),
	STATE_FILL(BLE_SWITCH,          BLE_CHANGE,   	        BLE_SWITCH,             state_ble_switch),
	STATE_FILL(BLE_SWITCH,          BLE_PAIR,               BLE_SWITCH,             state_ble_switch),
	STATE_FILL(BLE_SWITCH,          SET_TIME,               BLE_SWITCH,             state_ble_switch),
//	STATE_FILL(BLE_SWITCH,          REFRESH_STEPS,          BLE_SWITCH,             state_ble_switch),
	STATE_FILL(BLE_SWITCH,          CHARGE_SWING,           BLE_SWITCH,             state_ble_switch),
	STATE_FILL(BLE_SWITCH,          CHARGE_STOP,            BLE_SWITCH,             state_ble_switch),
	/*notify*/
	STATE_FILL(CLOCK,               ANCS_NOTIFY_INCOMING,   NOTIFY_COMING,          state_notify),
	STATE_FILL(CLOCK,               ANDROID_NOTIFY,         NOTIFY_COMING,          state_notify),
	/*battery & week switch*/
	STATE_FILL(CLOCK,               KEY_M_SHORT_PRESS,      BATTERY_WEEK_SWITCH,    state_battery_week_switch),
	STATE_FILL(BLE_SWITCH,          KEY_M_SHORT_PRESS,      BATTERY_WEEK_SWITCH,    state_battery_week_switch),
	/*time adjust*/
	STATE_FILL(CLOCK,               KEY_B_M_LONG_PRESS,   	TIME_ADJUST,            state_time_adjust),
	STATE_FILL(TIME_ADJUST,         KEY_A_SHORT_PRESS,   	TIME_ADJUST,            state_time_adjust),	
	STATE_FILL(TIME_ADJUST,         KEY_B_SHORT_PRESS,   	TIME_ADJUST,            state_time_adjust),
	STATE_FILL(TIME_ADJUST,         KEY_M_SHORT_PRESS,   	TIME_ADJUST,            state_time_adjust),
	STATE_FILL(TIME_ADJUST,         KEY_B_M_LONG_PRESS,   	TIME_ADJUST,            state_time_adjust),
	/*run test*/
	STATE_FILL(CLOCK,       		KEY_A_B_M_LONG_PRESS,   RUN_TEST,  				state_run_test),
	STATE_FILL(RUN_TEST,    		KEY_A_B_M_LONG_PRESS,   RUN_TEST,  				state_run_test),
	/*run compass*/
	STATE_FILL(CLOCK,       		COMPASS,                STATE_COMPASS,  	    state_compass),
	STATE_FILL(BLE_SWITCH,    		COMPASS,                STATE_COMPASS,  	    state_compass),
	STATE_FILL(STATE_COMPASS,    	COMPASS,                STATE_COMPASS,  	    state_compass),
};

static s16 adapter_cb_handler(REPORT_E cb, void *args)
{
	u16 i = 0;
    s16 res = 0;
    LOG_SEND_STATE_MACHINE_VARIABLE_DEF(log_send, log_send_sta_mc_t, LOG_CMD_SEND, LOG_SEND_STATE_MACHINE);
    //log_send_sta_mc_t log_send = {.head = {LOG_CMD_SEND, LOG_SEND_STATE_MACHINE, sizeof(log_send_sta_mc_t), 0}};

    LOG_SEND_STATE_MACHINE_VALUE_SET(log_send.sta_now, business.state_now);
    LOG_SEND_STATE_MACHINE_VALUE_SET(log_send.sta_report, cb);
    LOG_SEND_STATE_MACHINE_VALUE_SET(log_send.result, 0);
//    log_send.sta_now = business.state_now;
//    log_send.sta_report = cb;
//    log_send.result = 0;

	//return 0;
    #if USE_UART_PRINT
	print((u8 *)&cb, 1);
    #endif

    if(cb == REPORT_MAX) {
        return 0;
    }

	for(i = 0; i < sizeof(state)/sizeof(state_t); i++) {
		if((state[i].init_state == business.state_now) && 
			(state[i].ev == cb)) {
			set_last_state(business.state_now);
			business.state_now = state[i].next_state;
			res = state[i].func(cb, &business.state_now);
            LOG_SEND_STATE_MACHINE_VALUE_SET(log_send.result, 1);
//            log_send.result = 1;
            break;
		}
	}
    LOG_SEND_STATE_MACHINE_VALUE_SET(log_send.sta_next, business.state_now);
    LOG_SEND_STATE_MACHINE_VALUE_SEND(log_send.head);
//    log_send.sta_next = business.state_now;
//    log_send_initiate(&log_send.head);
	return res;
}

//#define TEST_TIME_ADJUST
s16 business_init(void)
{
	s16 battery_week_status;

	adapter_init(adapter_cb_handler);

    #if USE_UART_PRINT
    print((u8*)&"system started.", 15);
    #endif
	
	#ifdef TEST_ZERO_ADJUST
	timer_event(1000, zero_adjust_test);
	#endif

	#ifdef TEST_NOTIFY
	notify_test();
	#endif

	#ifdef TEST_BLE_SWITCH
	timer_event(1000, ble_switch_test);
	#endif

	#ifdef TEST_RUN_TEST
	timer_event(1000, test_run_test);
	#endif

	#ifdef TEST_BATTERY_WEEK
	battery_week_test(adapter_cb_handler);
	#endif

	#ifdef TEST_TIME_ADJUST
	timer_event(1000, time_adjust_test);
	#endif
	
	battery_week_status = state_battery_week_status_get();
	if(state_battery == battery_week_status) {
		state_battery_week_switch(KEY_M_SHORT_PRESS, NULL);
	}
	
	business.state_now = CLOCK;
	//state_clock(CLOCK_1_MINUTE, NULL);
    vib_stop();
    vib_run(1, 0x00);
	return 0;
}
