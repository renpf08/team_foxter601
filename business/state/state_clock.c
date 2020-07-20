#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

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
//static void sport_activity_calc(void)
//{
//    u32 target_steps = cmd_get()->user_info.target_steps;
//    u32 current_steps = sport_get()->StepCounts;
//    u8 activity = 40; // total 40 grids
//
//    if(current_steps < target_steps) {
//        activity = (current_steps*40)/target_steps;
//    } else if(target_steps == 0) {
//        activity = 0;
//    }
//    motor_activity_to_position(activity);
//}
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
static void minute_data_handler(clock_t *clock)
{
    his_data_t data;
    SPORT_INFO_T* sport_info = NULL;
    static u8 sys_init = 0;
    
    MemSet(&data, 0, sizeof(his_data_t));
    if(sys_init == 0) {
        sys_init = 1;
        sport_set(&cmd_get()->user_info);
        return;
    } else if((clock->hour == 59)&&(clock->minute == 59)&&(clock->second == 59)) { // new day
        sport_info = sport_get();
        data.year = clock->year;
        data.month = clock->month;
        data.day = clock->day;
        data.steps = sport_info->StepCounts;
        #if USE_DEV_CALORIE
        data.colorie = sport_info->Calorie;
        data.acute = sport_info->AcuteSportTimeCounts;
        #endif
        nvm_write_data(&data);
        sport_clear();
    } else {
        #if USE_DEV_CALORIE
        sport_minute_calc();
        #endif
        //cmd_resp(CMD_SYNC_DATA, 0, (u8*)&"\xF5\xFA"); // send real-time data every minutes
    }
    alarm_clock_check(clock);
//    sport_activity_calc();
}
s16 state_clock(REPORT_E cb, void *args)
{
    #if USE_UART_PRINT
	print((u8 *)&"clock", 5);
    #endif

	#ifndef TEST_CLOCK
	clock_t *clk;
	clk = clock_get();
	motor_minute_to_position(clk->minute);
	motor_hour_to_position(clk->hour);
    motor_date_to_position(date[clk->day]);
    minute_data_handler(clk);
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
    minute_data_handler(clk);
	#endif

	return 0;
}
