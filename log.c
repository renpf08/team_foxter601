
#include <mem.h>
#include "log.h"
#include "adapter/adapter.h"

//-----------------------------------------------------------------------
// define global log variables
//-----------------------------------------------------------------------
//vib_log_t _vib_log_;

static adapter_callback log_cb = NULL;
    
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

s16 log_init(adapter_callback cb)
{
	log_cb = cb;
    LOG_VAR_RESERT(vib_log);
    LOG_VAR_RESERT(state_log);

    return 0;
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

#if USE_CMD_TEST
/**
*   CMD_TEST_NVM_ACCESS:
*   AF 00 00 // nvm write one day data
*   AF 00 01 // nvm read all history data
*   AF 00 02 // nvm read one day data
*   AF 00 03 // nvm erase history data
*   CMD_TEST_ZERO_ADJUST
*   AF 01 00 // KEY_A_B_LONG_PRESS
*   AF 01 01 // KEY_M_SHORT_PRESS
*   AF 01 02 // KEY_A_SHORT_PRESS
*   AF 01 03 // KEY_B_SHORT_PRESS
*   CMD_TEST_STEP_COUNT
*   AF 02 xx // xx simulate steps
*   CMD_TEST_SYS_REBOOT
*   AF 03 	 // set system reboot
*   CMD_TEST_LOG_TYPE_EN
*   AF 04 ... 	 // set ble log en
*   ...
*   CMD_TEST_GET_CHARGE_STA
*   AF 05 	 // get charge state
*   CMD_TEST_VIBRATION
*   AF 06 00    // set vibration off
*   AF 06 01 xx // set vibration on, xx: vib step
*   CMD_TEST_CHARGE_SWING
*   AF 07 00    // test charge begin and swing
*   AF 07 01    // test charge stop
*/
typedef void (* cmd_test_handler)(u8 *buffer, u8 length);
typedef enum{
    CMD_TEST_NVM_ACCESS     = 0x00,
    CMD_TEST_ZERO_ADJUST    = 0x01,
    CMD_TEST_STEP_COUNT     = 0x02,
    CMD_TEST_SYS_REBOOT     = 0x03,
    CMD_TEST_LOG_TYPE_EN    = 0x04,
    CMD_TEST_GET_CHARGE_STA = 0x05,
    CMD_TEST_VIBRATION      = 0x06,
    CMD_TEST_CHARGE_SWING   = 0x07,
    
    CMD_TEST_NONE,
}cmt_test_enum_t;
typedef struct {
	const cmt_test_enum_t cmd;
	cmd_test_handler handler;
} cmd_test_entry_t;
#if USE_CMD_TEST_NVM_ACCESS
static void cmd_test_nvm_access(u8 *buffer, u8 length);
#endif
#if USE_CMD_TEST_ZERO_ADJUST
static void cmd_test_zero_adjust(u8 *buffer, u8 length);
#endif
#if USE_CMD_TEST_STEP_COUNT
static void cmd_test_step_count(u8 *buffer, u8 length);
#endif
#if USE_CMD_TEST_SYS_REBOOT
static void cmd_test_sys_reboot(u8 *buffer, u8 length);
#endif
#if USE_CMD_TEST_LOG_TYPE_EN
static void cmd_test_log_type_en(u8 *buffer, u8 length);
#endif
#if USE_CMD_TEST_GET_CHARGE_STA
static void cmd_test_get_charger_sta(u8 *buffer, u8 length);
#endif
#if USE_CMD_TEST_VIBRATION
static void cmd_test_vibration(u8 *buffer, u8 length);
#endif
#if USE_CMD_TEST_CHARGE_SWING
static void cmd_test_charge_swing(u8 *buffer, u8 length);
#endif
static const cmd_test_entry_t cmd_test_list[] =
{
    #if USE_CMD_TEST_NVM_ACCESS
    {CMD_TEST_NVM_ACCESS,   cmd_test_nvm_access},
    #endif
    #if USE_CMD_TEST_ZERO_ADJUST
    {CMD_TEST_ZERO_ADJUST,  cmd_test_zero_adjust},
    #endif
    #if USE_CMD_TEST_STEP_COUNT
    {CMD_TEST_STEP_COUNT,   cmd_test_step_count},
    #endif
    #if USE_CMD_TEST_SYS_REBOOT
    {CMD_TEST_SYS_REBOOT,   cmd_test_sys_reboot},
    #endif
    #if USE_CMD_TEST_LOG_TYPE_EN
    {CMD_TEST_LOG_TYPE_EN, cmd_test_log_type_en},
    #endif
    #if USE_CMD_TEST_GET_CHARGE_STA
    {CMD_TEST_GET_CHARGE_STA, cmd_test_get_charger_sta},
    #endif
    #if USE_CMD_TEST_VIBRATION
    {CMD_TEST_VIBRATION, cmd_test_vibration},
    #endif
    #if USE_CMD_TEST_CHARGE_SWING
    {CMD_TEST_CHARGE_SWING, cmd_test_charge_swing},
    #endif
    
	{CMD_TEST_NONE,         0}
};
#if USE_CMD_TEST_NVM_ACCESS
static void cmd_test_nvm_access(u8 *buffer, u8 length)
{
    typedef struct{u8 head; u8 cmd; u8 type;}cmd_test_nvm_access_t;
    cmd_test_nvm_access_t* adjust = (cmd_test_nvm_access_t*)buffer;
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
#endif
#if USE_CMD_TEST_ZERO_ADJUST
static void cmd_test_zero_adjust(u8 *buffer, u8 length)
{
    typedef struct{u8 head; u8 cmd; u8 type;}cmd_test_zero_adjust_t;
    cmd_test_zero_adjust_t* adjust = (cmd_test_zero_adjust_t*)buffer;
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
#endif
#if USE_CMD_TEST_STEP_COUNT
static void cmd_test_step_count(u8 *buffer, u8 length)
{
    typedef struct{u8 head; u8 cmd; u8 hi; u8 lo;}cmd_test_step_count_t;
    cmd_test_step_count_t* step = (cmd_test_step_count_t*)buffer;
    u16 steps = (u16)(step->hi<<8 | step->lo);
    step_test(steps);
}
#endif
#if USE_CMD_TEST_SYS_REBOOT
static void cmd_test_sys_reboot(u8 *buffer, u8 length)
{
    system_pre_reboot_handler(REBOOT_TYPE_CMD);
}
#endif
#if USE_CMD_TEST_LOG_TYPE_EN
ble_log_type_t ble_log_type[BLE_LOG_MAX];
static void cmd_test_log_type_en(u8 *buffer, u8 length)
{
    typedef struct{u8 head; u8 cmd; u8 type; u8 en;}cmd_test_log_type_en_t;
    cmd_test_log_type_en_t* log = (cmd_test_log_type_en_t*)buffer;
    ble_log_type[log->type] = log->en;
}
#endif
#if USE_CMD_TEST_GET_CHARGE_STA
static void cmd_test_get_charger_sta(u8 *buffer, u8 length)
{
    u8 ble_log[3] = {CMD_TEST_SEND, BLE_LOG_CHARGE_STATE, 0};
    ble_log[2] = charge_status_get();
    
    BLE_SEND_LOG(ble_log, 3);
}
#endif
#if USE_CMD_TEST_VIBRATION
static void cmd_test_vibration(u8 *buffer, u8 length)
{
    typedef struct{u8 head; u8 cmd; u8 type; u8 step;}cmd_test_vib_t;
    cmd_test_vib_t* vib = (cmd_test_vib_t*)buffer;

    if(vib->type == 0) {
        vib_stop();
    } else if(vib->type == 1) {
        vib_run(vib->step, 0x01);
    }
}
#endif
#if USE_CMD_TEST_CHARGE_SWING
static void cmd_test_charge_swing(u8 *buffer, u8 length)
{
    typedef struct{u8 head; u8 cmd; u8 act;}cmd_test_chg_t;
    cmd_test_chg_t* chg = (cmd_test_chg_t*)buffer;

    if(chg->act == 0) {
        log_cb(CHARGE_SWING, NULL);
    } else if(chg->act == 1) {
        log_cb(CHARGE_STOP, NULL);
//        log_cb(KEY_M_SHORT_PRESS, NULL);
    }
}
#endif
void log_parse(u8* content, u8 length)
{
    typedef struct{u8 head;u8 cmd;}cmd_test_t;
    cmd_test_t* test = (cmd_test_t*)content;
	u8 i = 0;

	if(length == 0) {
		return;
	}

    while(cmd_test_list[i].cmd != CMD_TEST_NONE) {
        if(cmd_test_list[i].cmd == test->cmd) {
            cmd_test_list[i].handler(content, length);
            break;
        }
        i++;
    }
    
    return;
}
#endif // USE_CMD_TEST


