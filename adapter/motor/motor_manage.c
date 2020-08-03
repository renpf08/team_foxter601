#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <time.h>
#include <mem.h>
#include "../adapter.h"

#define BAT_INTERVAL_STEP 2
#define WEEK_INTERVAL_STEP 5

enum {
	FIRST_HALF,
	RECOVER,
	SECOND_HALF,
	STOP,
};

typedef struct {
	/*record motor pointer*/
	motor_t *motor_ptr;
	/*record current position*/
	u8 cur_pos;
	/*record the motor dst position*/
	u8 dst_pos;
	/*run flag*/
	u8 run_flag;
	/*unit interval steps*/
	u8 unit_interval_step;
	/*run step state*/
	u8 run_step_state;
}motor_run_status_t;

typedef struct {
	/*get driver layer*/
	driver_t *drv;

	motor_run_status_t motor_status[max_motor];

	/*record step run state and run step interval*/
	motor_t *run_motor;
	u8 run_motor_num;
	u8 run_step_num[max_motor];
	u8 run_interval_ms;
	u8 run_direction;
	u8 bat_week_dst;
	u8 time_adjust_mode;
}motor_manager_t;

static motor_manager_t motor_manager = {
	.drv = NULL,
	.motor_status = {
		[minute_motor] = {
			NULL, MINUTE_0, 0, 0, 3, FIRST_HALF
		},
		[hour_motor] = {
			NULL, HOUR0_0, 0, 0, 2, FIRST_HALF
		},
		[activity_motor] = {
			NULL, ACTIVITY_0, 0, 0, 1, FIRST_HALF
		},
		[date_motor] = {
			NULL, DAY_1, 0, 0, 3, FIRST_HALF
		},
		[battery_week_motor] = {
			NULL, BAT_PECENT_0, 0, 0, BAT_INTERVAL_STEP, FIRST_HALF
		},
		[notify_motor] = {
			NULL, NOTIFY_NONE, 0, 0, 1, FIRST_HALF
		},
	 },
	.run_motor = NULL,
	.run_motor_num = max_motor,
	.run_step_num = {0,0,0,0,0,0},
	.run_interval_ms = 100,
	.run_direction  = pos,
	.bat_week_dst = BAT_PECENT_0,
	.time_adjust_mode = false,
};
            
motor_run_t motor_run = {
    .motor_range = {
        [minute_motor]          = {0, 182},
        [hour_motor]            = {0, 122},
        [activity_motor]        = {0, ACTIVITY_100},
        [date_motor]            = {0, 92},
        [battery_week_motor]    = {0, 62},
        [notify_motor]          = {0, NOTIFY_DONE},
    },
    .motor_contiune = {0,0,0,0,0,0},
    .motor_offset = {0,0,0,0,0,0},
    .motor_flag = {0,0,0,0,0,0},
    .motor_dirc = {0,0,0,0,0,0},
    .skip_total = {0,0,0,0,0,1},
    .skip_cnt = {0,0,0,0,0,0},
    .step_cnt = {0,0,0,0,0,0},
    .step_unit = {3,2,1,3,2,1},
    .random_val = {MINUTE_0,MINUTE_0,ACTIVITY_0,DAY_31,SUNDAY,NOTIFY_DONE},
    .calc_dirc = {1,1,1,1,1,1},
    .motor_state = FIRST_HALF,
    .timer_interval = 0,
    .run_cnt = 0,
    .test_mode = 0,
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

s16 motor_hour_to_position(void)
{
    u8 hour = adapter_ctrl.motor_dst[hour_motor];
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
        motor_run.motor_flag[hour_motor] = 1;
	} else {
		motor_manager.motor_status[hour_motor].run_flag = 0;
        motor_run.motor_flag[hour_motor] = 0;
	}
	return 0;
}

s16 motor_minute_to_position(void)
{
	/*minute dst pos config*/
	if((MINUTE_59 == motor_manager.motor_status[minute_motor].cur_pos) &&
		(MINUTE_0 == adapter_ctrl.motor_dst[minute_motor])) {
		motor_manager.motor_status[minute_motor].dst_pos = MINUTE_60;
	}else {
		motor_manager.motor_status[minute_motor].dst_pos = adapter_ctrl.motor_dst[minute_motor];
	}

	/*minute cur pos config*/
	if(MINUTE_60 == motor_manager.motor_status[minute_motor].cur_pos) {
		motor_manager.motor_status[minute_motor].cur_pos = MINUTE_0;
	}
	
	if(motor_manager.motor_status[minute_motor].dst_pos != 
	   motor_manager.motor_status[minute_motor].cur_pos) {
		motor_manager.motor_status[minute_motor].run_flag = 1;
        motor_run.motor_flag[minute_motor] = 1;
	} else {
		motor_manager.motor_status[minute_motor].run_flag = 0;
        motor_run.motor_flag[minute_motor] = 0;
	}
	return 0;
}

s16 motor_date_to_position(void)
{
	motor_manager.motor_status[date_motor].dst_pos = adapter_ctrl.motor_dst[date_motor];
	if(motor_manager.motor_status[date_motor].dst_pos != 
	   motor_manager.motor_status[date_motor].cur_pos) {
		motor_manager.motor_status[date_motor].run_flag = 1;
        motor_run.motor_flag[date_motor] = 1;
	} else {
		motor_manager.motor_status[date_motor].run_flag = 0;
        motor_run.motor_flag[date_motor] = 0;
	}
	return 0;
}

s16 motor_notify_to_position(void)
{
	motor_manager.motor_status[notify_motor].dst_pos = adapter_ctrl.motor_dst[notify_motor];
	if(motor_manager.motor_status[notify_motor].dst_pos != 
	   motor_manager.motor_status[notify_motor].cur_pos) {
		motor_manager.motor_status[notify_motor].run_flag = 1;
        motor_run.motor_flag[notify_motor] = 1;
	} else {
		motor_manager.motor_status[notify_motor].run_flag = 0;
        motor_run.motor_flag[notify_motor] = 0;
	}
	return 0;
}

static void motor_battery_week_change(u16 id)
{
	if((BAT_PECENT_100 == motor_manager.motor_status[battery_week_motor].cur_pos) &&
		(0 == motor_run.motor_flag[battery_week_motor])) {
		
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
        motor_run.motor_flag[battery_week_motor] = 1;
	}else {
		timer_event(100, motor_battery_week_change);
	}
}

s16 motor_battery_week_to_position(void)
{
	if((motor_manager.motor_status[battery_week_motor].cur_pos > BAT_PECENT_100) &&
		(adapter_ctrl.motor_dst[battery_week_motor] <= BAT_PECENT_100)) {
		motor_manager.motor_status[battery_week_motor].dst_pos = BAT_PECENT_100;
		motor_manager.bat_week_dst = adapter_ctrl.motor_dst[battery_week_motor];
		timer_event(motor_manager.run_interval_ms, motor_battery_week_change);
	}else if((motor_manager.motor_status[battery_week_motor].cur_pos < BAT_PECENT_100) &&
		(adapter_ctrl.motor_dst[battery_week_motor] >= BAT_PECENT_100)) {
		motor_manager.motor_status[battery_week_motor].dst_pos = BAT_PECENT_100;
		motor_manager.bat_week_dst = adapter_ctrl.motor_dst[battery_week_motor];
		timer_event(motor_manager.run_interval_ms, motor_battery_week_change);
	}else if(BAT_PECENT_100 == motor_manager.motor_status[battery_week_motor].cur_pos) {
		motor_manager.motor_status[battery_week_motor].dst_pos = adapter_ctrl.motor_dst[battery_week_motor];
		if(motor_manager.motor_status[battery_week_motor].dst_pos > BAT_PECENT_100) {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = BAT_INTERVAL_STEP;
		}else {
			motor_manager.motor_status[battery_week_motor].unit_interval_step = WEEK_INTERVAL_STEP;
		}
	}else {
		motor_manager.motor_status[battery_week_motor].dst_pos = adapter_ctrl.motor_dst[battery_week_motor];
	}

	if(motor_manager.motor_status[battery_week_motor].dst_pos != 
	   motor_manager.motor_status[battery_week_motor].cur_pos) {
		motor_manager.motor_status[battery_week_motor].run_flag = 1;
        motor_run.motor_flag[battery_week_motor] = 1;
	} else {
		motor_manager.motor_status[battery_week_motor].run_flag = 0;
        motor_run.motor_flag[battery_week_motor] = 0;
	}
	return 0;
}

s16 motor_activity_to_position(void)
{
	motor_manager.motor_status[activity_motor].dst_pos = adapter_ctrl.motor_dst[activity_motor];
	if(motor_manager.motor_status[activity_motor].dst_pos != 
	   motor_manager.motor_status[activity_motor].cur_pos) {
		motor_manager.motor_status[activity_motor].run_flag = 1;
        motor_run.motor_flag[activity_motor] = 1;
	} else {
		motor_manager.motor_status[activity_motor].run_flag = 0;
        motor_run.motor_flag[activity_motor] = 0;
	}
	return 0;
}

static u8 motor_check_continue(u8 motor_num)
{
	motor_manager.run_step_num[motor_num]++;
	if(motor_manager.run_step_num[motor_num] >= motor_manager.motor_status[motor_num].unit_interval_step) {
		motor_manager.run_step_num[motor_num] = 0;

        //motor_check_direction(motor_num);
    	if(motor_manager.motor_status[motor_num].dst_pos > motor_manager.motor_status[motor_num].cur_pos) {
    		motor_run.motor_dirc[motor_num] = pos;
			motor_manager.motor_status[motor_num].cur_pos++;
    	}else if(motor_manager.motor_status[motor_num].dst_pos < motor_manager.motor_status[motor_num].cur_pos) {
    		motor_run.motor_dirc[motor_num] = neg;
            if(motor_manager.motor_status[motor_num].cur_pos == 0) {
                motor_manager.motor_status[motor_num].cur_pos = 1;
            }
			motor_manager.motor_status[motor_num].cur_pos--;
    	} else {
            motor_run.motor_flag[motor_num] = 0;
            motor_run.motor_contiune[motor_num] = 0;
            motor_run.motor_dirc[motor_num] = none;
            return 0;
    	}
	}
    return 1;
}

void motor_check_run(u16 id)
{
    u8 i = 0;
    u16 continue_flag = 0;
    
	switch(motor_run.motor_state) {
		case FIRST_HALF:
            for(i = 0; i < max_motor; i++) {
                if(motor_run.motor_flag[i] == 1) {
                    if(motor_run.motor_dirc[i] == pos)
                        motor_manager.motor_status[i].motor_ptr->motor_positive_first_half(NULL);
                    else if(motor_run.motor_dirc[i] == neg)
                        motor_manager.motor_status[i].motor_ptr->motor_negtive_first_half(NULL);
                }
            }
			motor_run.motor_state = RECOVER;
			motor_manager.drv->timer->timer_start(motor_run.timer_interval, motor_check_run);
			break;
		case RECOVER:
            for(i = 0; i < max_motor; i++) {
                if(motor_run.motor_flag[i] == 1) {
                    motor_manager.motor_status[i].motor_ptr->motor_stop(NULL);
                    if(motor_run.motor_contiune[i] == 1) {
                        continue_flag += motor_check_continue(i);
                    } else {
                        motor_run.motor_flag[i] = 0;
                    }
                }
            }
			motor_run.motor_state = SECOND_HALF;
            if(continue_flag > 0) {
                motor_manager.drv->timer->timer_start(motor_run.timer_interval, motor_check_run);
            }
			break;
		case SECOND_HALF:
            for(i = 0; i < max_motor; i++) {
                if(motor_run.motor_flag[i] == 1) {
                    if(motor_run.motor_dirc[i] == pos)
                        motor_manager.motor_status[i].motor_ptr->motor_positive_second_half(NULL);
                    else if(motor_run.motor_dirc[i] == neg)
                        motor_manager.motor_status[i].motor_ptr->motor_negtive_second_half(NULL);
                }
            }
			motor_run.motor_state = STOP;
			motor_manager.drv->timer->timer_start(motor_run.timer_interval, motor_check_run);
			break;
		case STOP:
            for(i = 0; i < max_motor; i++) {
                if(motor_run.motor_flag[i] == 1) {
                    motor_manager.motor_status[i].motor_ptr->motor_stop(NULL);
                    if(motor_run.motor_contiune[i] == 1) {
                        continue_flag += motor_check_continue(i);
                    } else {
                        motor_run.motor_flag[i] = 0;
                    }
                }
            }
			motor_run.motor_state = FIRST_HALF;
            if(continue_flag > 0) {
                motor_manager.drv->timer->timer_start(motor_run.timer_interval, motor_check_run);
            }
			break;
		default :
			break;
	}
}
void motor_run_one_unit(u8 timer_intervel)
{
	u8 i = 0;

    motor_run.timer_interval = timer_intervel;
    motor_run.run_cnt = 0;
	for(i = 0; i < max_motor; i++) {
		if(motor_run.motor_flag[i] == 1) {
            //motor_check_direction(i);
        	if(motor_manager.motor_status[i].dst_pos > motor_manager.motor_status[i].cur_pos) {
        		motor_run.motor_dirc[i] = pos;
        	}else if(motor_manager.motor_status[i].dst_pos < motor_manager.motor_status[i].cur_pos) {
        		motor_run.motor_dirc[i] = neg;
        	} else {
                motor_run.motor_dirc[i] = none;
                motor_run.motor_flag[i] = 0;
        	}
		}
        if(motor_run.motor_flag[i] != 0) {
            motor_run.motor_contiune[i] = 1;
            motor_run.run_cnt++;
    	    motor_manager.run_step_num[i] = 0;
        }
	}

    if(motor_run.run_cnt > 0) {
        motor_manager.drv->timer->timer_start(5, motor_check_run);
    }
}

void motor_run_one_step(u8 motor_num, u8 direction)
{
    MemSet(motor_run.motor_flag, 0, max_motor*sizeof(u8));
    motor_run.motor_flag[motor_num] = 1;
    motor_run.motor_dirc[motor_num] = direction;
    motor_run.timer_interval = 10;
    motor_manager.drv->timer->timer_start(1, motor_check_run);
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
//    u8 i = 0;
    
	motor_manager.drv = get_driver();
	motor_manager.motor_status[hour_motor].motor_ptr = motor_manager.drv->motor_hour;
	motor_manager.motor_status[minute_motor].motor_ptr = motor_manager.drv->motor_minute;
	motor_manager.motor_status[activity_motor].motor_ptr = motor_manager.drv->motor_activity;
	motor_manager.motor_status[date_motor].motor_ptr = motor_manager.drv->motor_date;
	motor_manager.motor_status[battery_week_motor].motor_ptr = motor_manager.drv->motor_battery_week;
	motor_manager.motor_status[notify_motor].motor_ptr = motor_manager.drv->motor_notify;

	//motor_manager.drv->timer->timer_start(motor_manager.run_interval_ms, motor_run_handler2);
    system_post_reboot_handler();
	return 0;
}

u16 motor_check_idle(void)
{
    u16 i = 0;
    
    for(i = 0; i < max_motor; i++) {
        if(motor_run.motor_flag[i] == 1) {
            return 1;
        }
    }
    return 0;
}

