#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

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
    } else {
    	motor_minute_to_position(clock->minute);
    	motor_hour_to_position(clock->hour);
        *state = CLOCK;
    }
    
	return 0;
}

