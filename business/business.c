#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "business.h"
#include "state/state.h"

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
	/*battery mode*/
	STATE_FILL(CLOCK,               BATTERY_LOW,        	LOW_VOLTAGE,            state_low_voltage),
	STATE_FILL(LOW_VOLTAGE,         BATTERY_NORMAL,     	CLOCK,                  state_clock),
	/*zero adjust mode*/
	STATE_FILL(CLOCK,               KEY_A_B_LONG_PRESS, 	ZERO_ADJUST,            state_zero_adjust),
	STATE_FILL(ZERO_ADJUST,         KEY_A_SHORT_PRESS,  	ZERO_ADJUST,            state_zero_adjust),
	STATE_FILL(ZERO_ADJUST,         KEY_B_SHORT_PRESS,  	ZERO_ADJUST,            state_zero_adjust),
	STATE_FILL(ZERO_ADJUST,         KEY_M_SHORT_PRESS,		ZERO_ADJUST,            state_zero_adjust),
	STATE_FILL(ZERO_ADJUST,         KEY_A_B_LONG_PRESS, 	CLOCK,                  state_clock),
	/*ble switch open*/	
	STATE_FILL(CLOCK,               KEY_M_LONG_PRESS,   	BLE_SWITCH,             state_ble_switch),
	STATE_FILL(CLOCK,               BLE_CHANGE,   	        BLE_SWITCH,             state_ble_switch),
	STATE_FILL(CLOCK,               BLE_PAIR,               BLE_SWITCH,             state_ble_switch),
	STATE_FILL(BLE_SWITCH,          KEY_M_LONG_PRESS,   	BLE_SWITCH,             state_ble_switch),
	STATE_FILL(BLE_SWITCH,          BLE_CHANGE,   	        BLE_SWITCH,             state_ble_switch),
	STATE_FILL(BLE_SWITCH,          BLE_PAIR,               BLE_SWITCH,             state_ble_switch),
	/*notify*/
	STATE_FILL(CLOCK,               ANCS_NOTIFY_INCOMING,   NOTIFY_COMING,          state_notify),
	/*battery & week switch*/	
	STATE_FILL(CLOCK,               KEY_M_SHORT_PRESS,   	BATTERY_WEEK_SWITCH,    state_battery_week_switch),
	/*time adjust*/
	STATE_FILL(CLOCK,               KEY_B_M_LONG_PRESS,   	TIME_ADJUST,            state_time_adjust),
	/*run test*/
	STATE_FILL(CLOCK,               KEY_A_B_M_LONG_PRESS,   RUN_TEST,               state_run_test),
	/*cmd set time*/
	STATE_FILL(CLOCK,               SET_TIME,               SET_DATE_TIME,          state_set_date_time),
	STATE_FILL(BLE_SWITCH,          SET_TIME,               SET_DATE_TIME,          state_set_date_time),
};

static s16 adapter_cb_handler(REPORT_E cb, void *args)
{
	u16 i = 0;

    /*u8 cb_buf[11] = {"cb=xx,st=xx"};
    cb_buf[3] = (cb/10)+'0';
    cb_buf[4] = (cb%10)+'0';
    cb_buf[9] = (business.state_now/10)+'0';
    cb_buf[10] = (business.state_now%10)+'0';
    print(cb_buf, 11);*/
    

	for(i = 0; i < sizeof(state)/sizeof(state_t); i++) {
		if((state[i].init_state == business.state_now) && 
			(state[i].ev == cb)) {
			business.state_now = state[i].next_state;
			state[i].func(cb, &business.state_now);
            break;
		}
	}
	return 0;
}

//#define TEST_ZERO_ADJUST
#ifdef TEST_ZERO_ADJUST
void zero_adjust_test(u16 id);

u8 test[] = {KEY_A_B_LONG_PRESS, KEY_M_SHORT_PRESS, KEY_A_SHORT_PRESS, KEY_A_SHORT_PRESS, KEY_A_SHORT_PRESS, 
			KEY_B_SHORT_PRESS, KEY_B_SHORT_PRESS, KEY_B_SHORT_PRESS};
void zero_adjust_test(u16 id)
{
	static u8 cnt = 0;
	if(cnt < sizeof(test)) {
		state_zero_adjust(test[cnt], NULL);	
		cnt++;
	}
	timer_event(1000, zero_adjust_test);
}
#endif

//#define TEST_NOTIFY
#ifdef TEST_NOTIFY
void notify_test(u16 id);

void notify_test(u16 id)
{
	state_notify(ANCS_NOTIFY_INCOMING, NULL);
	timer_event(1000, notify_test);
}
#endif

s16 business_init(void)
{
	adapter_init(adapter_cb_handler);
	#ifdef TEST_ZERO_ADJUST
	timer_event(1000, zero_adjust_test);
	#endif

	#ifdef TEST_NOTIFY
	timer_event(1000, notify_test);
	#endif
	business.state_now = CLOCK;
	state_clock(CLOCK_1_MINUTE, NULL);
	return 0;
}
