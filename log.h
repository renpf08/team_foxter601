
#ifndef __LOG_H__
#define __LOG_H__

#include <mem.h>
#include "common/common.h"

typedef enum{
    LOG_SEND_NULL               = 0x00,
    LOG_SEND_STATE_MACHINE      = 0x01,
    LOG_SEND_PAIR_CODE          = 0x02,
    LOG_SEND_ZERO_ADJUST_JUMP   = 0x03,
    LOG_SEND_NOTIFY_TYPE        = 0x04,
    LOG_SEND_ANCS_APP_ID        = 0x05,
    LOG_SEND_CHARGE_STATE       = 0x06,
    LOG_SEND_MAG_SAMPLE         = 0x07,
    LOG_SEND_COMPASS_ADJ        = 0x08,
    LOG_SEND_SYNC_TIME          = 0x09,
    LOG_SEND_RUN_TIME           = 0x0A,
    LOG_SEND_VIB_STATE          = 0x0B,
    
    LOG_SEND_MAX,
    LOG_SEND_BROADCAST          = 0xFF,
}log_send_type_t;

typedef enum {
    LOG_SEND_FLAG               = 0x5F,
    LOG_RCVD_FLAG               = 0xAF,
    CMD_CMD_MAX
} log_cmd_t;

typedef struct {
    cmd_app_send_t cmd;
    log_send_type_t type;
}log_head_t;

typedef struct {
    log_head_t head;
    u8 sta_now;
    u8 sta_report;
    u8 sta_next;
    u8 result;
}log_send_sta_mc_t;
typedef struct {
    log_head_t head;
    u8 hour_code;
    u8 minute_code;
}log_send_pair_code_t;
typedef struct {
    log_head_t head;
    u8 sta_report;
    NOTIFY_E msg_type; 
    u8 result;
}log_send_notify_t;
typedef struct {
    log_head_t head;
    u8 recognized;
    u8 app_id[17];
}log_send_ancs_id_t;

typedef struct {
    log_head_t head;
    u8 caller;
    u8 steps;
    u8 cur_step;
    u8 run_flag;
    u8 vib_en;
}log_send_vib_info_t;
typedef struct {
    log_head_t head;
    u8 null[18];
}log_send_null_t;

typedef struct {
    u8 log_en; // set to 1 to enable ble log
    log_send_type_t log_type;
    u8 log_len;
    void* log_ptr;
}log_send_group_t;

typedef struct {
    u8 boradcast;
    u8 en;
    log_send_type_t type;
}log_en_t;

s16 log_send_init(adapter_callback cb);
void* log_send_get_ptr(log_send_type_t log_type);
void log_send_initiate(log_send_type_t log_type);

u8 log_rcvd_parse(u8* content, u8 length);

//extern vib_log_t _vib_log_;
extern log_send_group_t log_send_group[];

#endif // __LOG_H__

