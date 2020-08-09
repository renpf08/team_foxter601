#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

static void alarm_clock_check(clock_t *clock)
{
    cmd_set_alarm_clock_t *alarm_clock = (cmd_set_alarm_clock_t*)&cmd_get()->set_alarm_clock;
    u8 i = 0;

    for(i = 0; i < 4; i++) {
        if((alarm_clock->aclk[i].en == 1) && 
          ((alarm_clock->aclk[i].week&0x80) || (alarm_clock->aclk[i].week&(1<<clock->week))) &&
          ((alarm_clock->aclk[i].hour==clock->hour)&&(alarm_clock->aclk[i].minute==clock->minute)&&(clock->minute==0))) {
            BLE_SEND_LOG((u8*)&i, 1);
            break;
        }
    }
}
static void minutely_check(REPORT_E cb)
{
    his_data_t data;
    clock_t* clock = clock_get();
    cmd_params_t* params = cmd_get_params();

    if((clock->hour == 59)&&(clock->minute == 59)&&(clock->second == 59)) { // new day
        adapter_ctrl.motor_dst[activity_motor] = 40;
        data.year = clock->year;
        data.month = clock->month;
        data.day = clock->day;
        data.steps = step_get();
        nvm_write_data(&data);
        step_clear();
    }
    params->steps = step_get();
    params->clock = clock;
    alarm_clock_check(clock);
}
static void read_current_step(void)
{
    cmd_params_t* params = cmd_get_params();

    params->clock = clock_get();
    params->steps = step_get();
}
static void nvm_access(REPORT_E cb)
{
    his_data_t data;
    his_ctrl_t ctrl;
    cmd_params_t* params = cmd_get_params();
    #if USE_PARAM_STORE
    cmd_group_t *value = cmd_get();
    #endif

    MemSet(&data, 0, sizeof(his_data_t));
    MemSet(&ctrl, 0, sizeof(his_ctrl_t));
    if(cb == READ_HISDAYS) {
        nvm_read_ctrl(&ctrl);
        ctrl.read_tail = ctrl.ring_buf_tail; // reset read pointer
        nvm_write_ctrl(&ctrl);
        params->days = nvm_get_days();
    } else if(cb == READ_HISDATA) {
        nvm_read_history_data((u16*)&data, READ_HISDATA_TOTAL);//res = nvm_read_data(&data);//
        params->data = &data;
    }
    #if USE_PARAM_STORE
    else if(cb == WRITE_ALARM_CLOCK) {
        nvm_write_alarm_clock((u16*)&value->set_alarm_clock, 0);
    } else if(cb == WRITE_USER_INFO) {
        nvm_write_personal_info((u16*)&value->user_info, 0);
    } else if(cb == READ_SYS_PARAMS) {
        nvm_read_alarm_clock((u16*)&value->set_alarm_clock, 0);
        nvm_read_pairing_code((u16*)&value->pair_code, 0);
        nvm_read_personal_info((u16*)&value->user_info, 0);
        cmd_check(value);
    }
    #endif
}
static u8 state_check(REPORT_E cb)
{
    #if USE_PARAM_STORE
    static u8 flag = 0;

    if(flag == 0) {
        cb = READ_SYS_PARAMS;
    }
    #endif
    switch(cb) {
    case READ_CURDATA:
        read_current_step();
        break;
    case READ_HISDAYS:
    case READ_HISDATA:
    #if USE_PARAM_STORE
    case READ_SYS_PARAMS:
    #endif
    case WRITE_USER_INFO:
    case WRITE_ALARM_CLOCK:
        nvm_access(cb);
        break;
    case REFRESH_STEPS:
        refresh_step();
        break;
    case SET_TIME:
        sync_time();
        break;
    default:
        return 0;
        break;
    }

    #if USE_PARAM_STORE
    if(flag == 0) {
        flag = 1;
        return 0;
    }
    #endif
    return 1;
}
s16 state_clock(REPORT_E cb, void *args)
{
    MOTOR_MASK_E mask = MOTOR_MASK_NONE;
	clock_t *clock = clock_get();

    if(state_check(cb) != 0) {
        return 0;
    }
    if(cb == KEY_A_B_LONG_PRESS) {
        mask = (MOTOR_MASK_ALL);
    } else {
        mask = (MOTOR_MASK_HOUR|MOTOR_MASK_MINUTE|MOTOR_MASK_DATE|MOTOR_MASK_BAT_WEEK);
    }
    motor_set_date_time(clock, mask, QUEUE_USER_EXIT_ADJUST);
    minutely_check(cb);

	return 0;
}
