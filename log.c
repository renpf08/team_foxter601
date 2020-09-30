
#include "log.h"
#include "adapter/adapter.h"

#define USE_BCD_CONVERT 1
#if USE_BCD_CONVERT
#define HEX_TO_BCD(hex) ((hex % 10) + (hex/10 * 16))
#define BCD_TO_HEX(bcd) ((((bcd >> 4) & 0x0F) * 10) + (bcd & 0x0F))
#endif

#if USE_CMD_TEST
typedef void (* log_rcvd_handler)(u8 *buffer, u8 length);
typedef enum{
    // log to set with params(MSB=0)
    #if USE_LOG_RCVD_NVM_ACCESS
    LOG_RCVD_SET_NVM            = 0x00,
    #endif
    #if USE_LOG_RCVD_ZERO_ADJUST
    LOG_RCVD_SET_ZERO_ADJUST    = 0x01,
    #endif
    #if USE_LOG_RCVD_STEP_COUNT
    LOG_RCVD_SET_STEP_COUNT     = 0x02,
    #endif
    #if USE_LOG_RCVD_SET_LOG_EN
    LOG_RCVD_SET_LOG_EN         = 0x03,
    #endif
    #if USE_LOG_RCVD_CHARGE_SWING
    LOG_RCVD_SET_CHARGE_SWING   = 0x04,
    #endif
    #if USE_LOG_RCVD_VIBRATION
    LOG_RCVD_SET_VIBRATION      = 0x05,
    #endif
    #if USE_LOG_RCVD_VIB_EN
    LOG_RCVD_SET_VIB_EN         = 0x06,
    #endif

    // log to request with no params(MSB=1)
    #if USE_LOG_RCVD_SYS_REBOOT
    LOG_RCVD_REQ_SYS_REBOOT     = 0x80,
    #endif
    #if USE_LOG_RCVD_REQ_CHARGE_STA
    LOG_RCVD_REQ_CHARGE_STA     = 0x81,
    #endif
    #if USE_LOG_RCVD_REQ_SYSTEM_TIME
    LOG_RCVD_REQ_SYSTEM_TIME    = 0x82,
    #endif
    
    LOG_RCVD_NONE,
    LOG_RCVD_BROADCAST          = 0xFF,
}log_rcvd_type_t;
typedef struct {
	log_rcvd_type_t type;
	log_rcvd_handler handler;
} log_rcvd_head_t;

//-------------------------------------------------------------------------
// received log from peer device begin
typedef struct {
    log_cmd_t cmd;
    log_rcvd_type_t type;
} log_rcvd_t;
#if USE_LOG_RCVD_NVM_ACCESS
typedef struct { // LOG_RCVD_SET_NVM     = 0x00,
    log_rcvd_t ctrl; 
    u8 type;    // 00 - write one day data
                // 01 - read all history data
                // 02 - read one day data
                // 03 - read one day data
} log_rcvd_set_nvm_t;
#endif
#if USE_LOG_RCVD_ZERO_ADJUST
typedef struct {
    log_rcvd_t ctrl; // LOG_RCVD_SET_ZERO_ADJUST    = 0x01
    u8 type;    // 00 - KEY_A_B_LONG_PRESS
                // 01 - KEY_M_SHORT_PRESS
                // 02 - KEY_A_SHORT_PRESS
                // 03 - KEY_B_SHORT_PRESS
} log_rcvd_set_zero_adjust_t;
#endif
#if USE_LOG_RCVD_STEP_COUNT
typedef struct { // LOG_RCVD_SET_STEP_COUNT         = 0x02
    log_rcvd_t ctrl; 
    u8 hi;      // hi:lo = set current step count value
    u8 lo;
} log_rcvd_set_step_count_t;
#endif
#if USE_LOG_RCVD_SET_LOG_EN
typedef struct { // LOG_RCVD_SET_LOG_EN             = 0x03
    log_rcvd_t ctrl;
    log_send_type_t log_type; // set to  0xFF to be a broadcasting
    u8 en; // set to 0 or non-zero is ok
} log_rcvd_set_log_en_t;
#endif
#if USE_LOG_RCVD_CHARGE_SWING
typedef struct { // LOG_RCVD_SET_CHARGE_SWING       = 0x04
    log_rcvd_t ctrl;
    u8 act;     // 00 - charge begin and swing
                // 01 - charge swing stop
} log_rcvd_set_chg_swing_t;
#endif
#if USE_LOG_RCVD_VIBRATION
typedef struct { // LOG_RCVD_SET_VIBRATION          = 0x05
    log_rcvd_t ctrl;
    u8 type;    // 00 - set vibration off
                // 01 - set vibration on
    u8 step;    // xx - vib times(if type = 1)
} log_rcvd_set_vibration_t;
#endif
#if USE_LOG_RCVD_VIB_EN
typedef struct { // LOG_RCVD_SET_VIBRATION          = 0x05
    log_rcvd_t ctrl;
    u8 en;      // set to non-zero to enable vibration
} log_rcvd_set_vib_en_t;
#endif
#if USE_LOG_RCVD_NVM_ACCESS
static void log_rcvd_set_nvm(u8 *buffer, u8 length);
#endif
#if USE_LOG_RCVD_ZERO_ADJUST
static void log_rcvd_set_zero_adjust(u8 *buffer, u8 length);
#endif
#if USE_LOG_RCVD_STEP_COUNT
static void log_rcvd_set_step_count(u8 *buffer, u8 length);
#endif
#if USE_LOG_RCVD_SET_LOG_EN
static void log_rcvd_set_log_en(u8 *buffer, u8 length);
#endif
#if USE_LOG_RCVD_CHARGE_SWING
static void log_rcvd_set_charge_swing(u8 *buffer, u8 length);
#endif
#if USE_LOG_RCVD_VIBRATION
static void log_rcvd_set_vibration(u8 *buffer, u8 length);
#endif
#if USE_LOG_RCVD_VIB_EN
static void log_rcvd_set_vib_en(u8 *buffer, u8 length);
#endif

#if USE_LOG_RCVD_SYS_REBOOT
static void log_rcvd_req_sys_reboot(u8 *buffer, u8 length);
#endif
#if USE_LOG_RCVD_REQ_CHARGE_STA
static void log_rcvd_req_charger_sta(u8 *buffer, u8 length);
#endif
#if USE_LOG_RCVD_REQ_SYSTEM_TIME
static void log_rcvd_req_system_time(u8 *buffer, u8 length);
#endif

static log_rcvd_head_t log_rcvd_list[] =
{
    #if USE_LOG_RCVD_NVM_ACCESS
    {LOG_RCVD_SET_NVM,              log_rcvd_set_nvm},
    #endif
    #if USE_LOG_RCVD_ZERO_ADJUST
    {LOG_RCVD_SET_ZERO_ADJUST,      log_rcvd_set_zero_adjust},
    #endif
    #if USE_LOG_RCVD_STEP_COUNT
    {LOG_RCVD_SET_STEP_COUNT,       log_rcvd_set_step_count},
    #endif
    #if USE_LOG_RCVD_SET_LOG_EN
    {LOG_RCVD_SET_LOG_EN,           log_rcvd_set_log_en},
    #endif
    #if USE_LOG_RCVD_CHARGE_SWING
    {LOG_RCVD_SET_CHARGE_SWING,     log_rcvd_set_charge_swing},
    #endif
    #if USE_LOG_RCVD_VIBRATION
    {LOG_RCVD_SET_VIBRATION,        log_rcvd_set_vibration},
    #endif
    #if USE_LOG_RCVD_VIB_EN
    {LOG_RCVD_SET_VIB_EN,           log_rcvd_set_vib_en},
    #endif

    #if USE_LOG_RCVD_SYS_REBOOT
    {LOG_RCVD_REQ_SYS_REBOOT,       log_rcvd_req_sys_reboot},
    #endif
    #if USE_LOG_RCVD_REQ_CHARGE_STA
    {LOG_RCVD_REQ_CHARGE_STA,       log_rcvd_req_charger_sta},
    #endif
    #if USE_LOG_RCVD_REQ_SYSTEM_TIME
    {LOG_RCVD_REQ_SYSTEM_TIME,      log_rcvd_req_system_time},
    #endif

	{LOG_RCVD_NONE,                 NULL}
};
#endif // USE_CMD_TEST
static log_send_group_t log_send_list[] = {
    {1, LOG_SEND_STATE_MACHINE},
    {1, LOG_SEND_PAIR_CODE},
    {1, LOG_SEND_NOTIFY_TYPE},
    {1, LOG_SEND_ANCS_APP_ID},
    {1, LOG_SEND_GET_CHG_AUTO},
    {1, LOG_SEND_GET_CHG_MANUAL},
    {1, LOG_SEND_SYSTEM_TIME},
    {1, LOG_SEND_RUN_TIME},
    {1, LOG_SEND_COMPASS_ANGLE},
    {1, LOG_SEND_VIB_STATE},
    {0, LOG_SEND_MAX},
};

static adapter_callback log_cb = NULL;
static u8 vib_en = 1;

u8 get_vib_en(void)
{
    return vib_en;
}

s16 log_init(adapter_callback cb)
{
	log_cb = cb;

    return 0;
}

void log_send_initiate(log_send_head_t* log_send)
{
    u8 i = 0;
    static u8 index = 0;

    if(log_send->cmd != LOG_CMD_SEND) {
        return;
    } else if(log_send->len == 0) {
        return;
    }

    while(log_send_list[i].log_type <= LOG_SEND_MAX) {
        if(log_send->type == log_send_list[i].log_type) {
            if(log_send_list[i].log_en == 0) {
                return;
            }
            log_send->len = (log_send->len>20)?20:log_send->len;
            log_send->index = index++;
            BLE_SEND_LOG((u8*)log_send, log_send->len);
            break;
        }
        i++;
    }
}
#if USE_LOG_RCVD_NVM_ACCESS
static void log_rcvd_set_nvm(u8 *buffer, u8 length)
{
    log_rcvd_set_nvm_t* log_recv = (log_rcvd_set_nvm_t*)buffer;
    switch(log_recv->type) {
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
#endif
#if USE_LOG_RCVD_ZERO_ADJUST
static void log_rcvd_set_zero_adjust(u8 *buffer, u8 length)
{
    log_rcvd_set_zero_adjust_t* log_recv = (log_rcvd_set_zero_adjust_t*)buffer;
    switch(log_recv->type) {
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
#endif
#if USE_LOG_RCVD_STEP_COUNT
static void log_rcvd_set_step_count(u8 *buffer, u8 length)
{
    log_rcvd_set_step_count_t* log_recv = (log_rcvd_set_step_count_t*)buffer;
    u16 steps = (u16)(log_recv->hi<<8 | log_recv->lo);
    step_test(steps);
}
#endif
#if USE_LOG_RCVD_SET_LOG_EN
static void log_rcvd_set_log_en(u8 *buffer, u8 length)
{
    log_rcvd_set_log_en_t* log_recv = (log_rcvd_set_log_en_t*)buffer;
    u8 i = 0;

    if(log_recv->log_type == LOG_RCVD_BROADCAST) {
        while(log_send_list[i].log_type != LOG_SEND_MAX) {
            log_send_list[i].log_en = (log_recv->en==0)?0:1;
            i++;
        }
    } else {
        while(log_send_list[i].log_type != LOG_SEND_MAX) {
            if(log_recv->log_type == log_send_list[i].log_type) {
                log_send_list[i].log_en = (log_recv->en==0)?0:1;
                break;
            }
            i++;
        }
    }
}
#endif
#if USE_LOG_RCVD_CHARGE_SWING
static void log_rcvd_set_charge_swing(u8 *buffer, u8 length)
{
    log_rcvd_set_chg_swing_t* log_recv = (log_rcvd_set_chg_swing_t*)buffer;

    if(log_recv->act == 0) {
        log_cb(CHARGE_SWING, NULL);
    } else if(log_recv->act == 1) {
        log_cb(CHARGE_STOP, NULL);
    }
}
#endif
#if USE_LOG_RCVD_VIBRATION
static void log_rcvd_set_vibration(u8 *buffer, u8 length)
{
    log_rcvd_set_vibration_t* log_recv = (log_rcvd_set_vibration_t*)buffer;

    if(log_recv->type == 0) {
        vib_stop();
    } else if(log_recv->type == 1) {
        vib_run(log_recv->step, 0x01);
    }
}
#endif
#if USE_LOG_RCVD_VIB_EN
static void log_rcvd_set_vib_en(u8 *buffer, u8 length)
{
    log_rcvd_set_vib_en_t* vib = (log_rcvd_set_vib_en_t*)buffer;

    vib_en = (vib->en==0)?0:1;
}
#endif
#if USE_LOG_RCVD_SYS_REBOOT
static void log_rcvd_req_sys_reboot(u8 *buffer, u8 length)
{
    system_pre_reboot_handler(REBOOT_TYPE_CMD);
}
#endif
#if USE_LOG_RCVD_REQ_CHARGE_STA
static void log_rcvd_req_charger_sta(u8 *buffer, u8 length)
{
    log_send_chg_sta_t log_send = {.head = {LOG_CMD_SEND, LOG_SEND_GET_CHG_MANUAL, sizeof(log_send_chg_sta_t), 0}};
    log_send.chg_sta = charge_status_get();
    log_send_initiate(&log_send.head);
}
#endif
#if USE_LOG_RCVD_REQ_SYSTEM_TIME
static void log_rcvd_req_system_time(u8 *buffer, u8 length)
{
    clock_t* clock = clock_get();
    log_send_system_time_t log_send = {.head = {LOG_CMD_SEND, LOG_SEND_SYSTEM_TIME, sizeof(log_send_system_time_t), 0}};

    #if USE_BCD_CONVERT
    log_send.sys_time[0] = HEX_TO_BCD(clock->year/100);
    log_send.sys_time[1] = HEX_TO_BCD(clock->year%100);
    log_send.sys_time[2] = HEX_TO_BCD(clock->month);
    log_send.sys_time[3] = HEX_TO_BCD(clock->day);
    log_send.sys_time[4] = HEX_TO_BCD(clock->hour);
    log_send.sys_time[5] = HEX_TO_BCD(clock->minute);
    log_send.sys_time[6] = HEX_TO_BCD(clock->second);
    log_send.sys_time[7] = HEX_TO_BCD(clock->week);
    log_send_initiate(&log_send.head);
    #else
    log_send.sys_time[0] = (clock->year>>8) & 0x00FF;
    log_send.sys_time[1] = clock->year & 0x00FF;
    log_send.sys_time[2] = clock->month;
    log_send.sys_time[3] = clock->day;
    log_send.sys_time[4] = clock->hour;
    log_send.sys_time[5] = clock->minute;
    log_send.sys_time[6] = clock->second;
    log_send.sys_time[7] = clock->week;
    log_send_initiate(&log_send.head);
    #endif
}
#endif
#if USE_CMD_TEST
u8 log_rcvd_parse(u8* content, u8 length)
{
    log_rcvd_t* log_recv_head = (log_rcvd_t*)content;
	u8 i = 0;

	if(length == 0) {
		return 1;
	}
    if(log_recv_head->cmd != LOG_CMD_RCVD) {
        return 1;
    }

    while(log_rcvd_list[i].type < LOG_RCVD_NONE) {
        if(log_rcvd_list[i].type == log_recv_head->type) {
            log_rcvd_list[i].handler(content, length);
            return 0;
        }
        i++;
    }
    
    return 1;
}
#endif

