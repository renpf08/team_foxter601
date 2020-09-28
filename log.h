
#ifndef __LOG_H__
#define __LOG_H__

#include <mem.h>
#include "common/common.h"

//#if USE_CMD_TEST_LOG_TYPE_EN
typedef enum{
    BLE_LOG_PAIR_CODE           = 0x00,
    BLE_LOG_STATE_MACHINE       = 0x01,
    BLE_LOG_ZERO_ADJUST_JUMP    = 0x02,
    BLE_LOG_NOTIFY_TYPE         = 0x03,
    BLE_LOG_ANCS_APP_ID         = 0x04,
    BLE_LOG_CHARGE_STATE        = 0x05,
    BLE_LOG_MAG_SAMPLE          = 0x06,
    BLE_LOG_COMPASS_ADJ         = 0x07,
    BLE_LOG_SYNC_TIME           = 0x08,
    BLE_LOG_RUN_TIME            = 0x09,
    BLE_LOG_VIB_STATE           = 0x0A,
    
    BLE_LOG_MAX,
    BLE_LOG_BROADCAST           = 0xFF,
}ble_log_type_t;
//#endif

typedef struct {
    cmd_app_send_t cmd;
    ble_log_type_t type;
}log_head_t;

typedef struct {
    log_head_t head;
    u8 caller;
    u8 steps;
    u8 cur_step;
    u8 run_flag;
    u8 vib_en;
}vib_log_t;
typedef struct {
    log_head_t head;
    u8 sta_now;
    u8 report;
    u8 sta_next;
    u8 result;
}state_log_t;

typedef struct {
    u8 log_en; // set to 1 to enable ble log
    ble_log_type_t log_type;
    u8 log_len;
    void* log_ptr;
}log_group_t;

typedef struct {
    u8 boradcast;
    u8 en;
    ble_log_type_t type;
}log_en_t;

void log_set(log_en_t* log_en);
void* log_get(ble_log_type_t log_type);
void log_send(ble_log_type_t log_type);
void log_init(void);

//extern vib_log_t _vib_log_;
extern log_group_t log_group[];

#endif // __LOG_H__

