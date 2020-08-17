#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <time.h>
#include <mem.h>
#include "../adapter.h"

#define BAT_INTERVAL_STEP 2
#define WEEK_INTERVAL_STEP 5

enum {
	FIRST_HALF,
	SECOND_HALF,
	RUN,
	STOP,
};

enum {
    MOTOR_RUN_OVER,
    MOTOR_RUN_NEXT_STEP,
    MOTOR_RUN_NEXT_UNIT,
};

motor_manager_t motor_manager = {
	.status = {
     // motor                     cur               dst flag    unit_step             steps direc range              
		[minute_motor]          = {MINUTE_0,        0,  0,      3,                    0,    none, {0, 180}},
		[hour_motor]            = {HOUR0_0,         0,  0,      2,                    0,    none, {0, 120}},
		[activity_motor]        = {ACTIVITY_0,      0,  0,      1,                    0,    none, {0, ACTIVITY_100}},
		[date_motor]            = {DAY_1,           0,  0,      3,                    0,    none, {0, 90}},
		[battery_week_motor]    = {BAT_PECENT_0,    0,  0,      BAT_INTERVAL_STEP,    0,    none, {0, 60}},
		[notify_motor]          = {NOTIFY_NONE,     0,  0,      1,                    0,    none, {0, NOTIFY_DONE}},
	 },
    .run_next = {0,0,0,0,0,0},
    .run_state_self = {FIRST_HALF,FIRST_HALF,FIRST_HALF,FIRST_HALF,FIRST_HALF,FIRST_HALF},
    .run_state_main = FIRST_HALF,
    .skip_total = {0,0,1,0,0,1},
    .skip_cnt = {0,0,0,0,0,0},
    .step_cnt = {0,0,0,0,0,0},
    .calc_dirc = {1,1,1,1,1,1},
	.bat_week_dst = BAT_PECENT_0,
	.run_interval_ms = 100,
    .timer_interval = 0,
    .run_test_mode = 0,
    .motor_running = 0,
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

s16 motor_hour_to_position(u8 dst_pos)
{
    adapter_ctrl.motor_dst[hour_motor] = dst_pos;
    
	/*hour dst position configuration*/
	u8 hour_offset = motor_manager.status[minute_motor].dst_pos/12;

	if(dst_pos >= 12) {
		dst_pos -= 12;
	}

	motor_manager.status[hour_motor].dst_pos = hour_list[dst_pos] + hour_offset;
	if((HOUR11_8 == motor_manager.status[hour_motor].cur_pos) &&
	   (HOUR0_0 == motor_manager.status[hour_motor].dst_pos) ){
		motor_manager.status[hour_motor].dst_pos = HOUR12_0;
	}

	/*hour cur pos configuration*/
	if(HOUR12_0 == motor_manager.status[hour_motor].cur_pos) {
		motor_manager.status[hour_motor].cur_pos = HOUR0_0;
	}
	   
	if(motor_manager.status[hour_motor].dst_pos != 
	   motor_manager.status[hour_motor].cur_pos) {
		motor_manager.status[hour_motor].run_flag = 1;
	} else {
		motor_manager.status[hour_motor].run_flag = 0;
	}
	return motor_manager.status[hour_motor].run_flag;
}

s16 motor_minute_to_position(u8 dst_pos)
{
    adapter_ctrl.motor_dst[minute_motor] = dst_pos;
    
	/*minute dst pos config*/
	if((MINUTE_59 == motor_manager.status[minute_motor].cur_pos) && (MINUTE_0 == dst_pos)) {
		motor_manager.status[minute_motor].dst_pos = MINUTE_60;
	}else {
		motor_manager.status[minute_motor].dst_pos = dst_pos;
	}

	/*minute cur pos config*/
	if(MINUTE_60 == motor_manager.status[minute_motor].cur_pos) {
		motor_manager.status[minute_motor].cur_pos = MINUTE_0;
	}
	
	if(motor_manager.status[minute_motor].dst_pos != 
	   motor_manager.status[minute_motor].cur_pos) {
		motor_manager.status[minute_motor].run_flag = 1;
	} else {
		motor_manager.status[minute_motor].run_flag = 0;
	}
	return motor_manager.status[minute_motor].run_flag;
}

s16 motor_date_to_position(u8 dst_pos)
{
    adapter_ctrl.motor_dst[date_motor] = dst_pos;
    
	motor_manager.status[date_motor].dst_pos = dst_pos;
	if(motor_manager.status[date_motor].dst_pos != 
	   motor_manager.status[date_motor].cur_pos) {
		motor_manager.status[date_motor].run_flag = 1;
	} else {
		motor_manager.status[date_motor].run_flag = 0;
	}
	return motor_manager.status[date_motor].run_flag;
}

s16 motor_notify_to_position(u8 dst_pos)
{
    adapter_ctrl.motor_dst[notify_motor] = dst_pos;
    
	motor_manager.status[notify_motor].dst_pos = dst_pos;
	if(motor_manager.status[notify_motor].dst_pos != 
	   motor_manager.status[notify_motor].cur_pos) {
		motor_manager.status[notify_motor].run_flag = 1;
	} else {
		motor_manager.status[notify_motor].run_flag = 0;
	}
	return motor_manager.status[notify_motor].run_flag;
}

static void motor_battery_week_change(u16 id)
{
	if((BAT_PECENT_100 == motor_manager.status[battery_week_motor].cur_pos) &&
		(0 == motor_manager.status[battery_week_motor].run_flag)) {
		
		if(BAT_PECENT_100 == motor_manager.bat_week_dst) {
			return;
		}
		
		motor_manager.status[battery_week_motor].dst_pos = motor_manager.bat_week_dst;
		if(motor_manager.bat_week_dst > BAT_PECENT_100) {
			motor_manager.status[battery_week_motor].unit_interval_step = BAT_INTERVAL_STEP;
		}else {
			motor_manager.status[battery_week_motor].unit_interval_step = WEEK_INTERVAL_STEP;
		}
		
		motor_manager.status[battery_week_motor].run_flag = 1;
	}else {
		timer_event(100, motor_battery_week_change);
	}
}

s16 motor_battery_week_to_position(u8 dst_pos)
{
    adapter_ctrl.motor_dst[battery_week_motor] = dst_pos;
    
	if((motor_manager.status[battery_week_motor].cur_pos > BAT_PECENT_100) && (dst_pos <= BAT_PECENT_100)) {
		motor_manager.status[battery_week_motor].dst_pos = BAT_PECENT_100;
		motor_manager.bat_week_dst = dst_pos;
		timer_event(motor_manager.run_interval_ms, motor_battery_week_change);
	}else if((motor_manager.status[battery_week_motor].cur_pos < BAT_PECENT_100) && (dst_pos >= BAT_PECENT_100)) {
		motor_manager.status[battery_week_motor].dst_pos = BAT_PECENT_100;
		motor_manager.bat_week_dst = dst_pos;
		timer_event(motor_manager.run_interval_ms, motor_battery_week_change);
	}else if(BAT_PECENT_100 == motor_manager.status[battery_week_motor].cur_pos) {
		motor_manager.status[battery_week_motor].dst_pos = dst_pos;
		if(motor_manager.status[battery_week_motor].dst_pos > BAT_PECENT_100) {
			motor_manager.status[battery_week_motor].unit_interval_step = BAT_INTERVAL_STEP;
		}else {
			motor_manager.status[battery_week_motor].unit_interval_step = WEEK_INTERVAL_STEP;
		}
	}else {
		motor_manager.status[battery_week_motor].dst_pos = dst_pos;
	}

	if(motor_manager.status[battery_week_motor].dst_pos != 
	   motor_manager.status[battery_week_motor].cur_pos) {
		motor_manager.status[battery_week_motor].run_flag = 1;
	} else {
		motor_manager.status[battery_week_motor].run_flag = 0;
	}
	return motor_manager.status[battery_week_motor].run_flag;
}

s16 motor_activity_to_position(u8 dst_pos)
{
    adapter_ctrl.motor_dst[activity_motor] = dst_pos;
    
	motor_manager.status[activity_motor].dst_pos = dst_pos;
	if(motor_manager.status[activity_motor].dst_pos != 
	   motor_manager.status[activity_motor].cur_pos) {
		motor_manager.status[activity_motor].run_flag = 1;
	} else {
		motor_manager.status[activity_motor].run_flag = 0;
	}
	return motor_manager.status[activity_motor].run_flag;
}

static u8 motor_check_continue(u8 motor_num)
{
    if(motor_queue.cur_user == QUEUE_USER_MOTOR_TRIG) {
        motor_manager.motor_run_info.step++;
    }
	motor_manager.status[motor_num].unit_step_num++;
	if(motor_manager.status[motor_num].unit_step_num >= motor_manager.status[motor_num].unit_interval_step) {
		motor_manager.status[motor_num].unit_step_num = 0;

        //motor_check_direction(motor_num);
    	if(motor_manager.status[motor_num].dst_pos > motor_manager.status[motor_num].cur_pos) {
    		motor_manager.status[motor_num].run_direc = pos;
			motor_manager.status[motor_num].cur_pos++;
    	}else if(motor_manager.status[motor_num].dst_pos < motor_manager.status[motor_num].cur_pos) {
    		motor_manager.status[motor_num].run_direc = neg;
            if(motor_manager.status[motor_num].cur_pos == 0) {
                motor_manager.status[motor_num].cur_pos = 1;
            }
			motor_manager.status[motor_num].cur_pos--;
    	}
        if(motor_manager.status[motor_num].dst_pos == motor_manager.status[motor_num].cur_pos) {
            motor_manager.status[motor_num].run_flag = 0;
            motor_manager.run_next[motor_num] = 0;
            motor_manager.status[motor_num].run_direc = none;
            if(motor_queue.cur_user == QUEUE_USER_MOTOR_TRIG) {
                motor_manager.motor_run_info.index = motor_queue.cur_index;
                send_motor_run_info();
                MemSet(&motor_manager.motor_run_info, 0, sizeof(motor_run_info_t));   
                motor_manager.motor_run_info.stage++;
            }
            return MOTOR_RUN_OVER;
    	}
        return MOTOR_RUN_NEXT_UNIT;
	}
    return MOTOR_RUN_NEXT_STEP;
}
#if 1
void motor_check_run(u16 id)
{
    u8 i = 0;
    u8 run_flag = 0;

    if(motor_manager.run_state_main == FIRST_HALF) {
		motor_manager.run_state_main = SECOND_HALF;
        for(i = 0; i < max_motor; i++) {
            if(motor_manager.status[i].run_flag == 0) {
                continue;
            }
            motor_manager.cb[i].motor_run_half[motor_manager.run_state_self[i]][motor_manager.status[i].run_direc](NULL);
            motor_manager.run_state_self[i] = (motor_manager.run_state_self[i]==FIRST_HALF)?SECOND_HALF:FIRST_HALF;
            if(motor_manager.run_next[i] == 1) {
                motor_check_continue(i);
            } else {
                //motor_manager.cb[i].motor_stop(NULL);
                motor_manager.status[i].run_flag = 0;
            }
        }
		timer_event(motor_manager.timer_interval, motor_check_run);
    } else if(motor_manager.run_state_main == SECOND_HALF) {
		motor_manager.run_state_main = FIRST_HALF;
        for(i = 0; i < max_motor; i++) {
            motor_manager.cb[i].motor_stop(NULL);
            if(motor_manager.status[i].run_flag == 1) {
                run_flag++;
            }
        }
        if(run_flag == 0) {
            motor_manager.motor_running = 0;
            if(motor_manager.queue_cb != NULL) {
                motor_manager.queue_cb(NULL);
            }
            return;
        }
        timer_event(motor_manager.timer_interval, motor_check_run);
    }
}
#else
void motor_check_run(u16 id)
{
    u8 i = 0;
    u16 run_continue = 0;

    if(motor_manager.run_state_main == RUN) {
        for(i = 0; i < max_motor; i++) {
            //if((motor_manager.status[i].run_flag == 0) || (motor_manager.run_state_self[i] != motor_manager.run_state_main)) {
            if(motor_manager.status[i].run_flag == 0) {
                continue;
            }
            if(motor_queue.cur_user == QUEUE_USER_MOTOR_TRIG) {
                if(motor_manager.run_state_self[i] == FIRST_HALF) {
                    motor_manager.motor_run_info.first_half++;
                    if((motor_manager.motor_run_info.first_half-motor_manager.motor_run_info.second_half)>2) {
                        motor_manager.run_state_self[i] = FIRST_HALF;
                        BLE_SEND_LOG((u8*)&"\x00", 1);
                    }
                } else if(motor_manager.run_state_self[i] == SECOND_HALF) {
                    motor_manager.motor_run_info.second_half++;
                    if((motor_manager.motor_run_info.second_half-motor_manager.motor_run_info.first_half)>2) {
                        motor_manager.run_state_self[i] = SECOND_HALF;
                        BLE_SEND_LOG((u8*)&"\x01", 1);
                    }
                }
                if(motor_manager.status[i].run_direc == pos) {
                    motor_manager.motor_run_info.pos_step++;
                } else {
                    motor_manager.motor_run_info.neg_step++;
                }
            }
            motor_manager.cb[i].motor_run_half[motor_manager.run_state_self[i]][motor_manager.status[i].run_direc](NULL);
            motor_manager.run_state_self[i] = (motor_manager.run_state_self[i]==FIRST_HALF)?SECOND_HALF:FIRST_HALF;
        }
		motor_manager.run_state_main = STOP;
		timer_event(motor_manager.timer_interval, motor_check_run);
    }else if(motor_manager.run_state_main == STOP) {
        for(i = 0; i < max_motor; i++) {
            if(motor_manager.status[i].run_flag == 0) {
                continue;
            }
            if(motor_queue.cur_user == QUEUE_USER_MOTOR_TRIG) {
                motor_manager.motor_run_info.stop_cnt++;
            }
            motor_manager.cb[i].motor_stop(NULL);
            if(motor_manager.run_next[i] == 1) {
                run_continue += motor_check_continue(i);
            } else {
                motor_manager.status[i].run_flag = 0;
            }
        }
		motor_manager.run_state_main = RUN;
        if(run_continue > 0) {
            timer_event(motor_manager.timer_interval, motor_check_run);
        } else {
            motor_manager.motor_running = 0;
//            if(motor_check_idle() == 0) {
////                motor_manager.motor_running = 0;
//                timer_event(1, motor_ctrl_dequeue);
//            }
        }
    }
}
#endif
u8 motor_run_one_unit(motor_ctrl_queue_t *ctrl_params)
{
	u8 i = 0;
    u8 run_cnt = 0;

    motor_manager.timer_interval = ctrl_params->intervel;
    motor_manager.queue_cb = ctrl_params->cb_func;
    run_cnt = 0;
	for(i = 0; i < max_motor; i++) {
		if(motor_manager.status[i].run_flag == 1) {
        	if(motor_manager.status[i].dst_pos > motor_manager.status[i].cur_pos) {
        		motor_manager.status[i].run_direc = pos;
        	}else if(motor_manager.status[i].dst_pos < motor_manager.status[i].cur_pos) {
        		motor_manager.status[i].run_direc = neg;
        	} else {
                motor_manager.status[i].run_direc = none;
                motor_manager.status[i].run_flag = 0;
        	}
		}
        if(motor_manager.status[i].run_flag != 0) {
            motor_manager.run_next[i] = 1;
            run_cnt++;
    	    motor_manager.status[i].unit_step_num = 0;
        }
	}

    if(run_cnt > 0) {
        motor_manager.motor_running = 1;
        motor_check_run(0);
    } else if(motor_manager.queue_cb != NULL) {
        motor_manager.queue_cb(NULL);
    }

    return run_cnt;
}
void motor_run_one_step(motor_ctrl_queue_t *ctrl_params)
{
    volatile zero_adjust_t zero_adjust = {.motor_num=ctrl_params->cb_params[0], .motor_pos=ctrl_params->cb_params[1]};
    motor_manager.status[zero_adjust.motor_num].run_direc = zero_adjust.motor_pos;
    motor_manager.timer_interval = ctrl_params->intervel;
    motor_manager.status[zero_adjust.motor_num].run_flag = 1;
    motor_manager.run_next[zero_adjust.motor_num] = 0;
    motor_check_run(0);
}
void motor_run_test(motor_ctrl_queue_t *ctrl_params)
{
    motor_manager.timer_interval = ctrl_params->intervel;
    motor_manager.queue_cb = ctrl_params->cb_func;
    motor_check_run(0);
}
void motor_pre_handler(motor_ctrl_queue_t *ctrl_params, u8 instance)
{
    if(ctrl_params->mask == MOTOR_MASK_RUN_TEST) {
        motor_run_test(ctrl_params);
    } else if (ctrl_params->mask == MOTOR_MASK_ZERO_ADJUST) {
        motor_run_one_step(ctrl_params);
    } else if (instance > 0) {
        motor_run_one_unit(ctrl_params);
    } else if(ctrl_params->cb_func != NULL) {
        ctrl_params->cb_func(NULL);
    }
    
    u8 ble_log[7] = {0x5F, 0x04, 0,0,0,0,0};
    ble_log[2] = motor_queue.tail;
    ble_log[3] = motor_queue.head;
    ble_log[4] = motor_queue.cur_user;
    ble_log[5] = motor_queue.motor_name;
    ble_log[6] = motor_queue.handle_name;
    BLE_SEND_LOG(ble_log, 7);
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
        if(motor_manager.status[i].run_flag == 1) {
            return 1;
        }
    }
    return 0;
}

