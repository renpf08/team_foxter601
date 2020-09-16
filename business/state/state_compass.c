#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

typedef struct {
    u8 double_click_event;
    u8 compass_state;
} key_m_ctrl_t;
static key_m_ctrl_t key_m_ctrl = {0, 0};
static s16 m_click_tid = TIMER_INVALID;
static s16 compass_tid = TIMER_INVALID;

static void compass_end_handler(u16 id)
{
    m_click_tid = TIMER_INVALID;
    key_m_ctrl.double_click_event = 0;
}
static void compass_begin_handler(u16 id)
{
    static u8 last_angle = 0;
    u8 angle = angle_get();
    clock_t* clock = clock_get();
    u8 ble_log[3] = {CMD_TEST_SEND, BLE_LOG_MAG_SAMPLE, 0};
    
    compass_tid = TIMER_INVALID;
    if(key_m_ctrl.compass_state == 0) {
    	motor_minute_to_position(clock->minute);
    	motor_hour_to_position(clock->hour);
        return;
    }
    if(angle != last_angle) {
        ble_log[2] = angle;
        BLE_SEND_LOG(ble_log, 3);
    	motor_minute_to_position(angle%MINUTE_60);
    	motor_hour_to_position(angle%HOUR12_0);
    }
    last_angle = angle;
    timer_event(500, compass_begin_handler);
}
s16 state_compass(REPORT_E cb, void *args)
{
    STATE_E *state = (STATE_E *)args;
    
    if(key_m_ctrl.compass_state == 1) { // 03 exit compass mode
        key_m_ctrl.compass_state = 0;
        *state = CLOCK;
        return 0;
    } else if (key_m_ctrl.double_click_event == 1) { // 02 double click worked, enter compass mode
        key_m_ctrl.double_click_event = 0;
        key_m_ctrl.compass_state = 1;
        timer_remove(m_click_tid);
        m_click_tid = TIMER_INVALID;
        timer_remove(compass_tid);
        compass_tid = TIMER_INVALID;
        compass_tid = timer_event(1, compass_begin_handler);
        return 0;
    }
    key_m_ctrl.double_click_event = 1; // 01 first click
    timer_remove(m_click_tid);
    m_click_tid = TIMER_INVALID;
    m_click_tid = timer_event(1000, compass_end_handler);
    
	return 0;
}

