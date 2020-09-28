
#ifndef __LOG_H__
#define __LOG_H__

#include <mem.h>
#include "common/common.h"

#define LOG_VAR_DEF(_name, _cmd, _type, _stru)      \
    static _stru _name = {                          \
        .head = {                                   \
            .length = (sizeof(_stru)-1),            \
            .cmd = _cmd,                            \
            .type = _type,                          \
        }                                           \
    };
    


typedef struct {
    u8 length;
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

void log_send(u8* params);
void log_init(void);

extern vib_log_t _vib_log_;

#endif // __LOG_H__

