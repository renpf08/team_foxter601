#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

#define TIME_MOTOR_MAX 4

typedef struct {
	u8 motor_num;
	clock_t *clk;
}state_time_adjust_t;

static state_time_adjust_t state_time_adj = {
	.motor_num = 0,
	.clk = NULL,
};

u8 time_adj_motor[TIME_MOTOR_MAX] = {minute_motor, hour_motor, date_motor, battery_week_motor};
static void state_time_adjust_motor_run(u8 motor_num, u8 direction)
{
	switch(motor_num) {
		case hour_motor:
			if(pos == direction) {
				state_time_adj.clk->hour++;
			}else if(neg == direction){
				state_time_adj.clk->hour--;
			}
			motor_hour_to_position(state_time_adj.clk->hour);
			print(&state_time_adj.clk->hour, 1);
			break;
		case minute_motor:
			if(pos == direction) {
				state_time_adj.clk->minute++;
			}else if(neg == direction){
				state_time_adj.clk->minute--;
			}
			motor_minute_to_position(state_time_adj.clk->minute);
			print(&state_time_adj.clk->minute, 1);
			break;
		case date_motor:
			if(pos == direction) {
				state_time_adj.clk->day++;
			}else {
				state_time_adj.clk->day--;
			}
			motor_date_to_position(date[state_time_adj.clk->day]);
			break;
		case battery_week_motor:
			if(pos == direction) {
				state_time_adj.clk->week++;
			}else {
				state_time_adj.clk->week--;
			}
			motor_battery_week_to_position(state_time_adj.clk->week);
			break;
		default:
			break;
	}
}

s16 state_time_adjust(REPORT_E cb, void *args)
{
	//print((u8 *)&"time_adjust", 11);

	if(KEY_A_B_LONG_PRESS == cb) {
		state_time_adj.clk = clock_get();
		state_time_adj.motor_num = 0;
	}else if(KEY_M_SHORT_PRESS == cb) {
		/*motor switcch:minute -> hour -> date -> battery_week -> minute*/
		state_time_adj.motor_num++;
		if(TIME_MOTOR_MAX == state_time_adj.motor_num) {
			state_time_adj.motor_num = 0;
		}else if(battery_week_motor == state_time_adj.motor_num) {
			motor_battery_week_to_position(state_time_adj.clk->week);
		}
	}else if(KEY_A_SHORT_PRESS == cb) {
		/*motor run positive one unit*/
		state_time_adjust_motor_run(state_time_adj.motor_num, pos);
	}else if(KEY_B_SHORT_PRESS == cb){
		/*motor run negtive one unit*/
		state_time_adjust_motor_run(state_time_adj.motor_num, neg);
	}
	
	return 0;
}

#if 0
static u8 seq_num = 0;
static u8 time_adjust_test_seq[] = {KEY_A_B_LONG_PRESS, KEY_A_SHORT_PRESS, KEY_A_SHORT_PRESS, KEY_A_SHORT_PRESS,
							 					 		KEY_B_SHORT_PRESS, KEY_B_SHORT_PRESS, KEY_B_SHORT_PRESS,
							 		KEY_M_SHORT_PRESS, KEY_A_SHORT_PRESS, KEY_A_SHORT_PRESS, KEY_A_SHORT_PRESS,
							 							KEY_B_SHORT_PRESS, KEY_B_SHORT_PRESS, KEY_B_SHORT_PRESS};
void time_adjust_test(u16 id)
{
	if(seq_num < sizeof(time_adjust_test_seq)) {
		state_time_adjust(time_adjust_test_seq[seq_num++], NULL);
		timer_event(2000, time_adjust_test);
	}
}
#else
void time_adjust_test(u16 id)
{
	motor_hour_test_run(pos);
	motor_hour_test_run(neg);
}
#endif
