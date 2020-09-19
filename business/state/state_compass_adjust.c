#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

static s16 compass_adj_tid = TIMER_INVALID;

static void compass_adjust_handler(u16 id)
{
    static u8 last_angle = 0;
    u8 angle = angle_get();
    volatile u16 angle_to_pos = 0;
    u8 tmp_angle = 0;
    u8 ble_log[6] = {CMD_TEST_SEND, BLE_LOG_COMPASS_ADJ, 0, 0, 0, 0};

    if(key_m_ctrl.compass_adj_state == 0) {
        return;
    }

    if(angle != last_angle) {
        angle_to_pos = angle/3;
        while(angle_to_pos>60) angle_to_pos-=60;
    	motor_minute_to_position(angle_to_pos);
    	motor_hour_to_position(angle_to_pos);

        tmp_angle = angle;
        ble_log[2] = tmp_angle;
        ble_log[3] = angle_to_pos;
        ble_log[4] = tmp_angle/100;
        tmp_angle = tmp_angle%100;
        ble_log[5] = (tmp_angle/10<<4)&0xF0;
        tmp_angle = tmp_angle%10;
        ble_log[5] |= tmp_angle&0x0F;
        BLE_SEND_LOG(ble_log, 6);
    }
    last_angle = angle;
    compass_adj_tid = TIMER_INVALID;
    timer_event(500, compass_adjust_handler);
}
s16 state_compass_adjust(REPORT_E cb, void *args)
{
    STATE_E *state = (STATE_E *)args;
    motor_run_status_t *motor_sta = motor_get_status();
    clock_t* clock = clock_get();
    u8 hour_pos;

    key_m_ctrl.compass_adj_state = (key_m_ctrl.compass_adj_state==0)?1:0;
    if(key_m_ctrl.compass_adj_state == 1) {
        hour_pos = motor_sta[minute_motor].dst_pos;
    	motor_hour_to_position(hour_pos);
        compass_adj_tid = timer_event(1, compass_adjust_handler);
    } else {
    	motor_minute_to_position(clock->minute);
    	motor_hour_to_position(clock->hour);
        *state = CLOCK;
    }
    
	return 0;
}

