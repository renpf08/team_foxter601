
#ifndef __LOG_H__
#define __LOG_H__

#include <mem.h>
#include "common/common.h"

#define LOG_VAR_DEF(_name, _cmd, _type, _stru)      \
    static _stru _name = {                          \
        .head = {                                   \
            .cmd = _cmd,                            \
            .type = _type,                          \
        }                                           \
    };
    


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
    ble_log_type_t log_type;
    u8 log_len;
    void* log_ptr;
}log_group_t;

void* log_get(ble_log_type_t log_type);
void log_send(ble_log_type_t log_type);
void log_init(void);

//extern vib_log_t _vib_log_;
extern log_group_t log_group[];

#endif // __LOG_H__

