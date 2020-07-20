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
    his_data_t data;
    his_ctrl_t ctrl;
	clock_t *clock = NULL;
    SPORT_INFO_T* sport_info = NULL;
	clock = clock_get();
    cmd_params_t* params = cmd_get_params();
    #if USE_PARAM_STORE
    cmd_group_t *value = cmd_get();
    #endif

    MemSet(&data, 0, sizeof(his_data_t));
    MemSet(&ctrl, 0, sizeof(his_ctrl_t));
    if(cb == READ_STEPS_TARGET) {
        params->clock = clock;
    } else if(cb == READ_HISDAYS) {
        nvm_read_ctrl(&ctrl);
        ctrl.read_tail = ctrl.ring_buf_tail; // reset read pointer
        nvm_write_ctrl(&ctrl);
        params->days = nvm_get_days();
    } else if(cb == READ_HISDATA) {
        res = nvm_read_history_data((u16*)&data, READ_HISDATA_TOTAL);//res = nvm_read_data(&data);//
        params->data = &data;
    } else if(cb == READ_REALTIME_SPORT) {
        sport_info = sport_get();
        data.year = clock->year;
        data.month = clock->month;
        data.day = clock->day;
        data.steps = sport_info->StepCounts;
        #if USE_DEV_CALORIE
        data.colorie = sport_info->Calorie;
        data.acute = sport_info->AcuteSportTimeCounts;
        #endif
        params->data = &data;
    } 
    #if USE_PARAM_STORE
    else if(cb == WRITE_ALARM_CLOCK) {
        nvm_write_alarm_clock((u16*)&value->set_alarm_clock, 0);
    } else if(cb == WRITE_USER_INFO) {
        sport_set(&value->user_info);
        nvm_write_personal_info((u16*)&value->user_info, 0);
    } else if(cb == READ_SYS_PARAMS) {
        nvm_read_alarm_clock((u16*)&value->set_alarm_clock, 0);
        nvm_read_pairing_code((u16*)&value->pair_code, 0);
        nvm_read_personal_info((u16*)&value->user_info, 0);
    }
    cmd_set(value);
    #endif
    cmd_set_params(params);
	*state = CLOCK;
	return res;
}

