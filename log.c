
#include "log.h"
#include "adapter/adapter.h"

//-----------------------------------------------------------------------
// define global log variables
//-----------------------------------------------------------------------
//vib_log_t _vib_log_;

LOG_VAR_DEF(vib_log, CMD_TEST_SEND, BLE_LOG_VIB_STATE, vib_log_t);

log_group_t log_group[] = {
    {BLE_LOG_VIB_STATE, sizeof(vib_log_t), &vib_log},
    {0, 0, NULL},
};

void* log_get(ble_log_type_t log_type)
{
    u8 i = 0;

    for(i = 0; i < sizeof(log_group); i++) {
        if(log_type == log_group[i].log_type) {
            return &log_group[i].log_type;
        }
    }
    return NULL;
}

void log_send(ble_log_type_t log_type)
{
    u8 i = 0;

    for(i = 0; i < sizeof(log_group); i++) {
        if(log_type == log_group[i].log_type) {
            BLE_SEND_LOG((u8*)log_group[i].log_ptr, log_group[i].log_len);
        }
    }
}

void log_init(void)
{
}


