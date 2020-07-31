#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <time.h>
#include <mem.h>
#include "config.h"
#include "adapter.h"
#include <panic.h>
#include <buf_utils.h>
#include <csr_ota.h>

typedef s16 (* motor_handler)(u8 motor);
typedef struct {
    motor_handler func;
    u8 zero_pos;
    u8 trig_pos;
} motor_trig_t;
zero_adjust_lock_t zero_adjust_mode = {0, 0};
u8 motor_dst[max_motor+1] = {0, 0, 0, 0, 0, 0, state_battery};
reboot_type_t reboot_type = 0;
u8 motor_zero[max_motor] = {MINUTE_0, HOUR0_0, ACTIVITY_0, DAY_1, BAT_PECENT_0, NOTIFY_NONE};
u8 system_reboot_lock = 0;
u8 activity_pos = 0;
u8 notify_pos = 0;
u8 current_motor_num = 0;
motor_trig_t motor_trig[max_motor] = {
    [minute_motor]          = {motor_minute_to_position,        MINUTE_0,       MINUTE_3},
    [hour_motor]            = {motor_hour_to_position,          HOUR0_0,        HOUR0_2},
    [activity_motor]        = {motor_activity_to_position,      ACTIVITY_0,     ACTIVITY_10},
    [date_motor]            = {motor_date_to_position,          DAY_1,          DAY_5},
    [battery_week_motor]    = {motor_battery_week_to_position,  BAT_PECENT_0,   BAT_PECENT_40},
    [notify_motor]          = {motor_notify_to_position,        NOTIFY_NONE,    NOTIFY_EMAIL},
};

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

extern s16 step_sample_init(adapter_callback cb);
extern s16 mag_sample_init(void);
extern s16 ble_switch_init(adapter_callback cb);

s16 csr_event_callback(EVENT_E ev);
void driver_uninit(void);

typedef struct {
	driver_t *drv;
	adapter_callback cb;
}adapter_t;

static adapter_t adapter = {
	.drv = NULL,
	.cb = NULL,
};

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
	motor_manager_init();
	battery_init(cb);
    step_sample_init(cb);
    mag_sample_init();
    ble_switch_init(cb);

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

    if(current_steps > target_steps) {
        activity_pos = 40; // total 40 grids
    } else if(current_steps <= target_steps) {
        activity_pos = (current_steps*40)/target_steps;
    } else if(target_steps == 0) {
        activity_pos = 0;
    }
    motor_dst[activity_motor] = activity_pos;
    motor_set_position(motor_dst, MOTOR_MASK_ACTIVITY);
    
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
    timer_event(10, activity_handler);
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
    motor_set_day_time(clock, (MOTOR_MASK_HOUR|MOTOR_MASK_MINUTE|MOTOR_MASK_DATE));
    refresh_step();
}
static void pre_reboot_handler(u16 id)
{
    if(motor_check_idle() != 0) {
        timer_event(100, pre_reboot_handler);
        return;
    }

    if(reboot_type == REBOOT_TYPE_BUTTON) {
        Panic(0);
    } else if(reboot_type == REBOOT_TYPE_OTA) {
        OtaReset();
    }
}
static u8 get_battery_week_pos(void)
{
    if(motor_dst[max_motor] == state_battery) {
		motor_dst[battery_week_motor] = battery_percent_read();
    } else if(motor_dst[max_motor] == state_week) {
		motor_dst[battery_week_motor] = clock_get()->week;
    }
    return motor_dst[battery_week_motor];
}
static void motor_trig_handler(u16 id)
{
    static u8 trig_state = 0;

    if(motor_check_idle() == 0) {
        if(trig_state == 0) {
            trig_state = 1;
            motor_trig[current_motor_num].func(motor_trig[current_motor_num].trig_pos);
        } else if(trig_state == 1) {
            trig_state = 2;
            motor_trig[current_motor_num].func(motor_trig[current_motor_num].zero_pos);
        } else {
            trig_state = 0;
            return;
        }
    }
    timer_event(100, motor_trig_handler);
}
void motor_set_position(u8* motor_pos, MOTOR_MASK_E motor_mask)
{
    if(motor_mask & (MOTOR_MASK_ALL|MOTOR_MASK_MINUTE)) {
        motor_minute_to_position(motor_pos[minute_motor]);
    }
    if(motor_mask & (MOTOR_MASK_ALL|MOTOR_MASK_HOUR)) {
        motor_hour_to_position(motor_pos[hour_motor]);
    }
    if(motor_mask & (MOTOR_MASK_ALL|MOTOR_MASK_ACTIVITY)) {
        motor_activity_to_position(motor_pos[activity_motor]);
    }
    if(motor_mask & (MOTOR_MASK_ALL|MOTOR_MASK_DATE)) {
        motor_date_to_position(motor_pos[date_motor]);
    }
    if(motor_mask & (MOTOR_MASK_ALL|MOTOR_MASK_BAT_WEEK)) {
        motor_battery_week_to_position(motor_pos[battery_week_motor]);
    }
    if(motor_mask & (MOTOR_MASK_ALL|MOTOR_MASK_NOTIFY)) {
        motor_notify_to_position(motor_pos[notify_motor]);
    }
    if(motor_mask & MOTOR_MASK_TRIG) { // zero adjust mode
        motor_trig_handler(0);
    }
}
void motor_set_day_time(clock_t *clock, MOTOR_MASK_E mask)
{
    motor_dst[minute_motor] = clock->minute;
    motor_dst[hour_motor] = clock->hour;
    motor_dst[date_motor] = date[clock->day];
    motor_set_position(motor_dst, mask);
}
void system_pre_reboot_handler(reboot_type_t type)
{
    clock_t* clock = clock_get();

    system_reboot_lock = 1;
    reboot_type = type;
    APP_Move_Bonded(4);
    nvm_write_date_time((u16*)clock, 0);
    nvm_write_motor_current_position((u16*)&motor_dst, 0);
    motor_set_position(motor_zero, MOTOR_MASK_ALL);
    timer_event(100, pre_reboot_handler);
}
void system_post_reboot_handler(void)
{
    clock_t* clock = clock_get();
    
    if(nvm_read_motor_init_flag() == 0) {
        nvm_read_date_time((u16*)clock, 0);
        nvm_read_motor_current_position((u16*)motor_dst, 0);
        motor_dst[minute_motor] = clock->minute;
        motor_dst[hour_motor] = clock->hour;
        motor_dst[date_motor] = date[clock->day];
        get_battery_week_pos();
    } else {
        MemCopy(&motor_dst, motor_zero, max_motor);
    }
    motor_set_position(motor_dst, MOTOR_MASK_ALL);
}
u8 state_machine_check(REPORT_E cb)
{
    if(cb == KEY_M_ULTRA_LONG_PRESS) {
        system_pre_reboot_handler(REBOOT_TYPE_BUTTON);
    } else if(cb >= REPORT_MAX) {
        return 1;
    }
    if(system_reboot_lock == 1) {
        return 1;
    }
    return 0;
}
void timer_event(u16 ms, timer_cb cb)
{
	adapter.drv->timer->timer_start(ms, cb);
}
