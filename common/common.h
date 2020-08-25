#ifndef COMMON_H
#define COMMON_H

#include "typedef.h"
#include "user_config.h"

#define PIO_DIR_OUTPUT  TRUE        /* PIO direction configured as output */
#define PIO_DIR_INPUT   FALSE       /* PIO direction configured as input */

#define BIT_MASK(num) (0x01UL << (num))

#define VAL_GET(num)        PioGet(num)

#define QUEUE_BUFFER    64

typedef struct {
	u8 x_l;
	u8 x_h;
	u8 y_l;
	u8 y_h;
	u8 z_l;
	u8 z_h;
}gsensor_data_t;

typedef struct {
	u8 mag_xh;
	u8 mag_xl;
	u8 mag_yh;
	u8 mag_yl;
	u8 mag_zh;
	u8 mag_zl;
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
	KEY_A_LONG_PRESS = 0,
	KEY_A_SHORT_PRESS = 1,
	KEY_B_LONG_PRESS = 2,
	KEY_B_SHORT_PRESS = 3,
	KEY_M_LONG_PRESS = 4,
	KEY_M_SHORT_PRESS = 5,
	KEY_A_B_LONG_PRESS = 6,
	KEY_A_B_SHORT_PRESS = 7,
	KEY_A_M_LONG_PRESS = 8,
	KEY_A_M_SHORT_PRESS = 9,
	KEY_B_M_LONG_PRESS = 10,
	KEY_B_M_SHORT_PRESS = 11,
	KEY_A_B_M_LONG_PRESS = 12,
	KEY_A_B_M_SHORT_PRESS = 13,
	KEY_M_ULTRA_LONG_PRESS = 14,
	BATTERY_LOW = 15,
	BATTERY_NORMAL = 16,
	CLOCK_1_MINUTE = 17,
	ANCS_NOTIFY_INCOMING = 18,
	ANDROID_NOTIFY = 19,
	BLE_CHANGE = 20,
    BLE_PAIR = 21,
    WRITE_USER_INFO = 22,
    SET_TIME = 23,
    WRITE_ALARM_CLOCK = 24,
    NOTIFY_SWITCH = 25,
    SYNC_DATA = 26,
    APP_ACK = 27,
    SEND_NOTIFY = 28,
    SET_POINTERS = 29,
    READ_VERSION = 30,
    SET_CLOCK_POINTER = 31,
    SET_VIBRATION = 32,
    SET_FIND_WATCH = 33,
    SET_ANCS_BOND_REQ = 34,
    READ_STEPS_TARGET = 35,
    READ_HISDAYS = 36,
    READ_HISDATA = 37,
    READ_CURDATA = 38,
    REFRESH_STEPS = 39,
    #if USE_PARAM_STORE
    READ_SYS_PARAMS = 40,
    #endif
	REPORT_MAX,
}REPORT_E;

typedef enum {
	/*week*/
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY,
	SUNDAY,

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
	DAY_0,
}DATE_E;

typedef enum {
	HOUR0_0, HOUR0_2, HOUR0_4, HOUR0_6,	HOUR0_8,
	HOUR1_0, HOUR1_2, HOUR1_4, HOUR1_6,	HOUR1_8,
	HOUR2_0, HOUR2_2, HOUR2_4, HOUR2_6,	HOUR2_8,
	HOUR3_0, HOUR3_2, HOUR3_4, HOUR3_6, HOUR3_8,
	HOUR4_0, HOUR4_2, HOUR4_4, HOUR4_6, HOUR4_8,
	HOUR5_0, HOUR5_2, HOUR5_4, HOUR5_6, HOUR5_8,
	HOUR6_0, HOUR6_2, HOUR6_4, HOUR6_6,	HOUR6_8,
	HOUR7_0, HOUR7_2, HOUR7_4, HOUR7_6,	HOUR7_8,
	HOUR8_0, HOUR8_2, HOUR8_4, HOUR8_6,	HOUR8_8,
	HOUR9_0, HOUR9_2, HOUR9_4, HOUR9_6, HOUR9_8,
	HOUR10_0, HOUR10_2, HOUR10_4, HOUR10_6, HOUR10_8,
	HOUR11_0, HOUR11_2, HOUR11_4, HOUR11_6, HOUR11_8,
	HOUR12_0,
}HOUR_E;

typedef enum {
	MINUTE_0, MINUTE_1, MINUTE_2, MINUTE_3,	MINUTE_4,
	MINUTE_5, MINUTE_6, MINUTE_7, MINUTE_8, MINUTE_9,
	MINUTE_10, MINUTE_11, MINUTE_12, MINUTE_13, MINUTE_14,
	MINUTE_15, MINUTE_16, MINUTE_17, MINUTE_18, MINUTE_19,
	MINUTE_20, MINUTE_21, MINUTE_22, MINUTE_23, MINUTE_24,
	MINUTE_25, MINUTE_26, MINUTE_27, MINUTE_28, MINUTE_29,
	MINUTE_30, MINUTE_31, MINUTE_32, MINUTE_33,	MINUTE_34,
	MINUTE_35, MINUTE_36, MINUTE_37, MINUTE_38, MINUTE_39,
	MINUTE_40, MINUTE_41, MINUTE_42, MINUTE_43, MINUTE_44,
	MINUTE_45, MINUTE_46, MINUTE_47, MINUTE_48, MINUTE_49,
	MINUTE_50, MINUTE_51, MINUTE_52, MINUTE_53, MINUTE_54,
	MINUTE_55, MINUTE_56, MINUTE_57, MINUTE_58, MINUTE_59,
	MINUTE_60,
}MINUTE_E;

typedef enum {
	NOTIFY_NONE = 0,
	NOTIFY_SKYPE = 1,       // mask bit:8
	NOTIFY_WHATSAPP = 2,    // mask bit:10
	NOTIFY_TWITTER = 3,     // mask bit:9
	NOTIFY_EMAIL = 4,       // mask bit:2
	NOTIFY_FACEBOOK = 5,    // mask bit:5
	NOTIFY_SMS = 6,         // mask bit:1
	NOTIFY_LINKIN = 7,      // mask bit:12
	NOTIFY_COMMING_CALL = 8,// mask bit:0
	NOTIFY_DONE,
	NOTIFY_LINE,
	NOTIFY_QQ,
	NOTIFY_FACEMESSAGE,
	NOTIFY_WECHAT,
	NOTIFY_CALENDER,
	NOTIFY_NEWS,
	NOTIFY_MAX,
}NOTIFY_E;

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

typedef enum {
	INIT = 0,
	CLOCK = 1,
	ZERO_ADJUST = 2,
	LOW_VOLTAGE = 3,
	BLE_SWITCH = 4,
	BLE_CHANGING = 5,
	NOTIFY_COMING = 6,
	BATTERY_WEEK_SWITCH = 7,
	TIME_ADJUST = 8,
	RUN_TEST = 9,
	SET_DATE_TIME = 10,
	NVM_ACCESS = 11,
	SYSTEM_REBOOT = 12,
	STATE_MAX,
}STATE_E;

typedef enum {
	state_battery,
	state_week,
}STATE_BATTERY_WEEK_E;

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

enum {
	pos = 0,
	neg = 1,
};

enum{
	run,
	idle,
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

typedef struct
{
    u8 cmd; //! fixed to 0x07
    u8 sta; //! fixed to: 0:added, 1:modified, 2:removed
    u8 level; //! 0~255, look appMsgList[] of MESSAGE_POSITION_xxx for details
    u8 type; //! look appMsgList[] of APP_ID_STRING_xxx's index for details
    u8 cnt; //! msg count
} app_msg_t;
typedef app_msg_t cmd_recv_notify_t;

#if USE_CMD_TEST_LOG_TYPE_EN
typedef enum{
    BLE_LOG_PAIR_CODE           = 0x00,
    BLE_LOG_STATE_MACHINE       = 0x01,
    BLE_LOG_ZERO_ADJUST_JUMP    = 0x02,
    BLE_LOG_NOTIFY_TYPE         = 0x03,
    BLE_LOG_ANCS_APP_ID         = 0x04,
    
    BLE_LOG_MAX,
}ble_log_type_t;
#endif

typedef enum {
    CMD_PAIRING_CODE        = 0x00,
    CMD_USER_INFO           = 0x01,
    CMD_SET_TIME            = 0x02,
    CMD_SET_ALARM_CLOCK     = 0x03,
    CMD_NOTIFY_SWITCH       = 0x04,
    CMD_SYNC_DATA           = 0x05,
    CMD_APP_ACK             = 0x06,
    CMD_RECV_NOTIFY         = 0x07,
    CMD_SET_POINTERS        = 0x08, //! set all clock hands
    CMD_READ_VERSION        = 0x09, //! not use, has moved to DEVICE_INF_SERVICE
    CMD_SET_CLOCK_POINTER   = 0x0A, //! set single clock hand
    CMD_SET_VIBRATION       = 0x0B,
    CMD_SET_FIND_WATCH      = 0x0C,
    CMD_SET_ANCS_BOND_REQ   = 0x0D,
    CMD_READ_STEPS_TARGET   = 0x0E,

    CMD_TEST_SEND           = 0x5F,
    CMD_TEST_RCVD           = 0xAF,
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
    u32 target_steps;
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
} cmd_set_time_t;
typedef struct {
    u8 en;
    u8 week;
    u8 hour;
    u8 minute;
} alarm_clock_t;
typedef struct {
    u8 cmd;
    alarm_clock_t aclk[4];
} cmd_set_alarm_clock_t;
typedef struct { 
    u8 cmd;
    u8 en[4];
} cmd_notify_switch_t;
typedef struct { 
    u8 cmd; 
    u8 sync_data; 
} cmd_sync_data_t;

typedef struct { 
    u8 cmd; 
    u8 ack_cmd;
    u8 ack_result;
    u8 state; // not include in protocal
} cmd_app_ack_t;

//typedef struct { 
//    u8 cmd; 
//    u8 notif_sta;
//    u8 imp_level;
//    u8 msg_type;
//    u8 msg_cnt;
//} cmd_recv_notify_t;

typedef struct { 
    u8 cmd; 
    u8 hour_pointer;
    u8 hour_pointer_pos;
    u8 minute_pointer;
    u8 minute_pointer_pos;
    u8 week_pointer;
    u8 week_pointer_pos;
    u8 day_pointer;
    u8 day_pointer_pos;
    u8 battery_pointer;
    u8 battery_pointer_pos;
    u8 notify_pointer;
    u8 notify_pointer_pos;
} cmd_set_pointers_t;

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
} cmd_read_steps_target_t;

typedef struct {
    cmd_pairing_code_t pair_code;
    cmd_user_info_t user_info;
    cmd_set_time_t set_time;
    cmd_set_alarm_clock_t set_alarm_clock;
    cmd_notify_switch_t notify_switch;
    cmd_sync_data_t sync_data;
    cmd_app_ack_t app_ack;
    cmd_recv_notify_t recv_notif;
    cmd_set_pointers_t set_pointers;
    cmd_read_version_t read_ver;
    cmd_set_clock_hand_t set_clock_hand;
    cmd_set_vibration_t set_vib;
    cmd_find_watch_t find_watch;
    cmd_set_ancs_bond_req_t set_ancs_bond;
    cmd_read_steps_target_t read_time_step;
} cmd_group_t;

enum {
    READ_HISDATA_TOTAL = 0xFE,
    READ_HISDATA_LAST = 0xFF
};
typedef struct {
    u8 ring_buf_tail; /* sport data ring buffer tail */
    u8 ring_buf_head; /* sport data ring buffer head */
    u8 read_tail;
    u8 write_head;
}his_ctrl_t; /* for nvm to store */
typedef struct {
    u16 year;
    u8 month;
    u8 day;
    u32 steps;
}his_data_t; /* for nvm to store */
typedef struct {
    clock_t *clock;
    his_data_t *data;
    u32 steps;
    s16 days;
} cmd_params_t;
typedef struct {
    u8 press;
    u8 run;
} zero_adjust_lock_t;

typedef s16 (*event_callback)(EVENT_E ev);
typedef s16 (*adapter_callback)(REPORT_E cb, void *args);
typedef s16 (*driver_callback_handler)(void *args);

typedef s16 (*init)(cfg_t *args, event_callback cb);
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

typedef s16 (*state_func)(REPORT_E cb, void *args);

enum {
	minute_motor = 0,
	hour_motor = 1,
	activity_motor = 2,
	date_motor = 3,
	battery_week_motor = 4,
	notify_motor = 5,
	max_motor,
};

/* enum for event id */
typedef enum 
{
	NOTIFY_ADD = 0,
	NOTIFY_MODIFY = 1,
	NOTIFY_REMOVE = 2,
	NOTIFY_RESERVE = 3,
}NOTIFY_STATE_E;

typedef struct {
	STATE_E   init_state;
	REPORT_E  ev;
	STATE_E   next_state;
	state_func func;
}state_t;

typedef struct {
	pin_t pin;
	u8 flag;
	event_callback key_cb;
	EVENT_E last_state;
	EVENT_E now_state;
}csr_key_cfg_t;

#define POS_HIGH(num) PioSet((num), 1UL)
#define POS_LOW(num) PioSet((num), 0UL)

#define COM_HIGH(num) PioSet((num), 1UL)
#define COM_LOW(num) PioSet((num), 0UL)

#define NEG_HIGH(num) PioSet((num), 1UL)
#define NEG_LOW(num) PioSet((num), 0UL)

#endif
