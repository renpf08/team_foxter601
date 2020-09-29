#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <time.h>
#include <mem.h>
#include "config.h"
#include "adapter.h"
#include <panic.h>
#include <buf_utils.h>
#include <csr_ota.h>
#include "../log.h"

u8 stete_battery_week = state_battery;
u8 activity_percent = 0;
static u8 charge_start = 0;
static u8 swing_state = 0;
static STATE_E state_last = INIT;

key_sta_ctrl_t key_sta_ctrl = {0, 0, 0, 0};

const u8 date[] = {DAY_0,
	DAY_1, DAY_2, DAY_3, DAY_4, DAY_5,
	DAY_6, DAY_7, DAY_8, DAY_9, DAY_10,
	DAY_11, DAY_12, DAY_13, DAY_14, DAY_15,
	DAY_16, DAY_17, DAY_18, DAY_19, DAY_20,
	DAY_21, DAY_22, DAY_23, DAY_24, DAY_25,
	DAY_26, DAY_27, DAY_28, DAY_29, DAY_30,
	DAY_31};

//callback handler
extern s16 mag_cb_handler(void *args);
extern s16 button_cb_handler(void *args);

//module init
extern s16 clock_init(adapter_callback cb);
extern s16 ancs_init(adapter_callback cb);
extern s16 cmd_init(adapter_callback cb);
extern s16 vib_init(adapter_callback cb);
extern s16 step_sample_init(adapter_callback cb);
extern s16 mag_sample_init(void);
extern s16 ble_switch_init(adapter_callback cb);
extern s16 charge_status_init(adapter_callback cb);

s16 csr_event_callback(EVENT_E ev);
void driver_uninit(void);
static void system_polling_handler(u16 id);

typedef struct {
	driver_t *drv;
	adapter_callback cb;
}adapter_t;

static adapter_t adapter = {
	.drv = NULL,
	.cb = NULL,
};

static u8 button_event_check(u16 button_event)
{
    motor_run_status_t *motor_sta = motor_get_status();
    app_state state_ble = ble_state_get();
//    u8 vib_run_state = vib_state();
//
//    vib_stop();
//    if(vib_run_state == 1) {
//        return 1;
//    }
    if(button_event != KEY_M_SHORT_PRESS) {
        return 0;
    }
    if(state_ble == app_advertising) {
        return 0;
    }
    #if USE_ACTIVITY_NOTIFY
    if(motor_sta[activity_motor].cur_pos != 0) {
        motor_activity_to_position(NOTIFY_NONE);
        return 1;
    }
    #else
    if(motor_sta[notify_motor].cur_pos != 0) {
		motor_notify_to_position(NOTIFY_NONE);
        return 1;
    }
    #endif
    return 0;
}
s16 csr_event_callback(EVENT_E ev)
{
	if(ev >= EVENT_MAX) {
		return -1;
	}else if(ev < MAGNETOMETER_READY){
		u16 combo_event = button_cb_handler((void*)ev);
        if(combo_event == KEY_M_ULTRA_LONG_PRESS) {
            system_pre_reboot_handler(REBOOT_TYPE_BUTTON);
        }
        if(button_event_check(combo_event) == 1) {
            return 0;
        }
        if(combo_event < REPORT_MAX) {     // sure the button released
        	adapter.cb(combo_event, NULL);
        }
        if(combo_event == KEY_M_SHORT_PRESS) {
        	adapter.cb(COMPASS, NULL);
        }
	} else if(ev == MAGNETOMETER_READY) {
	    mag_cb_handler((void*)ev);
	}
	
	return 0;
}

static s16 driver_init(void)
{
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

	//charge detect init
	adapter.drv->charge->charge_init(&args, NULL);
	
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
    log_send_init(cb);
	clock_init(cb);
	ancs_init(cb);
    cmd_init(cb);
	motor_manager_init();
	battery_init(cb);
    step_sample_init(cb);
    mag_sample_init();
    ble_switch_init(cb);
    nvm_storage_init(cb);
	vib_init(cb);
	charge_status_init(cb);
    system_post_reboot_handler();

    timer_event(1000, system_polling_handler);
	return 0;
}

static void upload_current_steps(void)
{
    u16 length = 0; 
    u8 rsp_buf[20];
    u8 *tmp_buf = rsp_buf;
    clock_t *clock = clock_get();
    cmd_params_t* params = cmd_get_params();
    params->steps = step_get();
    params->clock = clock;

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
static void activity_polling_check(void)
{
    u32 target_steps = cmd_get()->user_info.target_steps;
    u32 current_steps = step_get();
    static u32 last_steps = 0;
    static u8 refresh_cnt = 0;

    if((target_steps <= 0) || (target_steps >= 50000)) {
        target_steps = 1000;
    }
    if(current_steps > target_steps) {
        activity_percent = 40; // total 40 grids
    } else if(current_steps <= target_steps) {
        activity_percent = (current_steps*40)/target_steps;
    } else if(target_steps == 0) {
        activity_percent = 0;
    }

    if((last_steps != current_steps) || (refresh_cnt >= 30)) {
        refresh_cnt = 0;
        motor_activity_to_position(activity_percent);
        upload_current_steps();
    } else {
        refresh_cnt++;
    }
    last_steps = current_steps;
}

//void refresh_step(void)
//{
//    clock_t *clock = clock_get();
//    cmd_params_t* params = cmd_get_params();
//    params->steps = step_get();
//    params->clock = clock;
//    //timer_event(10, activity_handler);
//}
static void clock_charge_swing(u16 id)
{
	u8 battery_level = BAT_PECENT_0;
    if(charge_start == 0) {
        swing_state = 0;
		battery_level = battery_percent_read();
		motor_battery_week_to_position(battery_level);
        return;
    }
    if(swing_state == 0) {
        swing_state = 1;
        motor_battery_week_to_position(BAT_PECENT_0);
    } else {
        swing_state = 0;
        motor_battery_week_to_position(BAT_PECENT_100);
    }
    timer_event(1500, clock_charge_swing);
}
void charge_check(REPORT_E cb)
{
    if(cb == CHARGE_SWING) {
        if(charge_start == 0) {
            timer_event(1, clock_charge_swing);
        }
        charge_start = 1;
    } else if(cb == CHARGE_STOP) {
        charge_start = 0;
    }
}
static void charge_polling_check(void)
{
    static s16 last_status = not_incharge;
    s16 now_status = charge_status_get();
    u8 ble_log[3] = {LOG_CMD_SEND_DEBUG, LOG_SEND_CHARGE_STATE, 0};
    
	if((last_status == not_incharge) && (now_status == incharge)) {
        ble_log[2] = 0;
        BLE_SEND_LOG(ble_log, 3);
        adapter.cb(CHARGE_SWING, NULL);
	} else if((last_status == incharge) && (now_status == not_incharge)) {
        ble_log[2] = 1;
        BLE_SEND_LOG(ble_log, 3);
	    adapter.cb(CHARGE_STOP, NULL);
	}
    last_status = now_status;
}
static void system_polling_handler(u16 id)
{
    charge_polling_check();
    activity_polling_check();
    
    timer_event(1000, system_polling_handler);
}
void system_pre_reboot_handler(reboot_type_t type)
{
    u8 motor_cur_pos[max_motor] = {0,0,0,0,0,0};
    motor_run_status_t *motor_sta = motor_get_status();

    motor_cur_pos[minute_motor] = motor_sta[minute_motor].cur_pos;
    motor_cur_pos[hour_motor] = motor_sta[hour_motor].cur_pos;
    motor_cur_pos[activity_motor] = motor_sta[activity_motor].cur_pos;
    motor_cur_pos[date_motor] = motor_sta[date_motor].cur_pos;
    motor_cur_pos[battery_week_motor] = motor_sta[battery_week_motor].cur_pos;
    motor_cur_pos[notify_motor] = motor_sta[notify_motor].cur_pos;    

    #if USE_PARAM_STORE
    clock_t* clock = clock_get();
    nvm_write_motor_current_position((u16*)motor_cur_pos, 0);
    nvm_write_date_time((u16*)clock, 0);
    nvm_write_motor_init_flag();
    #endif
    if(type == REBOOT_TYPE_BUTTON) {
        nvm_storage_reset();
        APP_Move_Bonded(4);
        Panic(0);
    } else if (type == REBOOT_TYPE_CMD) {
        nvm_storage_reset();
        APP_Move_Bonded(4);
        Panic(0);
    } else if (type == REBOOT_TYPE_OTA) {
        OtaReset();
    }
}
void system_post_reboot_handler(void)
{
    u8 motor_cur_pos[max_motor] = {MINUTE_0, HOUR0_0, ACTIVITY_0, DAY_1, BAT_PECENT_0, NOTIFY_NONE};
    motor_run_status_t *motor_sta = motor_get_status();
    cmd_group_t *value = cmd_get();

    #if USE_PARAM_STORE
    clock_t* clock = clock_get();
    if(nvm_read_motor_init_flag() == 0) {
        nvm_read_motor_current_position((u16*)motor_cur_pos, 0);
        nvm_read_date_time((u16*)clock, 0);
    
        nvm_read_alarm_clock((u16*)&value->set_alarm_clock, 0);
        nvm_read_pairing_code((u16*)&value->pair_code, 0);
        nvm_read_personal_info((u16*)&value->user_info, 0);
        cmd_check(value);
    }
    #endif
    motor_sta[minute_motor].cur_pos = motor_cur_pos[minute_motor];
    motor_sta[hour_motor].cur_pos = motor_cur_pos[hour_motor];
    motor_sta[activity_motor].cur_pos = motor_cur_pos[activity_motor];
    motor_sta[date_motor].cur_pos = motor_cur_pos[date_motor];
    motor_sta[battery_week_motor].cur_pos = motor_cur_pos[battery_week_motor];
    motor_sta[notify_motor].cur_pos = motor_cur_pos[notify_motor];
    
    motor_sta[minute_motor].dst_pos = motor_cur_pos[minute_motor];
    motor_sta[hour_motor].dst_pos = motor_cur_pos[hour_motor];
    motor_sta[activity_motor].dst_pos = motor_cur_pos[activity_motor];
    motor_sta[date_motor].dst_pos = motor_cur_pos[date_motor];
    motor_sta[battery_week_motor].dst_pos = motor_cur_pos[battery_week_motor];
    motor_sta[notify_motor].dst_pos = motor_cur_pos[notify_motor];
}
#if USE_UART_PRINT
void print(u8 *buf, u16 num)
{
//	u8 rn[2] = {0x0d, 0x0a};
//	if(NULL != adapter.drv) {
//		adapter.drv->uart->uart_write(buf, num);
//		adapter.drv->uart->uart_write(rn, 2);
//	}
}

void trace(u8 *buf, u16 num)
{
	u8 rn[2] = {0x0d, 0x0a};
	if(NULL != adapter.drv) {
		adapter.drv->uart->uart_write(buf, num);
		adapter.drv->uart->uart_write(rn, 2);
	}
}
#endif

void system_reboot(u8 reboot_type)
{
    if(reboot_type == 0) {
        APP_Move_Bonded(4);
    }
    Panic(0x5AFF);
}
void sync_time(void)
{
	cmd_set_time_t *time = (cmd_set_time_t *)&cmd_get()->set_time;
    clock_t* clock = clock_get();
    u8 ble_log[10] = {LOG_CMD_SEND_DEBUG, LOG_SEND_SYNC_TIME, 0, 0, 0, 0, 0, 0, 0, 0};
    ble_log[2] = time->year[1];
    ble_log[3] = time->year[0];
    ble_log[4] = time->month;
    ble_log[5] = time->day;
    ble_log[6] = time->hour;
    ble_log[7] = time->minute;
    ble_log[8] = time->second;
    ble_log[9] = time->week;

    clock->year = time->year[1]<<8 | time->year[0];
    clock->month = time->month;
    clock->day = time->day;
    clock->week = time->week;
    clock->hour = time->hour;
    clock->minute = time->minute;
    clock->second = time->second;

    BLE_SEND_LOG(ble_log, 10);
//    refresh_step();
	motor_minute_to_position(clock->minute);
	motor_hour_to_position(clock->hour);
    motor_date_to_position(date[clock->day]);
}

void motor_restore_position(REPORT_E cb)
{
    u8 position = 0;
    
    if(cb == KEY_A_B_LONG_PRESS) {
    	if(stete_battery_week == state_battery) {
    		position = battery_percent_read();
    	}else {
    		position = clock_get()->week;
    	}
        motor_battery_week_to_position(position);    
        motor_activity_to_position(activity_percent);
    }
}

STATE_E get_last_state(void)
{
    return state_last;
}

void set_last_state(STATE_E state)
{
    state_last = state;
}

s16 timer_remove(s16 tid)
{
	return adapter.drv->timer->timer_del(tid);
}

s16 timer_event(u16 ms, timer_cb cb)
{
	return adapter.drv->timer->timer_start(ms, cb);
}
