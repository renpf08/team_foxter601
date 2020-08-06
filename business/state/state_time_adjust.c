#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

#define TIME_MOTOR_MAX 4

typedef struct {
	u8 motor_num;
	clock_t *clk;
	u8 status;
}state_time_adjust_t;

static state_time_adjust_t state_time_adj = {
	.motor_num = 0,
	.clk = NULL,
	.status = run,
};

u8 time_adj_motor[TIME_MOTOR_MAX] = {minute_motor, hour_motor, date_motor, battery_week_motor};
static void state_time_adjust_motor_run(u8 motor_num, u8 direction)
{
    MOTOR_MASK_E mask = MOTOR_MASK_NONE;

	switch(motor_num) {
		case minute_motor:
			if(pos == direction) {
				state_time_adj.clk->minute++;
				if(MINUTE_60 == state_time_adj.clk->minute) {
					state_time_adj.clk->minute = MINUTE_0;
				}
			}else if(neg == direction){
				if(MINUTE_0 == state_time_adj.clk->minute) {
					state_time_adj.clk->minute = MINUTE_59;
				}else {
					state_time_adj.clk->minute--;
				}
			}

            mask = MOTOR_MASK_MINUTE;
            adapter_ctrl.motor_dst[minute_motor] = state_time_adj.clk->minute;
			break;
		case hour_motor:
			if(pos == direction) {
				state_time_adj.clk->hour++;
				if(24 == state_time_adj.clk->hour) {
					state_time_adj.clk->hour = 0;
				}
			}else if(neg == direction){
				if(0 == state_time_adj.clk->hour) {
					state_time_adj.clk->hour = 23;
				}else {
					state_time_adj.clk->hour--;
				}
			}
			
			//print(&state_time_adj.clk->hour, 1);
            mask = MOTOR_MASK_HOUR;
            adapter_ctrl.motor_dst[hour_motor] = state_time_adj.clk->hour;
			break;
		case date_motor:
			if(pos == direction) {
				if(DAY_1 == state_time_adj.clk->day) {
					state_time_adj.clk->day = DAY_31;
				}else {
					state_time_adj.clk->day--;
				}
			}else {
				if(DAY_31 == state_time_adj.clk->day) {
					state_time_adj.clk->day = DAY_1;
				}else {
					state_time_adj.clk->day++;
				}
			}
            mask = MOTOR_MASK_DATE;
            adapter_ctrl.motor_dst[date_motor] = adapter_ctrl.date[state_time_adj.clk->day];
			break;
		case battery_week_motor:
			if(pos == direction) {
				state_time_adj.clk->week++;
				if(state_time_adj.clk->week > SATURDAY) {
					state_time_adj.clk->week = SUNDAY;
				}
			}else {
				if(SUNDAY == state_time_adj.clk->week) {
					state_time_adj.clk->week = SATURDAY;
				}else {
					state_time_adj.clk->week--;
				}
			}
			//print(&state_time_adj.clk->week, 1);
            mask = MOTOR_MASK_DATE;
            adapter_ctrl.motor_dst[battery_week_motor] = state_time_adj.clk->week;
			break;
		default:
			return;
	}
    //motor_params_enqueue(10, mask);
}

s16 state_time_adjust(REPORT_E cb, void *args)
{
	STATE_E *state = (STATE_E *)args;
    
    #if USE_UART_PRINT
	print((u8 *)&"time_adjust", 11);
    #endif
    
	if(KEY_A_B_LONG_PRESS == cb) {
		if(run == state_time_adj.status) {
			state_time_adj.status = idle;
			state_time_adj.clk = clock_get();
			state_time_adj.motor_num = 0;
		}else {
			state_time_adj.status = run;
			//motor_time_adjust_mode_off();
			*state = CLOCK;
		}
	}else if(KEY_M_SHORT_PRESS == cb) {
		/*motor switcch:minute -> hour -> date -> battery_week -> minute*/
		state_time_adj.motor_num++;
		if(TIME_MOTOR_MAX == state_time_adj.motor_num) {
			state_time_adj.motor_num = 0;
		}else if(battery_week_motor == time_adj_motor[state_time_adj.motor_num]) {
		    adapter_ctrl.motor_dst[battery_week_motor] = state_time_adj.clk->week;
            //motor_params_enqueue(10, MOTOR_MASK_BAT_WEEK);
		}
	}else if(KEY_A_SHORT_PRESS == cb) {
		/*motor run positive one unit*/
		state_time_adjust_motor_run(time_adj_motor[state_time_adj.motor_num], pos);
	}else if(KEY_B_SHORT_PRESS == cb){
		/*motor run negtive one unit*/

		state_time_adjust_motor_run(time_adj_motor[state_time_adj.motor_num], neg);
	}
	
	return 0;
}

#if 0
static u8 seq_num = 0;
static u8 time_adjust_test_seq[] = {KEY_A_B_LONG_PRESS, 
														KEY_A_SHORT_PRESS, KEY_A_SHORT_PRESS, KEY_A_SHORT_PRESS,
														KEY_B_SHORT_PRESS, KEY_B_SHORT_PRESS, KEY_B_SHORT_PRESS};
void time_adjust_test(u16 id)
{
	if(seq_num < sizeof(time_adjust_test_seq)) {
		state_time_adjust(time_adjust_test_seq[seq_num++], NULL);
		timer_event(2000, time_adjust_test);
	}
}
#endif
