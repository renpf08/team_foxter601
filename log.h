
#ifndef __LOG_H__
#define __LOG_H__

#include <mem.h>
#include "user_config.h"
#include "common/common.h"

#define LOG_PREFIX_LENGTH       sizeof(log_send_head_t)

typedef enum{
    LOG_SEND_NULL               = 0x00,
    LOG_SEND_STATE_MACHINE      = 0x01,
    LOG_SEND_PAIR_CODE          = 0x02,
    LOG_SEND_NOTIFY_TYPE        = 0x03,
    LOG_SEND_ANCS_APP_ID        = 0x04,
    LOG_SEND_GET_CHG_AUTO       = 0x05,
    LOG_SEND_GET_CHG_MANUAL     = 0x06,
    LOG_SEND_COMPASS_ANGLE      = 0x07,
    LOG_SEND_SYSTEM_TIME        = 0x08,
    LOG_SEND_RUN_TIME           = 0x09,
    LOG_SEND_VIB_STATE          = 0x0A,
    
    LOG_SEND_MAX,
}log_send_type_t;

typedef enum {
    LOG_CMD_SEND               = 0x5F,
    LOG_CMD_RCVD                = 0xAF,
    LOG_CMD_MAX
} log_cmd_t;

typedef struct {
    log_cmd_t cmd;
    log_send_type_t type;
    u8 len;
    u8 index;
}log_send_head_t;

typedef struct {
    log_send_head_t head;
    u8 sta_now;
    u8 sta_report;
    u8 sta_next;
    u8 result;
}log_send_sta_mc_t;
typedef struct {
    log_send_head_t head;
    u8 hour_code;
    u8 minute_code;
}log_send_pair_code_t;
typedef struct {
    log_send_head_t head;
    u8 sta_report;
    NOTIFY_E msg_type; 
    u8 result;
}log_send_notify_t;
typedef struct {
    log_send_head_t head;
    u8 recognized;
    u8 app_id[LOG_PREFIX_LENGTH];
}log_send_ancs_id_t;
typedef struct {
    log_send_head_t head;
    u8 chg_sta;
}log_send_chg_sta_t;
typedef struct {
    log_send_head_t head;
    u8 sys_time[8];
}log_send_system_time_t;
typedef struct {
    log_send_head_t head;
    u8 run_time[2];
    u8 swing_lock[2];
}log_send_run_time_t;
typedef struct {
    log_send_head_t head;
    u8 minute_pos;
    u8 hour_pos;
    u8 angle[2]; // BCD format
}log_send_compass_angle_t;
typedef struct {
    log_send_head_t head;
    u8 caller;
    u8 steps;
    u8 cur_step;
    u8 run_flag;
    u8 vib_en;
}log_send_vib_info_t;

typedef struct {
    u8 log_en; // set to non-zero to enable ble log
    log_send_type_t log_type;
}log_send_group_t;

u8 get_vib_en(void);
s16 log_init(adapter_callback cb);
void log_send_initiate(log_send_head_t* log);
#if USE_CMD_TEST
u8 log_rcvd_parse(u8* content, u8 length);
#endif

//extern vib_log_t _vib_log_;
extern log_send_group_t log_send_group[];

#endif // __LOG_H__

