#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>
#include <buf_utils.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

#define HISTORY_STORE_INVERVAL  15 //! minute

//#define TEST_CLOCK

#ifdef TEST_CLOCK
static clock_t clk = {
	.year = 1970,
	.month = 1,
	.day = 1,
	.week = 0,
	.hour = 23,
	.minute = 30,
	.second = 0,
};
#endif

static void clock_activity_handler(u16 id)
{
    u32 target_steps = cmd_get()->user_info.target_steps;
    u32 current_steps = step_get();
    u8 activity = 40; // total 40 grids

    if(current_steps < target_steps) {
        activity = (current_steps*40)/target_steps;
    } else if(target_steps == 0) {
        activity = 0;
    }
    motor_activity_to_position(activity);
    
    u16 length = 0; 
    u8 rsp_buf[20];
    u8 *tmp_buf = rsp_buf;
    cmd_params_t* params = cmd_get_params();
    BufWriteUint8((uint8 **)&tmp_buf, 0x01);
    BufWriteUint8((uint8 **)&tmp_buf, 0x00);
    BufWriteUint8((uint8 **)&tmp_buf, 0x00);
    BufWriteUint16((uint8 **)&tmp_buf, params->data->year);//SB100_data.AppApplyDateData.Year);
    BufWriteUint8((uint8 **)&tmp_buf, params->data->month);//SB100_data.AppApplyDateData.Month);
    BufWriteUint8((uint8 **)&tmp_buf, params->data->day);//SB100_data.AppApplyDateData.Date);
    BufWriteUint16((uint8 **)&tmp_buf, params->data->steps);//(SB100_data.AppApplyData.StepCounts));
    BufWriteUint8((uint8 **)&tmp_buf, params->data->steps>>16);//(SB100_data.AppApplyData.StepCounts>>16));
    BufWriteUint16((uint8 **)&tmp_buf, 0);//cmd_get_data->distance);//(SB100_data.AppApplyData.Distance));
    BufWriteUint8((uint8 **)&tmp_buf, 0);//cmd_get_data->distance>>16);//(SB100_data.AppApplyData.Distance>>16));
    BufWriteUint16((uint8 **)&tmp_buf, 0);//cmd_get_data->calorie);//(SB100_data.AppApplyData.Calorie));
    BufWriteUint8((uint8 **)&tmp_buf, 0);//cmd_get_data->calorie>>16);//(SB100_data.AppApplyData.Calorie>>16));
    BufWriteUint16((uint8 **)&tmp_buf, 0);//cmd_get_data->floor_counts);//SB100_data.AppApplyData.FloorCounts);
    BufWriteUint16((uint8 **)&tmp_buf, 0);//cmd_get_data->acute_sport_time);//SB100_data.AppApplyData.AcuteSportTimeCounts);
    length = tmp_buf - rsp_buf;
    BLE_SEND_DATA(rsp_buf, length);
}
static void clock_refresh_step(void)
{
    clock_t *clock = clock_get();
    cmd_params_t* params = cmd_get_params();
    params->steps = step_get();
    params->clock = clock;
    timer_event(10, clock_activity_handler);
}
static void clock_set_time(void)
{
	cmd_set_time_t *time = (cmd_set_time_t *)&cmd_get()->set_time;
    clock_t* clock = clock_get();

    clock->year = time->year[1]<<8 | time->year[0];
    clock->month = time->month;
    clock->day = time->day;
    clock->week = time->week;
    clock->hour = time->hour;
    clock->minute = time->minute;
    clock->second = time->second;

    BLE_SEND_LOG((u8*)time, sizeof(cmd_set_time_t));
	motor_minute_to_position(clock->minute);
	motor_hour_to_position(clock->hour);
    motor_date_to_position(date[clock->day]);
}
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
    }
    cmd_set(value);
    #endif
    cmd_set_params(params);
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
        clock_refresh_step();
        break;
    case SET_TIME:
        clock_set_time();
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
    #if USE_UART_PRINT
	print((u8 *)&"clock", 5);
    #endif

    if(state_check(cb) != 0) {
        return 0;
    }

	#ifndef TEST_CLOCK
	clock_t *clk;
	clk = clock_get();
	motor_minute_to_position(clk->minute);
	motor_hour_to_position(clk->hour);
    motor_date_to_position(date[clk->day]);
    minutely_check(cb);
	#else
	clk.minute++;
	if(60 == clk.minute) {
		clk.minute = 0;
		clk.hour++;
	}

	if(24 == clk.hour) {
		clk.hour = 0;
	}
	
	clk.day++;
	if(31 == clk.day) {
		clk.day = 0;
	}

    //print((u8*)&"system clock", 12);
	motor_minute_to_position(clk.minute);
	motor_hour_to_position(clk.hour);
    motor_date_to_position(date[clk.day]);
    //minutely_check(cb);
	#endif

	return 0;
}
