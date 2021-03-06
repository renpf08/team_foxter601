#ifndef ADAPTER_H
#define ADAPTER_H

#include "../ancs_client.h"

#include "../common/common.h"
#include "../driver/driver.h"
#include "../log.h"

typedef struct {
	/*record motor pointer*/
	motor_t *motor_ptr;
	/*record current position*/
	volatile u8 cur_pos;
	/*record the motor dst position*/
	volatile u8 dst_pos;
	/*run flag*/
	u8 run_flag;
	/*unit interval steps*/
	u8 unit_interval_step;
	/*run step state*/
	u8 run_step_state;
}motor_run_status_t;

extern bool ble_send_data(uint8 *data, uint16 size);
#define BLE_SEND_DATA(data, size)   ble_send_data(data, size)
#if USE_BLE_LOG
extern bool ble_send_log(uint8 *data, uint16 size);
#define BLE_SEND_LOG(data, size)    ble_send_log(data, size)
#else
#define BLE_SEND_LOG(data, size)
#endif

void APP_Move_Bonded(uint8 caller);
s16 adapter_init(adapter_callback cb);
void system_pre_reboot_handler(reboot_type_t type);
void system_post_reboot_handler(void);
void charge_check(REPORT_E cb);
void system_reboot(u8 reboot_type);
//void refresh_step(void);
void motor_week_to_position(u8 week);
void sync_time(void);
void motor_restore_position(REPORT_E cb);
#if USE_UART_PRINT
void print(u8 *buf, u16 num);
void trace(u8 *buf, u16 num);
#endif
STATE_E get_last_state(void);
void set_last_state(STATE_E state);
s16 timer_remove(s16 tid);
s16 timer_event(u16 ms, timer_cb cb);

clock_t *clock_get(void);

s16 motor_manager_init(void);
motor_run_status_t *motor_get_status(void);
void motor_run_one_step(u8 motor_num, u8 direction);

/*clock hour:considering the minute position*/
s16 motor_hour_to_position(u8 hour);
/*not clock hour,just hour position*/
s16 motor_pure_hour_to_position(u8 hour);
s16 motor_hour_one_step(u8 hour_step);
//s16 motor_hour_test_run(u8 direction);

s16 motor_minute_to_position(u8 minute);
s16 motor_minute_one_step(u8 minute_step);

s16 motor_date_to_position(u8 day);
s16 motor_notify_to_position(u8 notify);
s16 motor_battery_week_to_position(u8 battery_week);
s16 motor_activity_to_position(u8 activity);

s16 battery_init(adapter_callback cb);
u8 battery_percent_read(void);

s16 nvm_storage_init(adapter_callback cb);
s16 nvm_storage_reset(void);

//void key_click_handler(EVENT_E key_event);
//u8 get_compass_stete(void);
#if USE_PARAM_STORE
s16 nvm_read_motor_init_flag(void);
s16 nvm_write_motor_init_flag(void);
s16 nvm_read_motor_current_position(u16 *buffer, u8 index);
s16 nvm_write_motor_current_position(u16 *buffer, u8 index);
s16 nvm_read_zero_position_polarity(u16 *buffer, u8 index);
s16 nvm_write_zero_position_polarity(u16 *buffer, u8 index);
s16 nvm_read_pairing_code(u16 *buffer, u8 index);
s16 nvm_write_pairing_code(u16 *buffer, u8 index);
s16 nvm_read_date_time(u16 *buffer, u8 index);
s16 nvm_write_date_time(u16 *buffer, u8 index);
s16 nvm_read_alarm_clock_single(u16 *buffer, u8 index);
s16 nvm_write_alarm_clock_single(u16 *buffer, u8 index);
s16 nvm_read_alarm_clock(u16 *buffer, u8 index);
s16 nvm_write_alarm_clock(u16 *buffer, u8 index);
s16 nvm_read_display_setting(u16 *buffer, u8 index);
s16 nvm_write_display_setting(u16 *buffer, u8 index);
s16 nvm_read_personal_info(u16 *buffer, u8 index);
s16 nvm_write_personal_info(u16 *buffer, u8 index);
s16 nvm_read_history_setting(u16 *buffer, u8 index);
s16 nvm_write_history_setting(u16 *buffer, u8 index);
#endif
s16 nvm_read_history_data(u16 *buffer, u8 index);
s16 nvm_write_history_data(u16 *buffer, u8 index);
s16 nvm_erase_history_data(void);
s16 nvm_read_ctrl(his_ctrl_t *ctrl);
s16 nvm_write_ctrl(his_ctrl_t *ctrl);
s16 nvm_read_data(his_data_t *data);
s16 nvm_write_data(his_data_t *data);
u8 nvm_get_days(void);
#if USE_LOG_RCVD_SET_NVM
u8 panic_get(void);
s16 nvm_read_oneday(u8 index);
s16 nvm_read_test(void);
s16 nvm_write_test(void);
#endif

u8 cmd_resp(cmd_app_send_t cmd_type, u8 result, u8 *buffer);
cmd_params_t* cmd_get_params(void);
cmd_group_t *cmd_get(void);
void cmd_check(cmd_group_t *value);

app_msg_t *ancs_get(void);

//void cmd_cb_handler(void);
//void ancs_cb_handler(void);
u8 angle_get(void);
void Update_BodyInfo(uint8 Gender, uint8 Height, uint8 Weight);
void One_Minute_Sport_Info_Pro(clock_t *clock);
u32 step_get(void);
#if USE_LOG_RCVD_SET_STEP_COUNT
void step_test(u32 steps);
#endif
void step_clear(void);
//void clear_minutes_info(void);
s16 sport_get_data(his_data_t *data, clock_t *clock);

void ble_switch_on(void);
void ble_switch_off(void);
void ble_state_set(app_state cur_state);
app_state ble_state_get(void);

s16 vib_stop(void);
s16 vib_run(u8 step_count, u8 caller);
u8 vib_state(void);

s16 charge_status_get(void);

//int sprintf(char *buf, const char * sFormat, ...);
//int printf(const char * sFormat, ...);
//void print_str_hex(u8 *buf, u16 hex_num);
//void print_str_dec(u8 *buf, u16 dec_num);
//void print_date_time(u8 *buf, clock_t *datm);
//u8 bcd_to_hex(u8 bcd_data);
//u32 hex_to_bcd(u8 hex_data);

extern key_sta_ctrl_t key_sta_ctrl;
extern u8 stete_battery_week;
extern const u8 date[];

#endif
