#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <time.h>
#include <mem.h>
#include "config.h"
#include "adapter.h"
#include <panic.h>
#include <buf_utils.h>
#include <csr_ota.h>

typedef struct {
	driver_t *drv;
	adapter_callback cb;
}adapter_t;

typedef struct {
    u8 rotate_speed;
    u8 backward_pos;
    u8 forward_pos;
} trig_range_t;

static adapter_t adapter = {
	.drv = NULL,
	.cb = NULL,
};

motor_queue_buffer_t motor_queue;
adapter_ctrl_t adapter_ctrl = {
    #if USE_CMD_TEST_LOG_TYPE_EN
    .log_type_en = {1, 1, 1},
    #endif
    .system_started = 0,
    .system_reboot_lock = 0,
    .activity_pos = 0,
    .current_motor_num = 0,
    .current_bat_week_sta = state_battery,
    .reboot_type = REBOOT_TYPE_BUTTON,
    .motor_dst = {0, 0, 0, 0, 0, 0},
    .motor_zero = {MINUTE_0, HOUR0_0, ACTIVITY_0, DAY_1, BAT_PECENT_0, NOTIFY_NONE},
    .date = { DAY_0, DAY_1, DAY_2, DAY_3, DAY_4, DAY_5,
            	DAY_6, DAY_7, DAY_8, DAY_9, DAY_10,
            	DAY_11, DAY_12, DAY_13, DAY_14, DAY_15,
            	DAY_16, DAY_17, DAY_18, DAY_19, DAY_20,
            	DAY_21, DAY_22, DAY_23, DAY_24, DAY_25,
            	DAY_26, DAY_27, DAY_28, DAY_29, DAY_30,
            	DAY_31},
};


//callback handler
extern s16 mag_cb_handler(void *args);
extern s16 button_cb_handler(void *args);

//module init
extern s16 clock_init(adapter_callback cb);
extern s16 ancs_init(adapter_callback cb);
extern s16 cmd_init(adapter_callback cb);

extern s16 step_sample_init(adapter_callback cb);
extern s16 mag_sample_init(void);
extern s16 ble_switch_init(adapter_callback cb);

s16 csr_event_callback(EVENT_E ev);
void driver_uninit(void);

s16 csr_event_callback(EVENT_E ev)
{
	if(ev >= EVENT_MAX) {
		return -1;
	}else if(ev < MAGNETOMETER_READY){
		//print((u8 *)&ev, 1);
		u16 combo_event = button_cb_handler((void*)ev);
        if(combo_event < REPORT_MAX) {     // sure the button released
        	adapter.cb(combo_event, NULL);
    	    //adapter.drv->uart->uart_write((u8 *)&combo_event, 1);
        }
	} else if(ev == MAGNETOMETER_READY) {
	    mag_cb_handler((void*)ev);
	}
	
	return 0;
}
static s16 driver_init(void)
{
	//u8 test[25] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};

	adapter.drv = get_driver();
	//timer init
	adapter.drv->timer->timer_init(NULL, NULL);
	adapter.drv->battery->battery_init(NULL, NULL);
	adapter.drv->keya->key_init(&args, csr_event_callback);
	adapter.drv->keym->key_init(&args, csr_event_callback);
	adapter.drv->keyb->key_init(&args, csr_event_callback);	
	adapter.drv->flash->flash_init(NULL, NULL);

    #if USE_UART_PRINT
	//uart init and test
	adapter.drv->uart->uart_init(&args, NULL);
	//adapter.drv->uart->uart_write(test, 23);
	#endif

	//vibrator init
	adapter.drv->vibrator->vibrator_init(&args, NULL);
	adapter.drv->vibrator->vibrator_off(NULL);

	//gsensor init
	adapter.drv->gsensor->gsensor_init(&args, NULL);

	//magnetometer init
	adapter.drv->magnetometer->magnetometer_init(&args, csr_event_callback);

	//motor init and off
	adapter.drv->motor_hour->motor_init(&args, NULL);
	adapter.drv->motor_hour->motor_stop(NULL);
	adapter.drv->motor_minute->motor_init(&args, NULL);
	adapter.drv->motor_minute->motor_stop(NULL);
	adapter.drv->motor_activity->motor_init(&args, NULL);
	adapter.drv->motor_activity->motor_stop(NULL);
	adapter.drv->motor_date->motor_init(&args, NULL);
	adapter.drv->motor_date->motor_stop(NULL);
	adapter.drv->motor_battery_week->motor_init(&args, NULL);
	adapter.drv->motor_battery_week->motor_stop(NULL);
	adapter.drv->motor_notify->motor_init(&args, NULL);
	adapter.drv->motor_notify->motor_stop(NULL);

	return 0;
}
s16 adapter_init(adapter_callback cb)
{
	//driver init
	driver_init();
	adapter.cb = cb;

	//module init
    nvm_init(cb);
	clock_init(cb);
	ancs_init(cb);
    cmd_init(cb);
	battery_init(cb);
	motor_manager_init();
    step_sample_init(cb);
    mag_sample_init();
    ble_switch_init(cb);

    MemSet(&motor_queue, 0, sizeof(motor_queue_buffer_t));
	return 0;
}
#if USE_UART_PRINT
void print(u8 *buf, u16 num)
{
	u8 rn[2] = {0x0d, 0x0a};
	if(NULL != adapter.drv) {
		adapter.drv->uart->uart_write(buf, num);
		adapter.drv->uart->uart_write(rn, 2);
	}
}
#endif
static void activity_handler(u16 id)
{
    u32 target_steps = cmd_get()->user_info.target_steps;
    u32 current_steps = step_get();
    motor_queue_t queue_param = {.user = QUEUE_USER_ACTIVITY_CALC, .intervel = 25, .mask = MOTOR_MASK_ACTIVITY};

    if(current_steps > target_steps) {
        queue_param.dest[activity_motor] = 40; // total 40 grids
    } else if(current_steps <= target_steps) {
        queue_param.dest[activity_motor] = (current_steps*40)/target_steps;
    } else if(target_steps == 0) {
        queue_param.dest[activity_motor] = 0;
    }
    motor_params_enqueue(&queue_param);
    
    u16 length = 0; 
    u8 rsp_buf[20];
    u8 *tmp_buf = rsp_buf;
    cmd_params_t* params = cmd_get_params();
    BufWriteUint8((uint8 **)&tmp_buf, 0x01);
    BufWriteUint8((uint8 **)&tmp_buf, 0x00);
    BufWriteUint8((uint8 **)&tmp_buf, 0x00);
    BufWriteUint16((uint8 **)&tmp_buf, params->clock->year);//SB100_data.AppApplyDateData.Year);
    BufWriteUint8((uint8 **)&tmp_buf, params->clock->month);//SB100_data.AppApplyDateData.Month);
    BufWriteUint8((uint8 **)&tmp_buf, params->clock->day);//SB100_data.AppApplyDateData.Date);
    BufWriteUint16((uint8 **)&tmp_buf, params->steps);//(SB100_data.AppApplyData.StepCounts));
    BufWriteUint8((uint8 **)&tmp_buf, params->steps>>16);//(SB100_data.AppApplyData.StepCounts>>16));
    BufWriteUint16((uint8 **)&tmp_buf, 0);//cmd_get_data->distance);//(SB100_data.AppApplyData.Distance));
    BufWriteUint8((uint8 **)&tmp_buf, 0);//cmd_get_data->distance>>16);//(SB100_data.AppApplyData.Distance>>16));
    BufWriteUint16((uint8 **)&tmp_buf, 0);//cmd_get_data->calorie);//(SB100_data.AppApplyData.Calorie));
    BufWriteUint8((uint8 **)&tmp_buf, 0);//cmd_get_data->calorie>>16);//(SB100_data.AppApplyData.Calorie>>16));
    BufWriteUint16((uint8 **)&tmp_buf, 0);//cmd_get_data->floor_counts);//SB100_data.AppApplyData.FloorCounts);
    BufWriteUint16((uint8 **)&tmp_buf, 0);//cmd_get_data->acute_sport_time);//SB100_data.AppApplyData.AcuteSportTimeCounts);
    length = tmp_buf - rsp_buf;
    BLE_SEND_DATA(rsp_buf, length);
}
void refresh_step(void)
{
    clock_t *clock = clock_get();
    cmd_params_t* params = cmd_get_params();
    params->steps = step_get();
    params->clock = clock;
    timer_event(1, activity_handler);
}
void sync_time(void)
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
    motor_set_date_time(clock, (MOTOR_MASK_HOUR|MOTOR_MASK_MINUTE|MOTOR_MASK_DATE|MOTOR_MASK_BAT_WEEK));
    refresh_step();
}
static void pre_reboot_handler(u16 id)
{
    static u8 wait_cnt = 0;
    
    if((motor_check_idle() != 0) && (wait_cnt < 50)) { // max waiting time: 5 second
        wait_cnt++;
        timer_event(100, pre_reboot_handler);
        return;
    }

    if(adapter_ctrl.reboot_type == REBOOT_TYPE_BUTTON) {
        Panic(0);
    } else if(adapter_ctrl.reboot_type == REBOOT_TYPE_OTA) {
        OtaReset();
    }
}
u8 get_battery_week_pos(STATE_BATTERY_WEEK_E state)
{
    if((state) == state_battery) {
		return battery_percent_read();
    } else {
		return clock_get()->week;
    }
}
static void motor_trig_handler(u16 id)
{
    motor_queue_t queue_param = {.user = QUEUE_USER_MOTOR_TRIG};
    trig_range_t trig_range[max_motor] = {
            [hour_motor]            = {10, HOUR0_0,        HOUR0_2},
            [minute_motor]          = {10, MINUTE_0,       MINUTE_3},
            [battery_week_motor]    = {10, BAT_PECENT_0,   BAT_PECENT_40},
            [date_motor]            = {10, DAY_1,          DAY_5},
            [activity_motor]        = {25, ACTIVITY_0,     ACTIVITY_10},
            [notify_motor]          = {25, NOTIFY_NONE,    NOTIFY_EMAIL},
    };

    queue_param.intervel = trig_range[adapter_ctrl.current_motor_num].rotate_speed;
    queue_param.mask = (1<<adapter_ctrl.current_motor_num);
    queue_param.dest[adapter_ctrl.current_motor_num] = trig_range[adapter_ctrl.current_motor_num].forward_pos;
    motor_params_enqueue(&queue_param);
    queue_param.dest[adapter_ctrl.current_motor_num] = trig_range[adapter_ctrl.current_motor_num].backward_pos;
    motor_params_enqueue(&queue_param);
}
static void motor_set_position(motor_queue_t *queue_params)
{
    u8 set_result = 0;

    if(queue_params->mask != MOTOR_MASK_TRIG) {
        if(queue_params->mask & (MOTOR_MASK_ALL|MOTOR_MASK_MINUTE)) {
            set_result += motor_minute_to_position(queue_params->dest[minute_motor]);
        }
        if(queue_params->mask & (MOTOR_MASK_ALL|MOTOR_MASK_HOUR)) {
            set_result += motor_hour_to_position(queue_params->dest[hour_motor]);
        }
        if(queue_params->mask & (MOTOR_MASK_ALL|MOTOR_MASK_ACTIVITY)) {
            set_result += motor_activity_to_position(queue_params->dest[activity_motor]);
        }
        if(queue_params->mask & (MOTOR_MASK_ALL|MOTOR_MASK_DATE)) {
            set_result += motor_date_to_position(queue_params->dest[date_motor]);
        }
        if(queue_params->mask & (MOTOR_MASK_ALL|MOTOR_MASK_BAT_WEEK)) {
            set_result += motor_battery_week_to_position(queue_params->dest[battery_week_motor]);
        }
        if(queue_params->mask & (MOTOR_MASK_ALL|MOTOR_MASK_NOTIFY)) {
            set_result += motor_notify_to_position(queue_params->dest[notify_motor]);
        }
        if(set_result > 0) {
            motor_run_one_unit(queue_params->intervel);
        }
    }
    if(queue_params->mask & MOTOR_MASK_TRIG) { // zero adjust mode
        timer_event(1, motor_trig_handler);
    }
}
static void motor_queue_handler(u16 id)
{
    if(motor_manager.motor_running != 0) {
        timer_event(100, motor_queue_handler);
    }
    if(motor_queue.tail != motor_queue.head) {
        timer_event(100, motor_params_dequeue);
    }
}
void motor_params_dequeue(u16 id)
{
    if(motor_queue.tail != motor_queue.head) {
        motor_queue.cur_user = motor_queue.queue_params[motor_queue.tail].user; // for debug
        motor_set_position(&motor_queue.queue_params[motor_queue.tail]);
        motor_queue.tail = (motor_queue.tail+1)%MOTOR_QUEUE_SIZE;
        motor_queue.read_cnt++;
        timer_event(1, motor_queue_handler);
    }
}
void motor_params_enqueue(motor_queue_t *queue_params)
{
    /** queue is fulled, discard the oldest one */
    if(motor_queue.tail == (motor_queue.head+1)%MOTOR_QUEUE_SIZE) {
        motor_queue.tail = (motor_queue.tail+1)%MOTOR_QUEUE_SIZE;
        motor_queue.drop_cnt++;
    }
    MemCopy(&motor_queue.queue_params[motor_queue.head], queue_params, sizeof(motor_queue_t));
    motor_queue.head = (motor_queue.head+1)%MOTOR_QUEUE_SIZE;
    motor_queue.write_cnt++;
    if(motor_manager.motor_running == 0) {
        timer_event(1, motor_queue_handler);
    }
}
void motor_set_date_time(clock_t *clock, MOTOR_MASK_E mask)
{
    motor_queue_t queue_param = {.user = QUEUE_USER_DATE_TIME, .intervel = 10};
    
    queue_param.dest[minute_motor] = clock->minute;
    queue_param.dest[hour_motor] = clock->hour;
    queue_param.dest[date_motor] = adapter_ctrl.date[clock->day];
    queue_param.dest[battery_week_motor] = get_battery_week_pos(adapter_ctrl.current_bat_week_sta);
    queue_param.mask = mask;
    motor_params_enqueue(&queue_param);
}
static void motor_test_mode_reboot_handler(u16 id)
{
    if(motor_manager.run_test_mode == 0) {
		system_pre_reboot_handler(adapter_ctrl.reboot_type);
        return;
    }
    timer_event(100, motor_test_mode_reboot_handler);
}
void system_pre_reboot_handler(reboot_type_t type)
{
    clock_t* clock = clock_get();
    motor_queue_t queue_param = {.user = QUEUE_USER_PRE_REBOOT, .intervel = 10, .mask = MOTOR_MASK_ALL};

    adapter_ctrl.reboot_type = type;
    if(motor_manager.run_test_mode == 1) {
        adapter.cb(KEY_A_B_M_LONG_PRESS, NULL);
        timer_event(100, motor_test_mode_reboot_handler);
    } else {
        adapter_ctrl.system_reboot_lock = 1;
        APP_Move_Bonded(4);
        nvm_write_date_time((u16*)clock, 0);
        nvm_write_motor_current_position((u16*)&adapter_ctrl.motor_dst, 0);
        MemCopy(queue_param.dest, adapter_ctrl.motor_zero, max_motor*sizeof(u8));
        motor_params_enqueue(&queue_param);
        timer_event(1, pre_reboot_handler);
    }
}
void system_post_reboot_handler(void)
{
    clock_t* clock = clock_get();
    motor_queue_t queue_param = {.user = QUEUE_USER_POST_REBOOT, .intervel = 10, .mask = MOTOR_MASK_ALL};;
    
    if(nvm_read_motor_init_flag() == 0) {
        nvm_read_motor_current_position((u16*)queue_param.dest, 0);
        nvm_read_date_time((u16*)clock, 0);
        queue_param.dest[minute_motor] = clock->minute;
        queue_param.dest[hour_motor] = clock->hour;
        queue_param.dest[date_motor] = adapter_ctrl.date[clock->day];
        queue_param.dest[battery_week_motor] = get_battery_week_pos(adapter_ctrl.current_bat_week_sta);
    } else {
        MemCopy(queue_param.dest, adapter_ctrl.motor_zero, max_motor);
    }
    motor_params_enqueue(&queue_param);
    adapter_ctrl.system_started = 1;
    timer_event(1, motor_queue_handler);
}
u8 state_machine_check(REPORT_E cb)
{
    if(cb == KEY_M_ULTRA_LONG_PRESS) {
        system_pre_reboot_handler(REBOOT_TYPE_BUTTON);
    } else if(cb >= REPORT_MAX) {
        return 1;
    }
    if(adapter_ctrl.system_reboot_lock == 1) {
        return 1;
    }
    if(adapter_ctrl.system_started == 0) {
        return 1;
    }
    if((motor_manager.run_test_mode == 1) && (cb != KEY_A_B_M_LONG_PRESS)) {
        return 1;
    }
    return 0;
}

void timer_event(u16 ms, timer_cb cb)
{
	adapter.drv->timer->timer_start(ms, cb);
}
