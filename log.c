
#include "log.h"
#include "adapter/adapter.h"

//-----------------------------------------------------------------------
// define global log variables
//-----------------------------------------------------------------------
vib_log_t _vib_log_;

LOG_VAR_DEF(vib_log, CMD_TEST_SEND, BLE_LOG_VIB_STATE, vib_log_t);

void log_send(u8* params)
{
    BLE_SEND_LOG((u8*)&params[1], params[0]);
}

void log_init(void)
{
    vib_log.head.cmd = CMD_TEST_SEND;
}


