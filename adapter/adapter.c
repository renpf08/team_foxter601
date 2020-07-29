#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <time.h>
#include <mem.h>
#include "config.h"
#include "adapter.h"
#include <panic.h>
#include <buf_utils.h>
#include <csr_ota.h>

zero_adjust_lock_t zero_adjust_mode = {0, 0};
u16 motor_dst[max_motor] = {0, 0, 0, 0, 0, 0};
u8 reset_supervise_timeout = 0;
reboot_type_t reboot_type = 0;
STATE_BATTERY_WEEK_E bat_week_state = state_battery;
u8 motor_zero[max_motor] = {MINUTE_0, HOUR0_0, ACTIVITY_0, DAY_1, BAT_PECENT_0, NOTIFY_NONE};

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
    u8 activity = 0;

    if(current_steps > target_steps) {
        activity = 40; // total 40 grids
    } else if(current_steps <= target_steps) {
        activity = (current_steps*40)/target_steps;
    } else if(target_steps == 0) {
        activity = 0;
    }
    motor_activity_to_position(activity);
    
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
	motor_minute_to_position(clock->minute);
	motor_hour_to_position(clock->hour);
    motor_date_to_position(date[clock->day]);
}
static void pre_reboot_handler(u16 id)
{
    if((motor_check_idle() != 0) && (reset_supervise_timeout == 0)) {
        timer_event(100, pre_reboot_handler);
        return;
    }

    if(reboot_type == REBOOT_TYPE_BUTTON) {
        Panic(0);
    } else if(reboot_type == REBOOT_TYPE_OTA) {
        OtaReset();
    }
}
static void pre_reboot_supervise_handler(u16 id)
{
    reset_supervise_timeout = 1;
}
static void motor_to_position(void)
{
    motor_minute_to_position(motor_dst[minute_motor]);
    motor_hour_to_position(motor_dst[hour_motor]);
    motor_activity_to_position(motor_dst[activity_motor]);
    motor_date_to_position(motor_dst[date_motor]);
    motor_battery_week_to_position(motor_dst[battery_week_motor]);
    motor_notify_to_position(motor_dst[notify_motor]);
}
void system_pre_reboot_handler(reboot_type_t type)
{
    clock_t* clock = clock_get();
    
    reboot_type = type;
    APP_Move_Bonded(4);
    nvm_write_date_time((u16*)clock, 0);
    nvm_write_motor_current_position((u16*)&motor_dst, 0);
    MemCopy(&motor_dst, motor_zero, max_motor);
    motor_to_position();
    reset_supervise_timeout = 0;
    timer_event(100, pre_reboot_handler);
    timer_event(30*1000, pre_reboot_supervise_handler);
}
void system_post_reboot_handler(void)
{
    clock_t* clock = clock_get();
    
    if(nvm_read_motor_init_flag() == 0) {
        nvm_read_date_time((u16*)clock, 0);
        nvm_read_motor_current_position(motor_dst, 0);
        motor_dst[minute_motor] = clock->minute;
        motor_dst[hour_motor] = clock->hour;
        motor_dst[date_motor] = date[clock->day];
    } else {
        MemCopy(&motor_dst, motor_zero, max_motor);
    }
    #if 0
    motor_dst[0] = 0x01;
    motor_dst[1] = 0x14;
    motor_dst[2] = 0x00;
    motor_dst[3] = 0x02;
    motor_dst[4] = 0x11;
    motor_dst[5] = 0x00;
    #endif
    motor_to_position();
}
void motor_recover_from_zero(void)
{
    motor_battery_week_to_position(motor_dst[battery_week_motor]);    
    motor_activity_to_position(motor_dst[activity_motor]);
}
void timer_event(u16 ms, timer_cb cb)
{
	adapter.drv->timer->timer_start(ms, cb);
}
