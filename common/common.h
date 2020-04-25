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

typedef enum {
    CMD_PAIRING_CODE        = 0x00,
    CMD_USER_INFO           = 0x01,
    CMD_SYNC_DATE           = 0x02,
    CMD_SET_ALARM_CLOCK     = 0x03,
    CMD_SET_DISP_FORMAT     = 0x04,
    CMD_SYNC_DATA           = 0x05,
    CMD_RESPONSE_TO_WATCH   = 0x06,
    CMD_SEND_NOTIFY         = 0x07,
    CMD_SET_TIME            = 0x08,
    CMD_READ_VERSION        = 0x09, //! need to response
    CMD_SET_CLOCK_POINTER   = 0x0A,
    CMD_SET_VIBRATION       = 0x0B,
    CMD_SET_FIND_WATCH      = 0x0C,
    CMD_SET_ANCS_BOND_REQ   = 0x0D,
    CMD_READ_TIME_STEPS     = 0x0E,
    
    CMD_APP_NONE            = 0xFF
} cmd_app_send_t;

typedef enum {
    CMD_RESPONS_TO_APP      = 0x00,
    CMD_SEND_MEASURE_DATA   = 0x01,
    CMD_FIND_PHONE          = 0x02,
    
    CMD_WATCH_NONE          = 0xFF
} cmd_watch_send_t;

typedef struct {
    u8 cmd; 
    u8 code[2];
} cmd_pairing_code_t;
typedef struct 
{
    u8 cmd; 
    u8 target_step[4];
    u8 target_dist[4];
    u8 target_calorie[4];
    u8 target_floor[2];
    u8 target_stre_exer[2];
    u8 sex;
    u8 height;
    u8 weight;
} cmd_user_info_t;
typedef struct {
    u8 cmd; 
    u8 year[2];
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
    u8 week;
} cmd_sync_date_t;
typedef struct {
    u8 cmd;
    u8 clock1_alarm_switch;
    union {
        u8 week;
        struct {
            u8 repeat_all:1;
            u8 repeat_sat:1;
            u8 repeat_fri:1;
            u8 repeat_thu:1;
            u8 repeat_wed:1;
            u8 repeat_tue:1;
            u8 repeat_mon:1;
            u8 repeat_sun:1;
        } repeat;
    }clock1_week;
    u8 clock1_hour;
    u8 clock1_minute;
    u8 clock2_alarm_switch;
    union {
        u8 week;
        struct {
            u8 repeat_all:1;
            u8 repeat_sat:1;
            u8 repeat_fri:1;
            u8 repeat_thu:1;
            u8 repeat_wed:1;
            u8 repeat_tue:1;
            u8 repeat_mon:1;
            u8 repeat_sun:1;
        } repeat;
    }clock2_week;
    u8 clock2_hour;
    u8 clock2_minute;
    u8 clock3_alarm_switch;
    union {
        u8 week;
        struct {
            u8 repeat_all:1;
            u8 repeat_sat:1;
            u8 repeat_fri:1;
            u8 repeat_thu:1;
            u8 repeat_wed:1;
            u8 repeat_tue:1;
            u8 repeat_mon:1;
            u8 repeat_sun:1;
        } repeat;
    }clock3_week;
    u8 clock3_hour;
    u8 clock3_minute;
    u8 clock4_alarm_switch;
    union {
        u8 week;
        struct {
            u8 repeat_all:1;
            u8 repeat_sat:1;
            u8 repeat_fri:1;
            u8 repeat_thu:1;
            u8 repeat_wed:1;
            u8 repeat_tue:1;
            u8 repeat_mon:1;
            u8 repeat_sun:1;
        } repeat;
    }clock4_week;
    u8 clock4_hour;
    u8 clock4_minute;
} cmd_set_alarm_clock_t;
typedef struct { 
    u8 cmd; 
    union {
        u8 disp_all;
        struct {
            u8 resv1:1;
            u8 resv2:1;
            u8 resv3:1;
            u8 stren_exer:1;
            u8 floor_level:1;
            u8 calorie:1;
            u8 distance:1;
            u8 clock:1;
        }disp;
    }custm_disp;
    u8 clock_format;
    u8 main_target;
    u8 used_hand;
} cmd_set_disp_format_t;
typedef struct { 
    u8 cmd; 
    u8 sync_data; 
} cmd_sync_data_t;
typedef struct { 
    u8 cmd; 
    u8 watch_cmd;
    u8 resp_value;
} cmd_response_t;
typedef struct { 
    u8 cmd; 
    u8 notif_sta;
    u8 imp_level;
    u8 msg_type;
    u8 msg_cnt;
} cmd_send_notify_t;
typedef struct { 
    u8 cmd; 
    u8 clock_hand1;
    u8 clock_hand1_pos;
    u8 clock_hand2;
    u8 clock_hand2_pos;
    u8 clock_hand3;
    u8 clock_hand3_pos;
    u8 clock_hand4;
    u8 clock_hand4_pos;
    u8 clock_hand5;
    u8 clock_hand5_pos;
    u8 clock_hand6;
    u8 clock_hand6_pos;
} cmd_set_time_t;
typedef struct { 
    u8 cmd; 
    u8 serial_num;
    u8 fw_version;
    u8 system_id;
} cmd_read_version_t;
typedef struct { 
    u8 cmd; 
    u8 clock_hand_num;
    u8 clock_hand_pos;
    u8 clock_hand_rotation;
} cmd_set_clock_hand_t;
typedef struct { 
    u8 cmd; 
    u8 vib_mode;
    u8 vib_times;
} cmd_set_vibration_t;
typedef struct { 
    u8 cmd; 
    u8 alarm_type; 
} cmd_find_watch_t;
typedef struct { 
    u8 cmd; 
    u8 action; 
} cmd_set_ancs_bond_req_t;
typedef struct { 
    u8 cmd; 
    u8 type; 
} cmd_read_time_steps_t;
typedef struct {
    cmd_pairing_code_t pair_code;
    cmd_user_info_t user_info;
    cmd_sync_date_t sync_date;
    cmd_set_alarm_clock_t set_alarm_clock;
    cmd_set_disp_format_t set_disp;
    cmd_sync_data_t sync_data;
    cmd_response_t send_resp;
    cmd_send_notify_t send_notif;
    cmd_set_time_t set_time;
    cmd_read_version_t read_ver;
    cmd_set_clock_hand_t set_clock_hand;
    cmd_set_vibration_t set_vib;
    cmd_find_watch_t find_watch;
    cmd_set_ancs_bond_req_t set_ancs_bond;
    cmd_read_time_steps_t read_time_step;
} cmd_group_t;

typedef struct
{
    u8 cmd; //! fixed to 0x07
    u8 sta; //! fixed to: 0:added, 1:modified, 2:removed
    u8 level; //! 0~255, look appMsgList[] of MESSAGE_POSITION_xxx for details
    u8 type; //! look appMsgList[] of APP_ID_STRING_xxx's index for details
    u8 cnt; //! msg count
} ancs_msg_t;

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
