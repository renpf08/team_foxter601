#ifndef ADAPTER_H
#define ADAPTER_H

#include "ancs_service_data.h"
#include "../common/common.h"
#include "../driver/driver.h"

#define USB_NVM_TEST    1

s16 adapter_init(adapter_callback cb);
s16 adapter_uninit(void);


clock_t *clock_get(void);
s16 clock_set(clock_t *ck);

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
s16 nvm_read_test(void);
s16 nvm_write_test(void);

void cmd_dispatch(char* content, uint8 length);

void ancs_data_source_handle(uint8 *p_data, uint16 size_value, data_source_t *p_data_source);
void ancs_noti_source_handle(GATT_CHAR_VAL_IND_T *p_ind, noti_t *p_noti_source);

#endif
