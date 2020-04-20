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
	KEY_A_UP,
	KEY_A_DOWN,
	KEY_B_UP,
	KEY_B_DOWN,
	KEY_M_UP,
	KEY_M_DOWN,
	MAGNETOMETER_READY,
	
	EVENT_MAX,
}EVENT_E;

typedef enum {
	KEY_A_LONG_PRESS,
	KEY_A_SHORT_PRESS,
	KEY_B_LONG_PRESS,
	KEY_B_SHORT_PRESS,
	KEY_M_LONG_PRESS,
	KEY_M_SHORT_PRESS,
	KEY_A_B_LONG_PRESS,
	KEY_A_B_SHORT_PRESS,
	KEY_A_M_LONG_PRESS,
	KEY_A_M_SHORT_PRESS,
	KEY_B_M_LONG_PRESS,
	KEY_B_M_SHORT_PRESS,
	KEY_A_B_M_LONG_PRESS,
	KEY_A_B_M_SHORT_PRESS,
	BATTERY_LOW,
	BATTERY_NORMAL,
	REPORT_MAX,
}REPORT_E;

typedef enum {
	/*week*/
	SUNDAY,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY,

	/*battery*/
	BAT_PECENT_100,
	BAT_PECENT_90,
	BAT_PECENT_80,
	BAT_PECENT_70,
	BAT_PECENT_60,
	BAT_PECENT_50,
	BAT_PECENT_40,
	BAT_PECENT_30,
	BAT_PECENT_20,
	BAT_PECENT_10,
	BAT_PECENT_0,
}BATTERY_WEEK_E;

typedef enum{
	DAY_31,
	DAY_30,
	DAY_29,
	DAY_28,
	DAY_27,
	DAY_26,
	DAY_25,
	DAY_24,
	DAY_23,
	DAY_22,
	DAY_21,
	DAY_20,
	DAY_19,
	DAY_18,
	DAY_17,
	DAY_16,
	DAY_15,
	DAY_14,
	DAY_13,
	DAY_12,
	DAY_11,
	DAY_10,
	DAY_9,
	DAY_8,
	DAY_7,
	DAY_6,
	DAY_5,
	DAY_4,
	DAY_3,
	DAY_2,
	DAY_1,
}DATE_E;

typedef enum {
	ACTIVITY_0,
	ACTIVITY_2_5,
	ACTIVITY_5,
	ACTIVITY_7_5,
	ACTIVITY_10,
	ACTIVITY_12_5,
	ACTIVITY_15,
	ACTIVITY_17_5,
	ACTIVITY_20,
	ACTIVITY_22_5,
	ACTIVITY_25,
	ACTIVITY_27_5,
	ACTIVITY_30,
	ACTIVITY_32_5,
	ACTIVITY_35,
	ACTIVITY_37_5,
	ACTIVITY_40,
	ACTIVITY_42_5,
	ACTIVITY_45,
	ACTIVITY_47_5,
	ACTIVITY_50,
	ACTIVITY_52_5,
	ACTIVITY_55,
	ACTIVITY_57_5,
	ACTIVITY_60,
	ACTIVITY_62_5,
	ACTIVITY_65,
	ACTIVITY_67_5,
	ACTIVITY_70,
	ACTIVITY_72_5,
	ACTIVITY_75,
	ACTIVITY_77_5,
	ACTIVITY_80,
	ACTIVITY_82_5,
	ACTIVITY_85,
	ACTIVITY_87_5,
	ACTIVITY_90,
	ACTIVITY_92_5,
	ACTIVITY_95,
	ACTIVITY_97_5,
	ACTIVITY_100,
}ACTIVITY_E;

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
