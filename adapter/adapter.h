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

s16 adapter_init(adapter_callback cb);
void print(u8 *buf, u16 num);
void timer_event(u16 ms, timer_cb cb);

clock_t *clock_get(void);
s16 clock_set(clock_t *ck);

s16 motor_manager_init(void);
void motor_run_one_step(u8 motor_num, u8 direction);

s16 motor_hour_to_position(u8 hour);
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

s16 nvm_check_storage_init(void);
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
s16 nvm_read_alarm_clock_total(u16 *buffer, u8 index);
s16 nvm_write_alarm_clock_total(u16 *buffer, u8 index);
s16 nvm_read_display_setting(u16 *buffer, u8 index);
s16 nvm_write_display_setting(u16 *buffer, u8 index);
s16 nvm_read_personal_info(u16 *buffer, u8 index);
s16 nvm_write_personal_info(u16 *buffer, u8 index);
s16 nvm_read_history_setting(u16 *buffer, u8 index);
s16 nvm_write_history_setting(u16 *buffer, u8 index);
s16 nvm_read_history_data(u16 *buffer, u8 index);
s16 nvm_write_step_data(u16 *buffer, u8 index);
s16 nvm_write_sleep_data(u16 *buffer, u8 index);
s16 nvm_erase_history_data(void);

u8 cmd_resp(cmd_app_send_t cmd_type, u8 result, u8 *buffer);
s16 cmd_refresh_time(clock_t *ck);
cmd_group_t *cmd_get(void);

app_msg_t *ancs_get(void);

//void cmd_cb_handler(void);
//void ancs_cb_handler(void);
u8 angle_get(void);
u32 step_get(void);

void ble_switch_on(void);
void ble_switch_off(void);
void ble_state_set(app_state cur_state);
app_state ble_state_get(void);

//int sprintf(char *buf, const char * sFormat, ...);
//int printf(const char * sFormat, ...);
//void print_str_hex(u8 *buf, u16 hex_num);
//void print_str_dec(u8 *buf, u16 dec_num);
//void print_date_time(u8 *buf, clock_t *datm);
u8 bcd_to_hex(u8 bcd_data);
u32 hex_to_bcd(u8 hex_data);

extern const u8 date[];

#endif
