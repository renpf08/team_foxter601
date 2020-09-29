
#include <mem.h>
#include "log.h"
#include "adapter/adapter.h"

#define LOG_SEND_VAR_SET(param) sizeof(param##_t), &param
#define LOG_SEND_VAR_RESERT(param)   MemSet(&((u8*)&param)[2], 0, (sizeof(param)-2));
#define LOG_SEND_VAR_DEF(_name, _cmd, _type) \
    static _name##_t _name = {          \
        .head = {                       \
            .cmd = _cmd,                \
            .type = _type,              \
        }                               \
    };

//-------------------------------------------------------------------------
// received log from peer device begin
typedef struct {
    u8 head;
    u8 cmd;
} log_rcvd_t;
typedef struct { // LOG_RCVD_SET_NVM     = 0x00,
    log_rcvd_t ctrl; 
    u8 type;    // 00 - write one day data
                // 01 - read all history data
                // 02 - read one day data
                // 03 - read one day data
} log_rcvd_set_nvm_t;
typedef struct {
    log_rcvd_t ctrl; // LOG_RCVD_SET_ZERO_ADJUST    = 0x01
    u8 type;    // 00 - KEY_A_B_LONG_PRESS
                // 01 - KEY_M_SHORT_PRESS
                // 02 - KEY_A_SHORT_PRESS
                // 03 - KEY_B_SHORT_PRESS
} log_rcvd_set_zero_adjust_t;
typedef struct { // LOG_RCVD_SET_STEP_COUNT         = 0x02
    log_rcvd_t ctrl; 
    u8 hi;      // hi:lo = set current step count value
    u8 lo;
} log_rcvd_set_step_count_t;
typedef struct { // LOG_RCVD_SET_LOG_EN        = 0x04
    log_rcvd_t ctrl;
    u8 type; 
    u8 en;
} log_rcvd_set_log_en_t;
typedef struct { // LOG_RCVD_SET_VIBRATION          = 0x06
    log_rcvd_t ctrl;
    u8 type;    // 00 - set vibration off
                // 01 - set vibration on
    u8 step;    // xx - vib times(if type = 1)
} log_rcvd_set_vib_t;
typedef struct { // LOG_RCVD_REQ_CHARGE_SWING       = 0x07
    log_rcvd_t ctrl;
    u8 act;     // 00 - charge begin and swing
                // 01 - charge swing stop
} log_rcvd_set_chg_t;

typedef void (* log_rcvd_handler)(u8 *buffer, u8 length);
typedef enum{
    // log to set with params(MSB=0)
    LOG_RCVD_SET_NVM            = 0x00,
    LOG_RCVD_SET_ZERO_ADJUST    = 0x01,
    LOG_RCVD_SET_STEP_COUNT     = 0x02,
    LOG_RCVD_SET_LOG_EN         = 0x03,
    LOG_RCVD_SET_CHARGE_STA     = 0x04,
    LOG_RCVD_SET_VIBRATION      = 0x05,

    // log to request with no params(MSB=1)
    LOG_RCVD_REQ_SYS_REBOOT     = 0x80,
    LOG_RCVD_REQ_CHARGE_SWING   = 0x81,
    
    LOG_RCVD_NONE,
}cmt_test_enum_t;
typedef struct {
	const cmt_test_enum_t cmd;
	log_rcvd_handler handler;
} log_rcvd_entry_t;
static void log_rcvd_set_nvm(u8 *buffer, u8 length);
static void log_rcvd_set_zero_adjust(u8 *buffer, u8 length);
static void log_rcvd_set_step_count(u8 *buffer, u8 length);
static void log_rcvd_set_log_en(u8 *buffer, u8 length);
static void log_rcvd_set_vibration(u8 *buffer, u8 length);
static void log_rcvd_set_charge_swing(u8 *buffer, u8 length);

static void log_rcvd_req_sys_reboot(u8 *buffer, u8 length);
static void log_rcvd_req_charger_sta(u8 *buffer, u8 length);

static const log_rcvd_entry_t log_rcvd_list[] =
{
    {LOG_RCVD_SET_NVM,              log_rcvd_set_nvm},
    {LOG_RCVD_SET_ZERO_ADJUST,      log_rcvd_set_zero_adjust},
    {LOG_RCVD_SET_STEP_COUNT,       log_rcvd_set_step_count},
    {LOG_RCVD_SET_LOG_EN,           log_rcvd_set_log_en},
    {LOG_RCVD_SET_VIBRATION,        log_rcvd_set_vibration},
    {LOG_RCVD_REQ_CHARGE_SWING,     log_rcvd_set_charge_swing},

    {LOG_RCVD_REQ_SYS_REBOOT,       log_rcvd_req_sys_reboot},
    {LOG_RCVD_SET_CHARGE_STA,       log_rcvd_req_charger_sta},

	{LOG_RCVD_NONE,             NULL}
};
// received log from peer device end
//-------------------------------------------------------------------------
// send log from local device begin
LOG_SEND_VAR_DEF(log_send_sta_mc,           LOG_CMD_SEND_DEBUG, LOG_SEND_STATE_MACHINE);
LOG_SEND_VAR_DEF(log_send_pair_code,        LOG_CMD_SEND_DEBUG, LOG_SEND_PAIR_CODE);
LOG_SEND_VAR_DEF(log_send_notify    ,       LOG_CMD_SEND_DEBUG, LOG_SEND_NOTIFY_TYPE);
LOG_SEND_VAR_DEF(log_send_ancs_id,          LOG_CMD_SEND_DEBUG, LOG_SEND_ANCS_APP_ID);

LOG_SEND_VAR_DEF(log_send_vib_info,         LOG_CMD_SEND_DEBUG, LOG_SEND_VIB_STATE);
LOG_SEND_VAR_DEF(log_send_null,             LOG_CMD_SEND_DEBUG, LOG_SEND_NULL);
log_send_group_t log_send_group[] = {
    {1, LOG_SEND_STATE_MACHINE,     LOG_SEND_VAR_SET(log_send_sta_mc)},
    {1, LOG_SEND_PAIR_CODE,         LOG_SEND_VAR_SET(log_send_pair_code)},
    {1, LOG_SEND_NOTIFY_TYPE,       LOG_SEND_VAR_SET(log_send_notify)},
    {1, LOG_SEND_ANCS_APP_ID,       LOG_SEND_VAR_SET(log_send_ancs_id)},
        
    {1, LOG_SEND_VIB_STATE,         LOG_SEND_VAR_SET(log_send_vib_info)},
    {1, LOG_SEND_MAX,               LOG_SEND_VAR_SET(log_send_null)},
};
// send log from local device end
//-------------------------------------------------------------------------

static adapter_callback log_cb = NULL;

s16 log_send_init(adapter_callback cb)
{
	log_cb = cb;
    LOG_SEND_VAR_RESERT(log_send_sta_mc);
    LOG_SEND_VAR_RESERT(log_send_pair_code);
    LOG_SEND_VAR_RESERT(log_send_notify);
    LOG_SEND_VAR_RESERT(log_send_ancs_id);
    
    LOG_SEND_VAR_RESERT(log_send_vib_info);

    log_send_pair_code_t* log = (log_send_pair_code_t*)log_send_get_ptr(LOG_SEND_PAIR_CODE);
    log->hour_code = 0x12;
    log->minute_code = 0x34;
    log_send_initiate(LOG_SEND_PAIR_CODE);
    log = (log_send_pair_code_t*)log_send_get_ptr(LOG_SEND_PAIR_CODE);
    log->hour_code = 0x56;
    log->minute_code = 0x78;
    log_send_initiate(LOG_SEND_PAIR_CODE);

    return 0;
}

//#define LOG_SEND_PTR_RESERT(param)   MemSet(&(u8*)&param[2], 0, (sizeof(param)-2));
//#define LOG_SEND_PTR_RESERT(param, len)   MemSet(&((u8*)param)[2], 0, (len-2));
//static u8 reset_var(void* var, u8 len)
//{
//    volatile u8* ptr = &((u8*)var)[2];
//    ptr[0] = 0x5A;
//    return 0;
//}
#define LOG_SEND_PTR_RESERT(param, len)   MemSet(&((u8*)&param)[2], 0, (len-2));
void* log_send_get_ptr(log_send_type_t log_type)
{
    u8 i = 0;

    while(log_send_group[i].log_type != LOG_SEND_MAX) {
        if(log_type == log_send_group[i].log_type) {
            if(log_send_group[i].log_en == 0) {
                continue;
            }
            //LOG_SEND_VAR_RESERT(((u8*)log_send_group[i].log_ptr)[0]);
            //LOG_SEND_PTR_RESERT(log_send_group[i].log_ptr, log_send_group[i].log_en);
            //reset_var(log_send_group[i].log_ptr, log_send_group[i].log_en);
            return log_send_group[i].log_ptr;
        }
        i++;
    }
    return log_send_group[i].log_ptr;
}

void log_send_initiate(log_send_type_t log_type)
{
    u8 i = 0;

    while(log_send_group[i].log_type != LOG_SEND_MAX) {
        if(log_type == log_send_group[i].log_type) {
            BLE_SEND_LOG((u8*)log_send_group[i].log_ptr, log_send_group[i].log_len);
            break;
        }
        i++;
    }
}

static void log_rcvd_set_nvm(u8 *buffer, u8 length)
{
    log_rcvd_set_nvm_t* adjust = (log_rcvd_set_nvm_t*)buffer;
    switch(adjust->type) {
    case 0:
        nvm_write_test();
        break;
    case 1:
        nvm_read_test();
        break;
    case 2:
        nvm_read_oneday(buffer[1]);
        break;
    case 3:
        nvm_erase_history_data();
        break;
    default:
        break;
    }
}
static void log_rcvd_set_zero_adjust(u8 *buffer, u8 length)
{
    log_rcvd_set_zero_adjust_t* adjust = (log_rcvd_set_zero_adjust_t*)buffer;
    switch(adjust->type) {
    case 0:
        log_cb(KEY_A_B_LONG_PRESS, NULL);
        break;
    case 1:
        log_cb(KEY_M_SHORT_PRESS, NULL);
        break;
    case 2:
        log_cb(KEY_A_SHORT_PRESS, NULL);
        break;
    case 3:
        log_cb(KEY_B_SHORT_PRESS, NULL);
        break;
    default:
        break;
    }
}
static void log_rcvd_set_step_count(u8 *buffer, u8 length)
{
    log_rcvd_set_step_count_t* step = (log_rcvd_set_step_count_t*)buffer;
    u16 steps = (u16)(step->hi<<8 | step->lo);
    step_test(steps);
}

static void log_rcvd_set_log_en(u8 *buffer, u8 length)
{
    log_rcvd_set_log_en_t* log = (log_rcvd_set_log_en_t*)buffer;
    u8 i = 0;

    if(log->en == LOG_SEND_BROADCAST) {
        while(log_send_group[i].log_type != LOG_SEND_MAX) {
            log_send_group[i].log_en = log->en;
            i++;
        }
    } else {
        while(log_send_group[i].log_type != LOG_SEND_MAX) {
            if(log->type == log_send_group[i].log_type) {
                log_send_group[i].log_en = log->en;
                break;
            }
            i++;
        }
    }
}
static void log_rcvd_set_vibration(u8 *buffer, u8 length)
{
    log_rcvd_set_vib_t* vib = (log_rcvd_set_vib_t*)buffer;

    if(vib->type == 0) {
        vib_stop();
    } else if(vib->type == 1) {
        vib_run(vib->step, 0x01);
    }
}
static void log_rcvd_set_charge_swing(u8 *buffer, u8 length)
{
    log_rcvd_set_chg_t* chg = (log_rcvd_set_chg_t*)buffer;

    if(chg->act == 0) {
        log_cb(CHARGE_SWING, NULL);
    } else if(chg->act == 1) {
        log_cb(CHARGE_STOP, NULL);
    }
}
static void log_rcvd_req_sys_reboot(u8 *buffer, u8 length)
{
    system_pre_reboot_handler(REBOOT_TYPE_CMD);
}
static void log_rcvd_req_charger_sta(u8 *buffer, u8 length)
{
    u8 ble_log[3] = {LOG_CMD_SEND_DEBUG, LOG_SEND_CHARGE_STATE, 0};
    ble_log[2] = charge_status_get();
    
    BLE_SEND_LOG(ble_log, 3);
}
void log_rcvd_parse(u8* content, u8 length)
{
    log_rcvd_t* test = (log_rcvd_t*)content;
	u8 i = 0;
    log_cmd_t log_cmd = (log_cmd_t)content[0];

	if(length == 0) {
		return;
	}
    if(log_cmd != LOG_CMD_SEND_DEBUG) {
        return;
    }

    while(log_rcvd_list[i].cmd != LOG_RCVD_NONE) {
        if(log_rcvd_list[i].cmd == test->cmd) {
            log_rcvd_list[i].handler(content, length);
            break;
        }
        i++;
    }
    
    return;
}


