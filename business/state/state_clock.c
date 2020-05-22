#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

static u8 day[] = {DAY_0,
	DAY_1, DAY_2, DAY_3, DAY_4, DAY_5,
	DAY_6, DAY_7, DAY_8, DAY_9, DAY_10,
	DAY_11, DAY_12, DAY_13, DAY_14, DAY_15,
	DAY_16, DAY_17, DAY_18, DAY_19, DAY_20,
	DAY_21, DAY_22, DAY_23, DAY_24, DAY_25,
	DAY_26, DAY_27,	DAY_28,	DAY_29, DAY_30,
	DAY_31};


#define TEST_CLOCK

#ifdef TEST_CLOCK
static clock_t clk = {
	.year = 1970,
	.month = 1,
	.day = 1,
	.week = 0,
	.hour = 11,
	.minute = 30,
	.second = 0,
};

#endif

s16 state_clock(REPORT_E cb, void *args)
{
	//u8 string_hour[4] = {'h', 'o', 'u', 'r'};

	#ifndef TEST_CLOCK	
	clock_t *clk;
	clk = clock_get();
	motor_minute_to_position(clk->minute);
	motor_hour_to_position(clk->hour);
    motor_date_to_position(day[clk->day]);
	#else
	clk.minute++;
	if(60 == clk.minute) {
		clk.minute = 0;
		clk.hour++;
	}

	if(24 == clk.hour) {
		clk.hour = 0;
		clk.day++;
	}

	if(31 == clk.day) {
		clk.day = 0;
	}

    //print((u8*)&"sec clock", 9);
	motor_minute_to_position(clk.minute);
	motor_hour_to_position(clk.hour);
    motor_date_to_position(day[clk.day]);
	#endif

	return 0;
}
