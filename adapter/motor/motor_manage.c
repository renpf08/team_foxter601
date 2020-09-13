#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <time.h>
#include "../adapter.h"

void motor_run_handler(u16 id);
static void motor_run_positive_one_unit(u16 id);
static void motor_run_negtive_one_unit(u16 id);
static void motor_run_continue_check(void);

#define LOOP_TIME         10
#define BAT_INTERVAL_STEP 2
#define WEEK_INTERVAL_STEP 5

enum {
	FIRST_HALF,
	RECOVER,
	SECOND_HALF,
	STOP,
};

typedef struct {
	/*get driver layer*/
	driver_t *drv;

	motor_run_status_t motor_status[max_motor];

	/*record step run state and run step interval*/
	motor_t *run_motor;
	u8 run_motor_num;
	u8 run_next_motor_num;
	u8 run_step_num;
	u8 run_interval_ms;
	u8 run_direction;
	u8 bat_week_dst;
	s16 tid;

	/*run one step param*/
	motor_t *run_step_motor;
	u8 run_step_motor_num;	
	u8 run_step_direction;
	s16 step_tid;
}motor_manager_t;

static motor_manager_t motor_manager = {
	.drv = NULL,
	.motor_status = {
		[hour_motor] = {
			NULL, HOUR0_0, 0, 0, 2, FIRST_HALF
		},
		[minute_motor] = {
			NULL, MINUTE_0, 0, 0, 3, FIRST_HALF
		},
		[date_motor] = {
			NULL, DAY_1, 0, 0, 3, FIRST_HALF
		},
		[notify_motor] = {
			NULL, NOTIFY_NONE, 0, 0, 1, FIRST_HALF
		},
		[battery_week_motor] = {
			NULL, BAT_PECENT_0, 0, 0, BAT_INTERVAL_STEP, FIRST_HALF
		},
		[activity_motor] = {
			NULL, ACTIVITY_0, 0, 0, 1, FIRST_HALF
		},
	 },
	.run_motor = NULL,
	.run_motor_num = max_motor,
	.run_next_motor_num = 0,
	.run_step_num = 0,
	.run_interval_ms = 100,
	.run_direction  = pos,
	.bat_week_dst = BATTERY_WEEK_MAX,
	.tid = TIMER_INVALID,

	/*run one step param*/
	.run_step_motor = NULL,
	.run_step_motor_num = max_motor,
	.run_step_direction = pos,
	.step_tid = TIMER_INVALID,
};

u8 hour_list[] = {
	HOUR0_0,
	HOUR1_0,
	HOUR2_0,
	HOUR3_0,
	HOUR4_0,
	HOUR5_0,
	HOUR6_0,
	HOUR7_0,
	HOUR8_0,
	HOUR9_0,
	HOUR10_0,
	HOUR11_0,
	HOUR12_0,
};

s16 motor_hour_to_position(u8 hour)
{
	/*hour dst position configuration*/
	u8 minute_pos = (motor_manager.motor_status[minute_motor].dst_pos == MINUTE_60) ? \
						MINUTE_0 : motor_manager.motor_status[minute_motor].dst_pos;

	if(hour >= 12) {
		hour -= 12;
	}

	motor_manager.motor_status[hour_motor].dst_pos = hour_list[hour] + minute_pos/12;
	if((HOUR11_8 == motor_manager.motor_status[hour_motor].cur_pos) &&
	   (HOUR0_0 == motor_manager.motor_status[hour_motor].dst_pos) ){
		motor_manager.motor_status[hour_motor].dst_pos = HOUR12_0;
	}

	/*hour cur pos configuration*/
	if(HOUR12_0 == motor_manager.motor_status[hour_motor].cur_pos) {
		motor_manager.motor_status[hour_motor].cur_pos = HOUR0_0;
	}
	   
	if(motor_manager.motor_status[hour_motor].dst_pos != 
	   motor_manager.motor_status[hour_motor].cur_pos) {
		motor_manager.motor_status[hour_motor].run_flag = 1;
	}
	return 0;
}

s16 motor_minute_to_position(u8 minute)
{
	/*minute dst pos config*/
	if((MINUTE_59 == motor_manager.motor_status[minute_motor].cur_pos) &&
		(MINUTE_0 == minute)) {
		motor_manager.motor_status[minute_motor].dst_pos = MINUTE_60;
	}else {
		motor_manager.motor_status[minute_motor].dst_pos = minute;
	}

	/*minute cur pos config*/
	if(MINUTE_60 == motor_manager.motor_status[minute_motor].cur_pos) {
		motor_manager.motor_status[minute_motor].cur_pos = MINUTE_0;
	}
	
	if(motor_manager.motor_status[minute_motor].dst_pos != 
	   motor_manager.motor_status[minute_motor].cur_pos) {
		motor_manager.motor_status[minute_motor].run_flag = 1;
	}
	return 0;
}

s16 motor_date_to_position(u8 day)
{
	motor_manager.motor_status[date_motor].dst_pos = day;
	if(motor_manager.motor_status[date_motor].dst_pos != 
	   motor_manager.motor_status[date_motor].cur_pos) {
		motor_manager.motor_status[date_motor].run_flag = 1;
	}
	return 0;
}

s16 motor_notify_to_position(u8 notify)
{
	motor_manager.motor_status[notify_motor].dst_pos = notify;
	if(motor_manager.motor_status[notify_motor].dst_pos != 
	   motor_manager.motor_status[notify_motor].cur_pos) {
		motor_manager.motor_status[notify_motor].run_flag = 1;
	}
	return 0;
}

#if 0
static void motor_battery_week_change(u16 id)
{
	if((BAT_PECENT_100 == motor_manager.motor_status[battery_week_motor].cur_pos) &&
		(0 == motor_manager.motor_status[battery_week_motor].run_flag)) {
		
		if(BAT_PECENT_100 == motor_manager.bat_week_dst) {
			return;
		}
		
		motor_manager.motor_status[battery_week_motor].dst_pos = motor_manager.bat_week_dst;
		if(motor_manager.bat_week_dst > BAT_PECENT_100) {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = BAT_INTERVAL_STEP;
		}else {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = WEEK_INTERVAL_STEP;
		}
		
		motor_manager.motor_status[battery_week_motor].run_flag = 1;
	}else {
		timer_event(100, motor_battery_week_change);
	}
}

s16 motor_battery_week_to_position(u8 battery_week)
{
	if((motor_manager.motor_status[battery_week_motor].cur_pos > BAT_PECENT_100) &&
		(battery_week <= BAT_PECENT_100)) {
		motor_manager.motor_status[battery_week_motor].dst_pos = BAT_PECENT_100;
		motor_manager.bat_week_dst = battery_week;
		timer_event(motor_manager.run_interval_ms, motor_battery_week_change);
	}else if((motor_manager.motor_status[battery_week_motor].cur_pos < BAT_PECENT_100) &&
		(battery_week >= BAT_PECENT_100)) {
		motor_manager.motor_status[battery_week_motor].dst_pos = BAT_PECENT_100;
		motor_manager.bat_week_dst = battery_week;
		timer_event(motor_manager.run_interval_ms, motor_battery_week_change);
	}else if(BAT_PECENT_100 == motor_manager.motor_status[battery_week_motor].cur_pos) {		
		motor_manager.bat_week_dst = BATTERY_WEEK_MAX;
		motor_manager.motor_status[battery_week_motor].dst_pos = battery_week;
		if(motor_manager.motor_status[battery_week_motor].dst_pos > BAT_PECENT_100) {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = BAT_INTERVAL_STEP;
		}else {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = WEEK_INTERVAL_STEP;
		}
	}else {	
		motor_manager.bat_week_dst = BATTERY_WEEK_MAX;
		motor_manager.motor_status[battery_week_motor].dst_pos = battery_week;
		if(motor_manager.motor_status[battery_week_motor].dst_pos > BAT_PECENT_100) {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = BAT_INTERVAL_STEP;
		}else {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = WEEK_INTERVAL_STEP;
		}
	}

	if(motor_manager.motor_status[battery_week_motor].dst_pos != 
	   motor_manager.motor_status[battery_week_motor].cur_pos) {
		motor_manager.motor_status[battery_week_motor].run_flag = 1;
	}
	return 0;
}

static void motor_battery_week_change(u16 id)
{
	if((BAT_PECENT_100 == motor_manager.motor_status[battery_week_motor].cur_pos) &&
		(0 == motor_manager.motor_status[battery_week_motor].run_flag)) {
		
		if(BAT_PECENT_100 == motor_manager.bat_week_dst) {
			return;
		}
		
		motor_manager.motor_status[battery_week_motor].dst_pos = motor_manager.bat_week_dst;
		if(motor_manager.bat_week_dst > BAT_PECENT_100) {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = BAT_INTERVAL_STEP;
		}else {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = WEEK_INTERVAL_STEP;
		}
		
		motor_manager.motor_status[battery_week_motor].run_flag = 1;
	}else {
		timer_event(100, motor_battery_week_change);
	}
}
#else

s16 motor_battery_week_to_position(u8 battery_week)
{
	if((motor_manager.motor_status[battery_week_motor].cur_pos > BAT_PECENT_100) &&
		(battery_week <= BAT_PECENT_100)) {
		motor_manager.motor_status[battery_week_motor].dst_pos = BAT_PECENT_100;
		motor_manager.bat_week_dst = battery_week;
	}else if((motor_manager.motor_status[battery_week_motor].cur_pos < BAT_PECENT_100) &&
		(battery_week >= BAT_PECENT_100)) {
		motor_manager.motor_status[battery_week_motor].dst_pos = BAT_PECENT_100;
		motor_manager.bat_week_dst = battery_week;
	}else if(BAT_PECENT_100 == motor_manager.motor_status[battery_week_motor].cur_pos) {		
		motor_manager.bat_week_dst = BATTERY_WEEK_MAX;
		motor_manager.motor_status[battery_week_motor].dst_pos = battery_week;
		if(motor_manager.motor_status[battery_week_motor].dst_pos > BAT_PECENT_100) {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = BAT_INTERVAL_STEP;
		}else {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = WEEK_INTERVAL_STEP;
		}
	}else {	
		motor_manager.bat_week_dst = BATTERY_WEEK_MAX;
		motor_manager.motor_status[battery_week_motor].dst_pos = battery_week;
		if(motor_manager.motor_status[battery_week_motor].dst_pos > BAT_PECENT_100) {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = BAT_INTERVAL_STEP;
		}else {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = WEEK_INTERVAL_STEP;
		}
	}

	if(motor_manager.motor_status[battery_week_motor].dst_pos != 
	   motor_manager.motor_status[battery_week_motor].cur_pos) {
		motor_manager.motor_status[battery_week_motor].run_flag = 1;
	}
	return 0;
}
#endif

s16 motor_activity_to_position(u8 activity)
{
	motor_manager.motor_status[activity_motor].dst_pos = activity;
	if(motor_manager.motor_status[activity_motor].dst_pos != 
	   motor_manager.motor_status[activity_motor].cur_pos) {
		motor_manager.motor_status[activity_motor].run_flag = 1;
	}
	return 0;
}

s16 motor_hour_one_step(u8 hour_step)
{
	motor_manager.motor_status[hour_motor].dst_pos = hour_step;
	
	if(motor_manager.motor_status[hour_motor].dst_pos != 
	   motor_manager.motor_status[hour_motor].cur_pos) {
		motor_manager.motor_status[hour_motor].run_flag = 1;
	}
	return 0;
}

s16 motor_minute_one_step(u8 minute_step)
{
	motor_manager.motor_status[minute_motor].dst_pos = minute_step;
	
	if(motor_manager.motor_status[minute_motor].dst_pos != 
	   motor_manager.motor_status[minute_motor].cur_pos) {
		motor_manager.motor_status[minute_motor].run_flag = 1;
	}
	return 0;
}

static void motor_run_continue_check(void)
{
	motor_manager.run_step_num++;
	if(motor_manager.run_step_num == motor_manager.motor_status[motor_manager.run_motor_num].unit_interval_step) {
		motor_manager.run_step_num = 0;

		/*motor position update*/
		if(pos == motor_manager.run_direction) {
			motor_manager.motor_status[motor_manager.run_motor_num].cur_pos++;
		}else {
			motor_manager.motor_status[motor_manager.run_motor_num].cur_pos--;
		}

		/*motor continue run or not check*/
		if(motor_manager.motor_status[motor_manager.run_motor_num].cur_pos == \
			motor_manager.motor_status[motor_manager.run_motor_num].dst_pos) {
			motor_manager.motor_status[motor_manager.run_motor_num].run_flag = 0;
		}

		/*run unit done,then check whether run or not*/
		motor_manager.tid = motor_manager.drv->timer->timer_start(LOOP_TIME, motor_run_handler);
	}else {
		if(pos == motor_manager.run_direction) {
			motor_manager.tid = motor_manager.drv->timer->timer_start(1, motor_run_positive_one_unit);
		}else {
			motor_manager.tid = motor_manager.drv->timer->timer_start(1, motor_run_negtive_one_unit);
		}
	}
}

#if 0
static void motor_time_adjust_run_continue_check(void)
{
	motor_manager.run_step_num++;
	if(motor_manager.run_step_num == motor_manager.motor_status[motor_manager.run_motor_num].unit_interval_step) {
		motor_manager.run_step_num = 0;
		
		/*motor position update*/
		if(pos == motor_manager.run_direction) {
			motor_manager.motor_status[motor_manager.run_motor_num].cur_pos++;
		}else {
			motor_manager.motor_status[motor_manager.run_motor_num].cur_pos--;
		}
		
		motor_manager.motor_status[motor_manager.run_motor_num].run_flag = 0;
	}else {
		if(pos == motor_manager.run_direction) {
			motor_manager.drv->timer->timer_start(1, motor_run_positive_one_unit);
		}else {
			motor_manager.drv->timer->timer_start(1, motor_run_negtive_one_unit);
		}
	}
}
#endif
static void motor_run_positive_one_unit(u16 id)
{
	motor_manager.drv->timer->timer_del(motor_manager.tid);
	motor_manager.tid = TIMER_INVALID;

	switch(motor_manager.motor_status[motor_manager.run_motor_num].run_step_state) {
		case FIRST_HALF:
			motor_manager.run_motor->motor_positive_first_half(NULL);
			motor_manager.motor_status[motor_manager.run_motor_num].run_step_state = RECOVER;
			motor_manager.tid = motor_manager.drv->timer->timer_start(5, motor_run_positive_one_unit);
			break;
		case RECOVER:
			motor_manager.run_motor->motor_stop(NULL);
			motor_manager.motor_status[motor_manager.run_motor_num].run_step_state = SECOND_HALF;
			motor_run_continue_check();
			break;
		case SECOND_HALF:
			motor_manager.run_motor->motor_positive_second_half(NULL);
			motor_manager.motor_status[motor_manager.run_motor_num].run_step_state = STOP;
			motor_manager.tid = motor_manager.drv->timer->timer_start(5, motor_run_positive_one_unit);
			break;
		case STOP:
			motor_manager.run_motor->motor_stop(NULL);
			motor_manager.motor_status[motor_manager.run_motor_num].run_step_state = FIRST_HALF;
			motor_run_continue_check();
			break;
		default:
			break;
	}
}

static void motor_run_negtive_one_unit(u16 id)
{
	motor_manager.tid = TIMER_INVALID;

	switch(motor_manager.motor_status[motor_manager.run_motor_num].run_step_state) {
		case FIRST_HALF:
			motor_manager.run_motor->motor_negtive_first_half(NULL);
			motor_manager.motor_status[motor_manager.run_motor_num].run_step_state = RECOVER;
			motor_manager.tid = motor_manager.drv->timer->timer_start(5, motor_run_negtive_one_unit);
			break;
		case RECOVER:
			motor_manager.run_motor->motor_stop(NULL);
			motor_manager.motor_status[motor_manager.run_motor_num].run_step_state = SECOND_HALF;
			motor_run_continue_check();
			break;
		case SECOND_HALF:
			motor_manager.run_motor->motor_negtive_second_half(NULL);
			motor_manager.motor_status[motor_manager.run_motor_num].run_step_state = STOP;
			motor_manager.tid = motor_manager.drv->timer->timer_start(5, motor_run_negtive_one_unit);
			break;
		case STOP:
			motor_manager.run_motor->motor_stop(NULL);
			motor_manager.motor_status[motor_manager.run_motor_num].run_step_state = FIRST_HALF;
			motor_run_continue_check();
			break;
		default:
			break;
	}
}

void motor_run_handler(u16 id)
{
	u8 i = 0;

	motor_manager.drv->timer->timer_del(motor_manager.tid);
	motor_manager.tid = TIMER_INVALID;

	#if 0
	if(BATTERY_WEEK_MAX != motor_manager.bat_week_dst) {
		if(BAT_PECENT_100 == motor_manager.bat_week_dst) {
			motor_manager.bat_week_dst = BATTERY_WEEK_MAX;
			goto loop;
		}
		
		motor_battery_week_change(motor_manager.bat_week_dst);
		motor_manager.motor_status[battery_week_motor].dst_pos = motor_manager.bat_week_dst;		
		motor_manager.bat_week_dst = BATTERY_WEEK_MAX;
		motor_manager.motor_status[battery_week_motor].run_flag = 1;
	}
	#else
	if((BAT_PECENT_100 == motor_manager.motor_status[battery_week_motor].cur_pos) &&
		(0 == motor_manager.motor_status[battery_week_motor].run_flag) &&
		(BATTERY_WEEK_MAX != motor_manager.bat_week_dst)) {

		if(BAT_PECENT_100 == motor_manager.bat_week_dst) {
			motor_manager.bat_week_dst = BATTERY_WEEK_MAX;
			goto loop;
		}

		motor_manager.motor_status[battery_week_motor].dst_pos = motor_manager.bat_week_dst;
		if(motor_manager.bat_week_dst > BAT_PECENT_100) {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = BAT_INTERVAL_STEP;
		}else {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = WEEK_INTERVAL_STEP;
		}
		
		motor_manager.bat_week_dst = BATTERY_WEEK_MAX;
		motor_manager.motor_status[battery_week_motor].run_flag = 1;
	}
	#endif

	/*search for run motor*/
	/*from run_next_motor_num pos to max_motor*/
	for(i = motor_manager.run_next_motor_num; i < max_motor; i++) {
		if(1 == motor_manager.motor_status[i].run_flag) {
			motor_manager.run_motor = motor_manager.motor_status[i].motor_ptr;
			motor_manager.run_motor_num = i;
			break;
		}
	}
	if(max_motor != i) {
		goto run;
	}

	/*from 0 to run_next_motor*/
	for(i = 0; i < motor_manager.run_next_motor_num; i++) {
		if(1 == motor_manager.motor_status[i].run_flag) {
			motor_manager.run_motor = motor_manager.motor_status[i].motor_ptr;
			motor_manager.run_motor_num = i;
			break;
		}
	}
	if(motor_manager.run_next_motor_num != i) {
		goto run;
	}

	/*loop*/
loop:	
	motor_manager.run_motor = NULL;
	motor_manager.run_motor_num = max_motor;
	motor_manager.run_next_motor_num = 0;
	/*check loop*/
	motor_manager.tid = motor_manager.drv->timer->timer_start(motor_manager.run_interval_ms, motor_run_handler);
	return;

run:
	/*motor run start*/
	motor_manager.run_step_num = 0;
	if(motor_manager.motor_status[motor_manager.run_motor_num].dst_pos > 
	   motor_manager.motor_status[motor_manager.run_motor_num].cur_pos) {
		motor_manager.run_direction = pos;
		motor_manager.tid = motor_manager.drv->timer->timer_start(1, motor_run_positive_one_unit);
	}else {
		motor_manager.run_direction = neg;
		motor_manager.tid = motor_manager.drv->timer->timer_start(1, motor_run_negtive_one_unit);
	}

	/*update next motor num*/
	motor_manager.run_next_motor_num++;
	if(max_motor == motor_manager.run_next_motor_num) {
		motor_manager.run_next_motor_num = 0;
	}
}

static void motor_run_pos_one_step(u16 id)
{
	motor_manager.step_tid = TIMER_INVALID;
	switch(motor_manager.motor_status[motor_manager.run_step_motor_num].run_step_state) {
		case FIRST_HALF:
			motor_manager.run_step_motor->motor_positive_first_half(NULL);
			motor_manager.motor_status[motor_manager.run_step_motor_num].run_step_state = RECOVER;
			motor_manager.step_tid = motor_manager.drv->timer->timer_start(5, motor_run_pos_one_step);
			break;
		case RECOVER:
			motor_manager.run_step_motor->motor_stop(NULL);
			motor_manager.motor_status[motor_manager.run_step_motor_num].run_step_state = SECOND_HALF;
			break;
		case SECOND_HALF:
			motor_manager.run_step_motor->motor_positive_second_half(NULL);
			motor_manager.motor_status[motor_manager.run_step_motor_num].run_step_state = STOP;
			motor_manager.step_tid = motor_manager.drv->timer->timer_start(5, motor_run_pos_one_step);
			break;
		case STOP:
			motor_manager.run_step_motor->motor_stop(NULL);
			motor_manager.motor_status[motor_manager.run_step_motor_num].run_step_state = FIRST_HALF;
			break;
		default :
			break;
	}
}

static void motor_run_neg_one_step(u16 id)
{
	motor_manager.step_tid = TIMER_INVALID;
	switch(motor_manager.motor_status[motor_manager.run_step_motor_num].run_step_state) {
		case FIRST_HALF:
			motor_manager.run_step_motor->motor_negtive_first_half(NULL);
			motor_manager.motor_status[motor_manager.run_step_motor_num].run_step_state = RECOVER;
			motor_manager.step_tid = motor_manager.drv->timer->timer_start(5, motor_run_neg_one_step);
			break;
		case RECOVER:
			motor_manager.run_step_motor->motor_stop(NULL);
			motor_manager.motor_status[motor_manager.run_step_motor_num].run_step_state = SECOND_HALF;
			break;
		case SECOND_HALF:
			motor_manager.run_step_motor->motor_negtive_second_half(NULL);
			motor_manager.motor_status[motor_manager.run_step_motor_num].run_step_state = STOP;
			motor_manager.step_tid = motor_manager.drv->timer->timer_start(5, motor_run_neg_one_step);
			break;
		case STOP:
			motor_manager.run_step_motor->motor_stop(NULL);
			motor_manager.motor_status[motor_manager.run_step_motor_num].run_step_state = FIRST_HALF;
			break;
		default :
			break;
	}
}

#if 0
.run_step_motor = NULL,
.run_step_motor_num = max_motor,
.run_step_direction = pos,
#endif
void motor_run_one_step(u8 motor_num, u8 direction)
{
	motor_manager.run_step_motor = motor_manager.motor_status[motor_num].motor_ptr;
	motor_manager.run_step_motor_num = motor_num;
	motor_manager.run_step_direction = direction;
	if(pos == motor_manager.run_step_direction) {
		motor_manager.step_tid = motor_manager.drv->timer->timer_start(1, motor_run_pos_one_step);
	}else {
		motor_manager.step_tid = motor_manager.drv->timer->timer_start(1, motor_run_neg_one_step);
	}
}

/*only for zero adjust mode*/

/*定义
V长1号Motor为分针，180格，一分钟2格
V短0号Motor为时针，120格，一小时15格
V长3号Motor为日期，180格，只用180度，1天2格
V短2号Motor为星期与电量，120格，只用180度，1星期8格
I      4号Motor为运动百分比，60格，5%为3格
I      5号Motor为消息显示60格*/
s16 motor_manager_init(void)
{
	motor_manager.drv = get_driver();
	motor_manager.motor_status[hour_motor].motor_ptr = motor_manager.drv->motor_hour;
	motor_manager.motor_status[minute_motor].motor_ptr = motor_manager.drv->motor_minute;
	motor_manager.motor_status[activity_motor].motor_ptr = motor_manager.drv->motor_activity;
	motor_manager.motor_status[date_motor].motor_ptr = motor_manager.drv->motor_date;
	motor_manager.motor_status[battery_week_motor].motor_ptr = motor_manager.drv->motor_battery_week;
	motor_manager.motor_status[notify_motor].motor_ptr = motor_manager.drv->motor_notify;

	motor_manager.tid = motor_manager.drv->timer->timer_start(motor_manager.run_interval_ms, motor_run_handler);
	return 0;
}

motor_run_status_t *motor_get_status(void)
{
	return &motor_manager.motor_status[0];
}

#if 0
void time_delay_ms(u16 ms);
void time_delay_ms(u16 ms)
{
    for(; ms > 0; ms--)
    {
        TimeDelayUSec(1000);
    }
} /* TimeDelayMSec */

s16 motor_hour_test_run(u8 direction)
{
	u8 cnt;
	if(pos == direction) {
		for(cnt = 0; cnt < 5; cnt++) {
			motor_manager.drv->motor_hour->motor_positive_first_half(NULL);
			time_delay_ms(5);
			motor_manager.drv->motor_hour->motor_stop(NULL);
			time_delay_ms(5);
			motor_manager.drv->motor_hour->motor_positive_second_half(NULL);
			time_delay_ms(5);
			motor_manager.drv->motor_hour->motor_stop(NULL);
			time_delay_ms(5);
		}
		time_delay_ms(2000);
	} else if(neg == direction) {
		for(cnt = 0; cnt < 5; cnt++) {
			motor_manager.drv->motor_hour->motor_negtive_first_half(NULL);
			time_delay_ms(5);
			motor_manager.drv->motor_hour->motor_stop(NULL);
			time_delay_ms(5);
			motor_manager.drv->motor_hour->motor_negtive_second_half(NULL);
			time_delay_ms(5);
			motor_manager.drv->motor_hour->motor_stop(NULL);
			time_delay_ms(5);
		}
		time_delay_ms(2000);
	}
	return 0;
}
#endif
