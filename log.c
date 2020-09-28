
#include "log.h"
#include "adapter/adapter.h"

//-----------------------------------------------------------------------
// define global log variables
//-----------------------------------------------------------------------
//vib_log_t _vib_log_;

#define LOG_VAR_DEF(_name, _cmd, _type, _stru)      \
    static _stru _name = {                          \
        .head = {                                   \
            .cmd = _cmd,                            \
            .type = _type,                          \
        }                                           \
    };

LOG_VAR_DEF(vib_log, CMD_TEST_SEND, BLE_LOG_VIB_STATE, vib_log_t);
LOG_VAR_DEF(state_log, CMD_TEST_SEND, BLE_LOG_STATE_MACHINE, state_log_t);

static u8 null_ptr[20] = {CMD_TEST_SEND, BLE_LOG_BROADCAST};
log_group_t log_group[] = {
    {1, BLE_LOG_VIB_STATE, sizeof(vib_log_t), &vib_log},
    {1, BLE_LOG_STATE_MACHINE, sizeof(state_log_t), &state_log},
    {1, BLE_LOG_MAX, 0, NULL},
};

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
                return null_ptr;
            }
            return log_group[i].log_ptr;
        }
        i++;
    }
    return null_ptr;
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

void log_init(void)
{
}


