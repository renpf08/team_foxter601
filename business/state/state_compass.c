#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

static s16 m_click_tid = TIMER_INVALID;
static s16 compass_tid = TIMER_INVALID;

static void compass_end_handler(u16 id)
{
    m_click_tid = TIMER_INVALID;
    key_sta_ctrl.m_double_click = 0;
}
static void compass_begin_handler(u16 id)
{
    static u8 last_angle = 0;
    u8 angle = angle_get();
    clock_t* clock = clock_get();
    volatile u16 minute_angle = 0;
    volatile u16 hour_angle = 0;
    u8 ble_log[5] = {LOG_SEND_FLAG, LOG_SEND_MAG_SAMPLE, 0, 0, 0};
    
    compass_tid = TIMER_INVALID;
    if(key_sta_ctrl.compass_state == 0) {
    	motor_minute_to_position(clock->minute);
    	motor_hour_to_position(clock->hour);
        return;
    }
    if(angle != last_angle) {
        hour_angle = angle/3;
        while(hour_angle>60) hour_angle-=60;
        minute_angle = hour_angle+30;
        while(minute_angle>60) minute_angle-=60;
    	motor_minute_to_position(minute_angle);
    	motor_hour_to_position(hour_angle);
        ble_log[2] = angle;
        ble_log[3] = minute_angle;
        ble_log[4] = hour_angle;
        BLE_SEND_LOG(ble_log, 5);
    }
    last_angle = angle;
    timer_event(28, compass_begin_handler);
}
s16 state_compass(REPORT_E cb, void *args)
{
    STATE_E *state = (STATE_E *)args;
    static STATE_E pre_state = INIT;
    
    if(get_last_state() != STATE_COMPASS) {
        pre_state = get_last_state(); // auto detect to return to CLOCK state or BLE_CHANGE state
    }
    
    if(key_sta_ctrl.compass_state == 1) { // 03 exit compass mode
        key_sta_ctrl.compass_state = 0;
        *state = pre_state;
        return 0;
    } else if (key_sta_ctrl.m_double_click == 1) { // 02 double click worked, enter compass mode
        key_sta_ctrl.m_double_click = 0;
        key_sta_ctrl.compass_state = 1;
        timer_remove(m_click_tid);
        m_click_tid = TIMER_INVALID;
        timer_remove(compass_tid);
        compass_tid = TIMER_INVALID;
        compass_tid = timer_event(1, compass_begin_handler);
        return 0;
    }
    key_sta_ctrl.m_double_click = 1; // 01 first click
    timer_remove(m_click_tid);
    m_click_tid = TIMER_INVALID;
    m_click_tid = timer_event(500, compass_end_handler);
    *state = pre_state;
    
	return 0;
}

