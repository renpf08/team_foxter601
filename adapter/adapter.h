#ifndef ADAPTER_H
#define ADAPTER_H

#include "../common/common.h"
#include "../driver/driver.h"

s16 adapter_init(adapter_callback cb);
s16 adapter_uninit(void);

clock_t *clock_get(void);
s16 clock_set(clock_t *ck);

s16 motor_manager_init(void);
s16 motor_hour_to_position(u8 hour);
s16 motor_minute_to_position(u8 minute);
s16 motor_date_to_position(u8 day);
s16 motor_notify_to_position(u8 notify);
s16 motor_battery_week_to_position(u8 battery_week);
s16 motor_activity_to_position(u8 activity);

s16 battery_init(adapter_callback cb);
u8 battery_percent_read(void);

#endif
