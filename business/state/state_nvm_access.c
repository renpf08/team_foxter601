#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

static u8 re_in = 0;
s16 state_access_nvm(REPORT_E cb, void *args)
{
    if(re_in == 1) {
        return 1;
    }
    re_in = 1;
    STATE_E *state = (STATE_E *)args;
    s16 res = 0;
    his_data_t data = {0,0,0,0,0,0};
    his_ctrl_t ctrl = {0,0,0,0};
	clock_t *clock = NULL;
    SPORT_INFO_T* sport_info = NULL;
	clock = clock_get();
    
    if(cb == READ_STEPS_TARGET) {
        cmd_set_clock(clock);
    } else if(cb == READ_HISDAYS) {
        nvm_read_ctrl(&ctrl);
        ctrl.read_tail = ctrl.ring_buf_tail; // reset read pointer
        nvm_write_ctrl(&ctrl);
        cmd_set_days(nvm_get_days());
    } else if(cb == READ_HISDATA) {
        res = nvm_read_history_data((u16*)&data, READ_HISDATA_TOTAL);//res = nvm_read_data(&data);//
        cmd_set_data(&data);
    } else if(cb == SET_USER_INFO) {
        sport_set(&cmd_get()->user_info);
    } else if(cb == READ_REALTIME_SPORT) {
        sport_info = sport_get();
        data.year = clock->year;
        data.month = clock->month;
        data.day = clock->day;
        data.steps = sport_info->StepCounts;
        data.colorie = sport_info->Calorie;
        data.acute = sport_info->AcuteSportTimeCounts;
        cmd_set_data(&data);
    }
    
	*state = CLOCK;
    re_in = 0;
	return res;
}

