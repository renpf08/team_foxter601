#ifndef ADAPTER_H
#define ADAPTER_H

#include "../ancs_client.h"

#include "../common/common.h"
#include "../driver/driver.h"

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
u8 get_battery_week_pos(STATE_BATTERY_WEEK_E state);
void motor_set_position(u8* motor_pos, MOTOR_MASK_E motor_mask);
void motor_set_day_time(clock_t *clock, MOTOR_MASK_E mask);
void system_pre_reboot_handler(reboot_type_t type);
void system_post_reboot_handler(void);
void refresh_step(void);
void sync_time(void);
void motor_to_zero(void);
u8 state_machine_check(REPORT_E cb);
#if USE_UART_PRINT
void print(u8 *buf, u16 num);
#endif
void timer_event(u16 ms, timer_cb cb);

clock_t *clock_get(void);

s16 motor_manager_init(void);
void motor_run_one_step(u8 motor_num, u8 direction);
#if 0
void motor_time_adjust_mode_on(void);
void motor_time_adjust_mode_off(void);
void motor_run_one_unit(u8 motor_num, u8 direction);
#endif

s16 motor_hour_to_position(u8 hour);
s16 motor_hour_one_step(u8 hour_step);
//s16 motor_hour_test_run(u8 direction);

s16 motor_minute_to_position(u8 minute);
s16 motor_minute_one_step(u8 minute_step);

s16 motor_date_to_position(u8 day);
s16 motor_notify_to_position(u8 notify);
s16 motor_battery_week_to_position(u8 battery_week);
s16 motor_activity_to_position(u8 activity);

u16 motor_check_idle(void);

s16 battery_init(adapter_callback cb);
u8 battery_percent_read(void);

s16 nvm_init(adapter_callback cb);
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
#if USE_CMD_TEST_NVM_ACCESS
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
#if USE_CMD_TEST_STEP_COUNT
void step_test(u32 steps);
#endif
void step_clear(void);
//void clear_minutes_info(void);
s16 sport_get_data(his_data_t *data, clock_t *clock);

void ble_switch_on(void);
void ble_switch_off(void);
void ble_state_set(app_state cur_state);
app_state ble_state_get(void);

extern u8 activity_pos;
extern u8 motor_zero[max_motor];
extern u8 current_motor_num;
extern u8 current_bat_week_sta;
extern u8 notify_pos;
extern u8 motor_dst[max_motor];
extern zero_adjust_lock_t zero_adjust_mode;
extern const u8 date[];
#if USE_CMD_TEST_LOG_TYPE_EN
extern u8 log_type_en[LOG_SEND_MAX];
#endif

#endif
