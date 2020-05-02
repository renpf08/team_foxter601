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

s16 state_clock(REPORT_E cb, void *args)
{
	clock_t *clk;

	//u8 string[11] = {'s', 't', 'a', 't', 'e', '_', 'c', 'l', 'o', 'c', 'k'};

	clk = clock_get();
    motor_minute_to_position(clk->minute);
	motor_hour_to_position(clk->hour);
    motor_date_to_position(day[clk->day]);
	
	return 0;
}
