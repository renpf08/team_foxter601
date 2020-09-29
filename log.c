
#include <mem.h>
#include "log.h"
#include "adapter/adapter.h"

//-----------------------------------------------------------------------
// define global log variables
//-----------------------------------------------------------------------
//vib_log_t _vib_log_;
    
#define LOG_VAR_SET(param) sizeof(param##_t), &param
#define LOG_VAR_RESERT(param)   MemSet(&((u8*)&param)[2], 0, (sizeof(param)-2));
#define LOG_VAR_DEF(_name, _cmd, _type) \
    static _name##_t _name = {          \
        .head = {                       \
            .cmd = _cmd,                \
            .type = _type,              \
        }                               \
    };
LOG_VAR_DEF(null_log, CMD_TEST_SEND, BLE_LOG_NULL);
LOG_VAR_DEF(vib_log, CMD_TEST_SEND, BLE_LOG_VIB_STATE);
LOG_VAR_DEF(state_log, CMD_TEST_SEND, BLE_LOG_STATE_MACHINE);
log_group_t log_group[] = {
    {1, BLE_LOG_VIB_STATE, LOG_VAR_SET(vib_log)},
    {1, BLE_LOG_STATE_MACHINE, LOG_VAR_SET(state_log)},
    {1, BLE_LOG_MAX, LOG_VAR_SET(null_log)},
};

void log_init(void)
{
    LOG_VAR_RESERT(vib_log);
    LOG_VAR_RESERT(state_log);
}

/** set log enable or disable */
void log_set(log_en_t* log_en)
{
    u8 i = 0;

    if(log_en->boradcast == BLE_LOG_BROADCAST) {
        while(log_group[i].log_type != BLE_LOG_MAX) {
            log_group[i].log_en = log_en->en;
            i++;
        }
    } else {
        while(log_group[i].log_type != BLE_LOG_MAX) {
            if(log_en->type == log_group[i].log_type) {
                log_group[i].log_en = log_en->en;
                break;
            }
            i++;
        }
    }
}

void* log_get(ble_log_type_t log_type)
{
    u8 i = 0;

    while(log_group[i].log_type != BLE_LOG_MAX) {
        if(log_type == log_group[i].log_type) {
            if(log_group[i].log_en == 0) {
                continue;
            }
            return log_group[i].log_ptr;
        }
        i++;
    }
    return log_group[i].log_ptr;
}

void log_send(ble_log_type_t log_type)
{
    u8 i = 0;

    while(log_group[i].log_type != BLE_LOG_MAX) {
        if(log_type == log_group[i].log_type) {
            BLE_SEND_LOG((u8*)log_group[i].log_ptr, log_group[i].log_len);
            break;
        }
        i++;
    }
}


