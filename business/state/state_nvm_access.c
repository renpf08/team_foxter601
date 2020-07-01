#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

s16 state_access_nvm(REPORT_E cb, void *args)
{
    STATE_E *state = (STATE_E *)args;
    s16 res = 0;
    his_data_t data = {0,0,0,0};
    his_ctrl_t ctrl = {0,0,0,0};
	clock_t *clock;
	clock = clock_get();
    
    if(cb == READ_TIME_STEPS) {
        data.year = clock->year;
        data.month = clock->month;
        data.day = clock->day;
        data.steps = steps_get();
        cmd_set_data(&data);
        cmd_set_clock(clock_get());
        //cmd_set_steps(steps_get());
    } else if(cb == READ_HISDAYS) {
        nvm_read_ctrl(&ctrl);
        ctrl.read_tail = ctrl.ring_buf_tail; // reset read pointer
        nvm_write_ctrl(&ctrl);
        cmd_set_days(nvm_get_days());
    } else if(cb == READ_HISDATA) {
        res = nvm_read_history_data((u16*)&data, READ_HISDATA_TOTAL);
        cmd_set_data(&data);
    }
//    else if(cb == UPDATE_MINUTES_INFO) {
//        set_minutes_info(get_minutes_info());
//    }
    
	*state = CLOCK;
	return res;
}

