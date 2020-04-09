#ifndef COMMON_H
#define COMMON_H

#include "typedef.h"

#define PIO_DIR_OUTPUT  TRUE        /* PIO direction configured as output */
#define PIO_DIR_INPUT   FALSE       /* PIO direction configured as input */

#define BIT_MASK(num) (0x01UL << (num))

typedef struct {
	u8 x_l;
	u8 x_h;
	u8 y_l;
	u8 y_h;
	u8 z_l;
	u8 z_h;
}gsensor_data_t;

typedef struct {
	u8 mag_xl;
	u8 mag_xh;
	u8 mag_yl;
	u8 mag_yh;
	u8 mag_zl;
	u8 mag_zh;
}mag_data_t;

typedef enum {
	KEY_A_DOWN,
	KEY_B_DOWN,
	KEY_M_DOWN,
    RESERVE, /* use for half byte alignment */
	KEY_A_UP,
	KEY_B_UP,
	KEY_M_UP,
	MAGNETOMETER_READY,
	
	EVENT_MAX,
}EVENT_E;

typedef enum {
	KEY_A_SHORT_PRESS,
	KEY_B_SHORT_PRESS,
	KEY_M_SHORT_PRESS,
	KEY_A_B_SHORT_PRESS,
	KEY_A_M_SHORT_PRESS,
	KEY_B_M_SHORT_PRESS,
	KEY_A_B_M_SHORT_PRESS,
	KEY_A_LONG_PRESS,
	KEY_B_LONG_PRESS,
	KEY_M_LONG_PRESS,
	KEY_A_B_LONG_PRESS,
	KEY_A_M_LONG_PRESS,
	KEY_B_M_LONG_PRESS,
	KEY_A_B_M_LONG_PRESS,
	REPORT_MAX,
}REPORT_E;

typedef enum {
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY,
	SUNDAY,
}WEEK_E;

typedef struct {
	u16 year;
	u8 month;	
	u8 day;
	u8 week;
	u8 hour;
	u8 minute;
	u8 second;
}clock_t;

enum {
	false = 0,
	true = 1,
};

typedef struct {
	u8 group;
	u8 num;
}pin_t;

typedef struct {
	pin_t tx;
	pin_t rx;
}uart_cfg_t;

typedef struct {
	pin_t clk;
	pin_t mosi;
	pin_t miso;
	pin_t cs;
	pin_t int1;
	pin_t int2;
}gsensor_cfg_t;

typedef struct {
	pin_t scl;
	pin_t sda;
	pin_t int1;
}magnetometer_cfg_t;

typedef struct {
	pin_t pos;
	pin_t com;
	pin_t neg;
}motor_cfg_t;

typedef struct {
	uart_cfg_t uart_cfg;
	pin_t      keya_cfg;
	pin_t      keym_cfg;
	pin_t      keyb_cfg;
	pin_t      vibrator_cfg;
	gsensor_cfg_t gsensor_cfg;
	magnetometer_cfg_t magnetometer_cfg;
	motor_cfg_t motor_hour_cfg;
	motor_cfg_t motor_minute_cfg;
	motor_cfg_t motor_activity_cfg;	
	motor_cfg_t motor_date_cfg;
	motor_cfg_t motor_battery_week_cfg;	
	motor_cfg_t motor_notify_cfg;
}cfg_t;

typedef s16 (*event_callback)(EVENT_E ev);
typedef s16 (*adapter_callback)(REPORT_E cb, void *args);
typedef s16 (*driver_callback_handler)(void *args);

typedef s16 (*init)(cfg_t *args, event_callback cb);
typedef s16 (*uninit)(void);
typedef s16 (*read)(void *args);
typedef s16 (*write)(u8 *buf, u16 num);

typedef s16 (*on)(void *args);
typedef s16 (*off)(void *args);

typedef s16 (*positive)(void *args);
typedef s16 (*negtive)(void *args);
typedef s16 (*stop)(void *args);

typedef s16 (*fread)(u16 *buffer, u16 length, u16 offset);
typedef s16 (*fwrite)(u16 *buffer, u16 length, u16 offset);

typedef void(*timer_cb)(u16 id);
typedef s16 (*timer_start_func)(u16 ms, timer_cb cb);

#endif
