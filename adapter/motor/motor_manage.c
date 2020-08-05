#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <time.h>
#include <mem.h>
#include "../adapter.h"

#define BAT_INTERVAL_STEP 2
#define WEEK_INTERVAL_STEP 5

typedef s16 (*motor_cb_handler)(void *args);

enum {
	FIRST_HALF,
	SECOND_HALF,
	RECOVER,
	STOP,
};
    
typedef struct {
    motor_cb_handler motor_run_half[2][2];
    motor_cb_handler motor_stop;
} motor_cb_ctrl_t;

typedef struct {
	/*record current position*/
	u8 cur_pos;
	/*record the motor dst position*/
	u8 dst_pos;
	/*run flag*/
	u8 run_flag;
	/*unit interval steps*/
	u8 unit_interval_step;
	/*run step state*/
}motor_run_status_t;

typedef struct {
	motor_run_status_t motor_status[max_motor];
    motor_cb_ctrl_t cb[max_motor];
    
	/*record step run state and run step interval*/
	u8 run_step_num[max_motor];
	u8 run_interval_ms;
	u8 bat_week_dst;
}motor_manager_t;

static motor_manager_t motor_manager = {
	.motor_status = {
		[minute_motor] = {MINUTE_0, 0, 0, 3},
		[hour_motor] = {HOUR0_0, 0, 0, 2},
		[activity_motor] = {ACTIVITY_0, 0, 0, 1},
		[date_motor] = {DAY_1, 0, 0, 3},
		[battery_week_motor] = {BAT_PECENT_0, 0, 0, BAT_INTERVAL_STEP},
		[notify_motor] = {NOTIFY_NONE, 0, 0, 1},
	 },
	.run_step_num = {0,0,0,0,0,0},
	.run_interval_ms = 100,
	.bat_week_dst = BAT_PECENT_0,
};
            
motor_run_t motor_run = {
    .run_range = {
        [minute_motor]          = {0, 182},
        [hour_motor]            = {0, 122},
        [activity_motor]        = {0, ACTIVITY_100},
        [date_motor]            = {0, 92},
        [battery_week_motor]    = {0, 62},
        [notify_motor]          = {0, NOTIFY_DONE},
    },
    .run_next = {0,0,0,0,0,0},
    .motor_runnig = {0,0,0,0,0,0},
    .run_direc = {none,none,none,none,none,none},
    .run_state_self = {FIRST_HALF,FIRST_HALF,FIRST_HALF,FIRST_HALF,FIRST_HALF,FIRST_HALF},
    .run_main_self = FIRST_HALF,
    .skip_total = {0,0,0,0,0,1},
    .skip_cnt = {0,0,0,0,0,0},
    .step_cnt = {0,0,0,0,0,0},
    .calc_dirc = {1,1,1,1,1,1},
    .timer_interval = 0,
    .run_test_mode = 0,
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
	/*hour dst position configuration*/
    u8 hour_pos = adapter_ctrl.motor_dst[hour_motor];
	u8 hour_offset = motor_manager.motor_status[minute_motor].dst_pos/12;

	if(hour_pos >= 12) {
		hour_pos -= 12;
	}

	motor_manager.motor_status[hour_motor].dst_pos = hour_list[hour_pos] + hour_offset;
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
        motor_run.motor_runnig[hour_motor] = 1;
	} else {
		motor_manager.motor_status[hour_motor].run_flag = 0;
        motor_run.motor_runnig[hour_motor] = 0;
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
        motor_run.motor_runnig[minute_motor] = 1;
	} else {
		motor_manager.motor_status[minute_motor].run_flag = 0;
        motor_run.motor_runnig[minute_motor] = 0;
	}
	return 0;
}

s16 motor_date_to_position(void)
{
	motor_manager.motor_status[date_motor].dst_pos = adapter_ctrl.motor_dst[date_motor];
	if(motor_manager.motor_status[date_motor].dst_pos != 
	   motor_manager.motor_status[date_motor].cur_pos) {
		motor_manager.motor_status[date_motor].run_flag = 1;
        motor_run.motor_runnig[date_motor] = 1;
	} else {
		motor_manager.motor_status[date_motor].run_flag = 0;
        motor_run.motor_runnig[date_motor] = 0;
	}
	return 0;
}

s16 motor_notify_to_position(void)
{
	motor_manager.motor_status[notify_motor].dst_pos = adapter_ctrl.motor_dst[notify_motor];
	if(motor_manager.motor_status[notify_motor].dst_pos != 
	   motor_manager.motor_status[notify_motor].cur_pos) {
		motor_manager.motor_status[notify_motor].run_flag = 1;
        motor_run.motor_runnig[notify_motor] = 1;
	} else {
		motor_manager.motor_status[notify_motor].run_flag = 0;
        motor_run.motor_runnig[notify_motor] = 0;
	}
	return 0;
}

static void motor_battery_week_change(u16 id)
{
	if((BAT_PECENT_100 == motor_manager.motor_status[battery_week_motor].cur_pos) &&
		(0 == motor_run.motor_runnig[battery_week_motor])) {
		
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
        motor_run.motor_runnig[battery_week_motor] = 1;
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
        motor_run.motor_runnig[battery_week_motor] = 1;
	} else {
		motor_manager.motor_status[battery_week_motor].run_flag = 0;
        motor_run.motor_runnig[battery_week_motor] = 0;
	}
	return 0;
}

s16 motor_activity_to_position(void)
{
	motor_manager.motor_status[activity_motor].dst_pos = adapter_ctrl.motor_dst[activity_motor];
	if(motor_manager.motor_status[activity_motor].dst_pos != 
	   motor_manager.motor_status[activity_motor].cur_pos) {
		motor_manager.motor_status[activity_motor].run_flag = 1;
        motor_run.motor_runnig[activity_motor] = 1;
	} else {
		motor_manager.motor_status[activity_motor].run_flag = 0;
        motor_run.motor_runnig[activity_motor] = 0;
	}
	return 0;
}

enum {
    MOTOR_RUN_OVER,
    MOTOR_RUN_NEXT_STEP,
    MOTOR_RUN_NEXT_UNIT,
};
static u8 motor_check_continue(u8 motor_num)
{
	motor_manager.run_step_num[motor_num]++;
	if(motor_manager.run_step_num[motor_num] >= motor_manager.motor_status[motor_num].unit_interval_step) {
		motor_manager.run_step_num[motor_num] = 0;

        //motor_check_direction(motor_num);
    	if(motor_manager.motor_status[motor_num].dst_pos > motor_manager.motor_status[motor_num].cur_pos) {
    		motor_run.run_direc[motor_num] = pos;
			motor_manager.motor_status[motor_num].cur_pos++;
    	}else if(motor_manager.motor_status[motor_num].dst_pos < motor_manager.motor_status[motor_num].cur_pos) {
    		motor_run.run_direc[motor_num] = neg;
            if(motor_manager.motor_status[motor_num].cur_pos == 0) {
                motor_manager.motor_status[motor_num].cur_pos = 1;
            }
			motor_manager.motor_status[motor_num].cur_pos--;
    	}
        if(motor_manager.motor_status[motor_num].dst_pos == motor_manager.motor_status[motor_num].cur_pos) {
            motor_run.motor_runnig[motor_num] = 0;
            motor_run.run_next[motor_num] = 0;
            motor_run.run_direc[motor_num] = none;
            return MOTOR_RUN_OVER;
    	}
        return MOTOR_RUN_NEXT_UNIT;
	}
    return MOTOR_RUN_NEXT_STEP;
}

void motor_check_run(u16 id)
{
    u8 i = 0;
    u16 run_continue = 0;

    if((motor_run.run_main_self == FIRST_HALF) || (motor_run.run_main_self == SECOND_HALF)) {
        for(i = 0; i < max_motor; i++) {
            if((motor_run.motor_runnig[i] == 0) || (motor_run.run_state_self[i] != motor_run.run_main_self)) {
                continue;
            }
            motor_manager.cb[i].motor_run_half[motor_run.run_state_self[i]][motor_run.run_direc[i]](NULL);
            motor_run.run_state_self[i] = (motor_run.run_state_self[i]==FIRST_HALF)?RECOVER:STOP;
        }
		motor_run.run_main_self = (motor_run.run_main_self==FIRST_HALF)?RECOVER:STOP;
		timer_event(motor_run.timer_interval, motor_check_run);
    }else if((motor_run.run_main_self == RECOVER) || (motor_run.run_main_self == STOP)) {
        for(i = 0; i < max_motor; i++) {
            if(motor_run.motor_runnig[i] == 0) {
                continue;
            }
            motor_manager.cb[i].motor_stop(NULL);
            motor_run.run_state_self[i] = (motor_run.run_state_self[i]==RECOVER)?SECOND_HALF:FIRST_HALF;
            if(motor_run.run_next[i] == 1) {
                run_continue += motor_check_continue(i);
            } else {
                motor_run.motor_runnig[i] = 0;
            }
        }
		motor_run.run_main_self = (motor_run.run_main_self==RECOVER)?SECOND_HALF:FIRST_HALF;
        if(run_continue > 0) {
            timer_event(motor_run.timer_interval, motor_check_run);
        }
    }

    #if 0
	switch(motor_run.motor_state) {
		case FIRST_HALF:
		case SECOND_HALF:
            for(i = 0; i < max_motor; i++) {
                if(motor_run.motor_flag[i] == 0) {
                    continue;
                }
                motor_manager.cb[i].motor_run_half[motor_run.motor_state][motor_run.run_direc[i]](NULL);
            }
			motor_run.motor_state = (motor_run.motor_state==FIRST_HALF)?RECOVER:STOP;
			timer_event(motor_run.timer_interval, motor_check_run);
			break;
		case RECOVER:
		case STOP:
            for(i = 0; i < max_motor; i++) {
                if(motor_run.motor_flag[i] == 0) {
                    continue;
                }
                motor_manager.cb[i].motor_stop(NULL);
                if(motor_run.run_next[i] == 1) {
                    continue_flag += motor_check_continue(i);
                } else {
                    motor_run.motor_flag[i] = 0;
                }
            }
			//motor_run.motor_state = FIRST_HALF;
			motor_run.motor_state = (motor_run.motor_state==RECOVER)?SECOND_HALF:FIRST_HALF;
            if(continue_flag > 0) {
                timer_event(motor_run.timer_interval, motor_check_run);
            }
			break;
		default :
			break;
	}
    #endif
}
void motor_run_one_unit(u8 timer_intervel)
{
	u8 i = 0;
    u8 run_cnt = 0;

    motor_run.timer_interval = timer_intervel;
    run_cnt = 0;
	for(i = 0; i < max_motor; i++) {
		if(motor_run.motor_runnig[i] == 1) {
            //motor_check_direction(i);
        	if(motor_manager.motor_status[i].dst_pos > motor_manager.motor_status[i].cur_pos) {
        		motor_run.run_direc[i] = pos;
        	}else if(motor_manager.motor_status[i].dst_pos < motor_manager.motor_status[i].cur_pos) {
        		motor_run.run_direc[i] = neg;
        	} else {
                motor_run.run_direc[i] = none;
                motor_run.motor_runnig[i] = 0;
        	}
		}
        if(motor_run.motor_runnig[i] != 0) {
            motor_run.run_next[i] = 1;
            run_cnt++;
    	    motor_manager.run_step_num[i] = 0;
        }
	}

    if(run_cnt > 0) {
        timer_event(5, motor_check_run);
    }
}

void motor_run_one_step(u8 motor_num, u8 direction)
{
    MemSet(motor_run.motor_runnig, 0, max_motor*sizeof(u8));
    motor_run.motor_runnig[motor_num] = 1;
    motor_run.run_direc[motor_num] = direction;
    motor_run.timer_interval = 10;
    timer_event(1, motor_check_run);
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
	driver_t *driver = get_driver();

    motor_cb_ctrl_t motor_cb_ctrl[max_motor] = {
        [hour_motor] = {
            .motor_run_half = {
                [FIRST_HALF] = {driver->motor_hour->motor_positive_first_half, driver->motor_hour->motor_negtive_first_half},
                [SECOND_HALF] = {driver->motor_hour->motor_positive_second_half, driver->motor_hour->motor_negtive_second_half},
            },
            .motor_stop = driver->motor_hour->motor_stop,
        },
        [minute_motor] = {
            .motor_run_half = {
                [FIRST_HALF] = {driver->motor_minute->motor_positive_first_half, driver->motor_minute->motor_negtive_first_half},
                [SECOND_HALF] = {driver->motor_minute->motor_positive_second_half, driver->motor_minute->motor_negtive_second_half},
            },
            .motor_stop = driver->motor_minute->motor_stop,
        },
        [activity_motor] = {
            .motor_run_half = {
                [FIRST_HALF] = {driver->motor_activity->motor_positive_first_half, driver->motor_activity->motor_negtive_first_half},
                [SECOND_HALF] = {driver->motor_activity->motor_positive_second_half, driver->motor_activity->motor_negtive_second_half},
            },
            .motor_stop = driver->motor_activity->motor_stop,
        },
        [date_motor] = {
            .motor_run_half = {
                [FIRST_HALF] = {driver->motor_date->motor_positive_first_half, driver->motor_date->motor_negtive_first_half},
                [SECOND_HALF] = {driver->motor_date->motor_positive_second_half, driver->motor_date->motor_negtive_second_half},
            },
            .motor_stop = driver->motor_date->motor_stop,
        },
        [battery_week_motor] = {
            .motor_run_half = {
                [FIRST_HALF] = {driver->motor_battery_week->motor_positive_first_half, driver->motor_battery_week->motor_negtive_first_half},
                [SECOND_HALF] = {driver->motor_battery_week->motor_positive_second_half, driver->motor_battery_week->motor_negtive_second_half},
            },
            .motor_stop = driver->motor_battery_week->motor_stop,
        },
        [notify_motor] = {
            .motor_run_half = {
                [FIRST_HALF] = {driver->motor_notify->motor_positive_first_half, driver->motor_notify->motor_negtive_first_half},
                [SECOND_HALF] = {driver->motor_notify->motor_positive_second_half, driver->motor_notify->motor_negtive_second_half},
            },
            .motor_stop = driver->motor_notify->motor_stop,
        },
    };
    MemCopy(&motor_manager.cb, &motor_cb_ctrl, max_motor*sizeof(motor_cb_ctrl_t));

    system_post_reboot_handler();
	return 0;
}

u16 motor_check_idle(void)
{
    u16 i = 0;
    
    for(i = 0; i < max_motor; i++) {
        if(motor_run.motor_runnig[i] == 1) {
            return 1;
        }
    }
    return 0;
}

