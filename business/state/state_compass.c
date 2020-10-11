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
    u8 log_angle = 0;
    u8 angle = angle_get();
    clock_t* clock = clock_get();
    volatile u16 minute_angle = 0;
    volatile u16 hour_angle = 0;
    LOG_SEND_COMPASS_ANGLE_VARIABLE_DEF(log_send, log_send_compass_angle_t, LOG_CMD_SEND, LOG_SEND_COMPASS_ANGLE);

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
        LOG_SEND_COMPASS_ANGLE_VALUE_SET(log_send.minute_pos, minute_angle);
        LOG_SEND_COMPASS_ANGLE_VALUE_SET(log_send.hour_pos, hour_angle);
        log_angle = angle;
        LOG_SEND_COMPASS_ANGLE_VALUE_SET(log_send.angle[0], log_angle/100);           log_angle %= 100;
        LOG_SEND_COMPASS_ANGLE_VALUE_SET(log_send.angle[1], (log_angle/10<<4)&0xF0);  log_angle = log_angle%10;
        LOG_SEND_COMPASS_ANGLE_VALUE_OR(log_send.angle[1], log_angle&0x0F);
        LOG_SEND_COMPASS_ANGLE_VALUE_SEND(log_send.head);
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

