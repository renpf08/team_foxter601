#ifndef ADAPTER_H
#define ADAPTER_H

#include "../common/common.h"
#include "../driver/driver.h"

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
s16 nvm_read_sport_setting(u16 *buffer, u8 index);
s16 nvm_write_sport_setting(u16 *buffer, u8 index);
s16 nvm_read_sport_data(u16 *buffer, u8 index);
s16 nvm_write_sport_data(u16 *buffer, u8 index);

#endif
