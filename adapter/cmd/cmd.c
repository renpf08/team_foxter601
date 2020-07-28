/**
*  @file    m_cmd.c
*  @author  maliwen
*  @date    2020/03/31
*  @history 1.0.0.0(版本号)
*           1.0.0.0: Creat              2020/03/31
*/

#include <mem.h>
#include <buf_utils.h>
#include "serial_service.h"
#include "adapter/adapter.h"

#define   DEFAULT_TARGET_STEPCOUNTS         1000     /*默认目标步数*/

enum {
    STATE_HISDATA_START,
    STATE_HISDATA_DAYS,
    STATE_HISDATA_READ,
    STATE_HISDATA_SEND,
    STATE_SLEEP_SEND,
    STATE_INVALID
};

typedef s16 (* LFPCMDHANDLER)(u8 *buffer, u8 length);

typedef struct cmdEntry_T {
	const cmd_app_send_t cmd;
	REPORT_E report;
	LFPCMDHANDLER handler;
}CMDENTRY, *LPCMDENTRY;

cmd_group_t cmd_group;
static adapter_callback cmd_cb = NULL;

cmd_params_t cmd_params;

static s16 cmd_pairing_code(u8 *buffer, u8 length);
static s16 cmd_user_info(u8 *buffer, u8 length);
static s16 cmd_set_time(u8 *buffer, u8 length);
static s16 cmd_set_alarm_clock(u8 *buffer, u8 length);
static s16 cmd_notify_switch(u8 *buffer, u8 length);
static s16 cmd_sync_data(u8 *buffer, u8 length);
static s16 cmd_app_ack(u8 *buffer, u8 length);
static s16 cmd_recv_notify(u8 *buffer, u8 length);
static s16 cmd_set_pointers(u8 *buffer, u8 length);
static s16 cmd_read_version(u8 *buffer, u8 length);
static s16 cmd_set_clock_hand(u8 *buffer, u8 length);
static s16 cmd_set_vibration(u8 *buffer, u8 length);
static s16 cmd_find_watch(u8 *buffer, u8 length);
static s16 cmd_set_ancs_bond_req(u8 *buffer, u8 length);
static s16 cmd_read_steps_target(u8 *buffer, u8 length);
#if USE_CMD_TEST
static s16 cmd_test(u8 *buffer, u8 length);
#endif

void cmd_parse(u8* content, u8 length);
s16 cmd_init(adapter_callback cb);

static const CMDENTRY cmd_list[] =
{
    {CMD_PAIRING_CODE,      BLE_PAIR,           cmd_pairing_code},
    {CMD_USER_INFO,         WRITE_USER_INFO,    cmd_user_info},
    {CMD_SET_TIME,          SET_TIME,           cmd_set_time},
    {CMD_SET_ALARM_CLOCK,   WRITE_ALARM_CLOCK,  cmd_set_alarm_clock},
    {CMD_NOTIFY_SWITCH,     NOTIFY_SWITCH,      cmd_notify_switch},
    {CMD_SYNC_DATA,         SYNC_DATA,          cmd_sync_data},
    {CMD_APP_ACK,           APP_ACK,            cmd_app_ack},
    {CMD_RECV_NOTIFY,       ANDROID_NOTIFY,     cmd_recv_notify},
    {CMD_SET_POINTERS,      SET_POINTERS,       cmd_set_pointers},
    {CMD_READ_VERSION,      READ_VERSION,       cmd_read_version}, // not use, has moved to DEVICE_INF_SERVICE
    {CMD_SET_CLOCK_POINTER, SET_CLOCK_POINTER,  cmd_set_clock_hand},
    {CMD_SET_VIBRATION,     SET_VIBRATION,      cmd_set_vibration},
    {CMD_SET_FIND_WATCH,    SET_FIND_WATCH,     cmd_find_watch},
    {CMD_SET_ANCS_BOND_REQ, SET_ANCS_BOND_REQ,  cmd_set_ancs_bond_req},
    {CMD_READ_STEPS_TARGET, READ_STEPS_TARGET,  cmd_read_steps_target},
        
    #if USE_CMD_TEST
    {CMD_TEST_RCVD,         REPORT_MAX,         cmd_test},
    #endif
    
	{CMD_APP_NONE,          REPORT_MAX, NULL}
};

#if USE_CMD_TEST
/**
*   CMD_TEST_NVM_ACCESS:
*   AF 00 00 // nvm write one day data
*   AF 00 01 // nvm read all history data
*   AF 00 02 xx // nvm read one day data, index:xx
*   AF 00 03 // nvm erase history data
*   CMD_TEST_ZERO_ADJUST
*   AF 01 00 // KEY_A_B_LONG_PRESS
*   AF 01 01 // KEY_M_SHORT_PRESS
*   AF 01 02 // KEY_A_SHORT_PRESS
*   AF 01 03 // KEY_B_SHORT_PRESS
*   CMD_TEST_STEP_COUNT
*   AF 02 xx xx // xx xx simulate steps
*   CMD_TEST_SYS_REBOOT
*   AF 03 	 // set system reboot
*   CMD_TEST_LOG_EN(xx=1 send; xx=0 not send)
*   AF 04 00 xx // send pair code
*   AF 04 01 xx // send state machine
*   AF 04 02 xx // send zero adjut jump
*/
typedef void (* cmd_test_handler)(u8 *buffer, u8 length);
typedef enum{
    CMD_TEST_NVM_ACCESS     = 0x00,
    CMD_TEST_ZERO_ADJUST    = 0x01,
    CMD_TEST_STEP_COUNT     = 0x02,
    CMD_TEST_SYS_REBOOT     = 0x03,
    CMD_TEST_LOG_TYPE_EN    = 0x04,
    
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
        cmd_cb(KEY_A_B_LONG_PRESS, NULL);
        break;
    case 1:
        cmd_cb(KEY_M_SHORT_PRESS, NULL);
        break;
    case 2:
        cmd_cb(KEY_A_SHORT_PRESS, NULL);
        break;
    case 3:
        cmd_cb(KEY_B_SHORT_PRESS, NULL);
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
    system_pre_reboot(REBOOT_TYPE_BUTTON);
}
#endif
#if USE_CMD_TEST_LOG_TYPE_EN
u8 log_type_en[LOG_SEND_MAX];
static void cmd_test_log_type_en(u8 *buffer, u8 length)
{
    typedef struct{u8 head; u8 cmd; u8 type; u8 en;}cmd_test_log_type_en_t;
    cmd_test_log_type_en_t* log = (cmd_test_log_type_en_t*)buffer;
    log_type_en[log->type] = log->en;
}
#endif
static s16 cmd_test(u8 *buffer, u8 length)
{
    typedef struct{u8 head;u8 cmd;}cmd_test_t;
    cmd_test_t* test = (cmd_test_t*)buffer;
	u8 i = 0;

	if(length == 0) {
		return 0;
	}

    while(cmd_test_list[i].cmd != CMD_TEST_NONE) {
        if(cmd_test_list[i].cmd == test->cmd) {
            cmd_test_list[i].handler(buffer, length);
            break;
        }
        i++;
    }
    
    return 0;
}
#endif // USE_CMD_TEST
static s16 cmd_pairing_code(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.pair_code, buffer, sizeof(cmd_pairing_code_t));
    return 0;
}
static s16 cmd_user_info(u8 *buffer, u8 length)
{
    cmd_group.user_info.target_steps = ((u32)buffer[4]<<24)|((u32)buffer[3]<<16)|((u32)buffer[2]<<8)|((u32)buffer[1]);
    return 0;
}
static s16 cmd_set_time(u8 *buffer, u8 length)
{
    u8 days[13] = {00,31,28,31,30,31,30,31,31,30,31,30,31};
    u16 year = 0;
    volatile cmd_set_time_t* set_time = (cmd_set_time_t*)buffer;

    year = (u16)((set_time->year[1]<<8 & 0xFF00) | set_time->year[0]);
    days[2] = (0 == (year % 400) || (0 == (year % 4) && (0 != (year % 100))))?29:28;

    if((year > 2120) || (year < 2020)) return 1;
    if(set_time->month > 12) return 2;
    if(set_time->day > days[set_time->month]) return 3;
    if(set_time->hour > 23) return 4;
    if(set_time->minute > 59) return 5;
    if(set_time->second > 59) return 6;
    if(set_time->week > 07) return 7;
    
    MemCopy(&cmd_group.set_time, buffer, sizeof(cmd_set_time_t));
    return 0;
}
static s16 cmd_set_alarm_clock(u8 *buffer, u8 length)
{
    cmd_set_alarm_clock_t* alarm_clock = (cmd_set_alarm_clock_t*)buffer;
    u8 i = 0;

    for(i = 0; i < 4; i++) {
        if(alarm_clock->aclk[i].en > 1) return 1;
        if(alarm_clock->aclk[i].hour > 23) return 1;
        if(alarm_clock->aclk[i].minute > 59) return 1;
    }
    
    MemCopy(&cmd_group.set_alarm_clock, buffer, sizeof(cmd_set_alarm_clock_t));
    #if USE_PARAM_STORE
    cmd_cb(WRITE_ALARM_CLOCK, NULL);
    #endif
    return 0;
}
static s16 cmd_notify_switch(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.notify_switch, buffer, sizeof(cmd_notify_switch_t));
    return 0;
}
static s16 cmd_sync_data(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.sync_data, buffer, sizeof(cmd_sync_data_t));
    cmd_group.app_ack.state = STATE_HISDATA_START; // to tell a data-sync is begin
    return 0;
}
static s16 cmd_app_ack(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.app_ack, buffer, (sizeof(cmd_app_ack_t)-1));

    if(cmd_group.app_ack.ack_cmd == 0x01) { // resp to send-data cmd:0x01
        if(cmd_group.app_ack.state == STATE_HISDATA_READ) {
            buffer[1] = 0x00; // 0 to indicate it's a sport data
        }
    }
    
    return cmd_group.app_ack.ack_result;
}
static s16 cmd_recv_notify(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.recv_notif, buffer, sizeof(cmd_recv_notify_t));
    return 0;
}
static s16 cmd_set_pointers(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.set_pointers, buffer, sizeof(cmd_set_pointers_t));
    return 0;
}
static s16 cmd_read_version(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.read_ver, buffer, sizeof(cmd_read_version_t));
    return 0;
}
static s16 cmd_set_clock_hand(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.set_clock_hand, buffer, sizeof(cmd_set_clock_hand_t));
    return 0;
}
static s16 cmd_set_vibration(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.set_vib, buffer, sizeof(cmd_set_vibration_t));
    return 0;
}
static s16 cmd_find_watch(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.find_watch, buffer, sizeof(cmd_find_watch_t));
    return 0;
}
static s16 cmd_set_ancs_bond_req(u8 *buffer, u8 length)
{
    if(buffer[1] == 0xAA) {
        DiscoverServices();
    }
    //MemCopy(&cmd_group.set_ancs_bond, buffer, sizeof(cmd_set_ancs_bond_req_t));  
    return 0;
}
static s16 cmd_read_steps_target(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.read_time_step, buffer, sizeof(cmd_read_steps_target_t)); 
    return 0;
}
u8 cmd_resp(cmd_app_send_t cmd_type, u8 result, u8 *data)
{
    u16 length = 0; 
    u8 rsp_buf[20];
    u8 *tmp_buf = rsp_buf;
    BD_ADDR_T addr;

    CSReadBdaddr(&addr);
    BufWriteUint8((uint8 **)&tmp_buf,0x00);
    BufWriteUint8((uint8 **)&tmp_buf,cmd_type);
    BufWriteUint8((uint8 **)&tmp_buf,result);
    switch(cmd_type) {
        case CMD_USER_INFO:         // 0x01
        case CMD_SET_TIME:          // 0x02
        case CMD_SET_ALARM_CLOCK:   // 0x03
        case CMD_NOTIFY_SWITCH:     // 0x04
        case CMD_RECV_NOTIFY:       // 0x07
        case CMD_SET_POINTERS:      // 0x08(set all clock hands)
        case CMD_SET_CLOCK_POINTER: // 0x0A(set single clock hand)
        case CMD_SET_VIBRATION:     // 0x0B
        case CMD_SET_FIND_WATCH:    // 0x0C
            break; // only need to response 3 bytes: 0x00, cmd type, result
        case CMD_READ_VERSION:      // 0x09(not use, has moved to DEVICE_INF_SERVICE)
        case CMD_SET_ANCS_BOND_REQ: // 0x0A
            return 0; // no need to send response
        case CMD_PAIRING_CODE:      // 0x00
            BufWriteUint16((uint8 **)&tmp_buf, addr.nap);
            BufWriteUint8((uint8 **)&tmp_buf, addr.uap);
            BufWriteUint16((uint8 **)&tmp_buf,( addr.lap));
            BufWriteUint8((uint8 **)&tmp_buf,( addr.lap>>16));
            break;
        case CMD_SYNC_DATA:         // 0x05
        case CMD_APP_ACK:           // 0x06
            tmp_buf = rsp_buf;
            if((result != 0) || (cmd_group.app_ack.state == STATE_INVALID)) {
                return 0;
            }
            switch(cmd_group.app_ack.state) {
                case STATE_HISDATA_START:
                    cmd_cb(READ_HISDAYS, NULL);
                    cmd_group.app_ack.state = STATE_HISDATA_DAYS;
                    //break;
                case STATE_HISDATA_DAYS:
                    BufWriteUint8((uint8 **)&tmp_buf, 0x01);
                    BufWriteUint8((uint8 **)&tmp_buf, 0x01);
                    BufWriteUint8((uint8 **)&tmp_buf, (cmd_params.days+1));  // data size +1:current day
                    cmd_group.app_ack.state = STATE_HISDATA_READ;
                    break;
                case STATE_HISDATA_READ:
                    if(cmd_params.days > 0) {
                        cmd_cb(READ_HISDATA, NULL);
                    } else if(cmd_params.days == 0) { // current day
                        cmd_cb(READ_CURDATA, NULL);
                    } else {
                        cmd_group.app_ack.state = STATE_INVALID;
                        return 0;
                    }
                    BufWriteUint8((uint8 **)&tmp_buf, 0x01);
                    BufWriteUint8((uint8 **)&tmp_buf, 0x02);
                    BufWriteUint8((uint8 **)&tmp_buf, data[0]);
                    BufWriteUint16((uint8 **)&tmp_buf, cmd_params.data->year);//SB100_data.AppApplyDateData.Year);
                    BufWriteUint8((uint8 **)&tmp_buf, cmd_params.data->month);//SB100_data.AppApplyDateData.Month);
                    BufWriteUint8((uint8 **)&tmp_buf, cmd_params.data->day);//SB100_data.AppApplyDateData.Date);
                    BufWriteUint16((uint8 **)&tmp_buf, cmd_params.data->steps);//(SB100_data.AppApplyData.StepCounts));
                    BufWriteUint8((uint8 **)&tmp_buf, cmd_params.data->steps>>16);//(SB100_data.AppApplyData.StepCounts>>16));
                    BufWriteUint16((uint8 **)&tmp_buf, 0);//cmd_get_data->distance);//(SB100_data.AppApplyData.Distance));
                    BufWriteUint8((uint8 **)&tmp_buf, 0);//cmd_get_data->distance>>16);//(SB100_data.AppApplyData.Distance>>16));
                    BufWriteUint16((uint8 **)&tmp_buf, 0);//cmd_get_data->calorie);//(SB100_data.AppApplyData.Calorie));
                    BufWriteUint8((uint8 **)&tmp_buf, 0);//cmd_get_data->calorie>>16);//(SB100_data.AppApplyData.Calorie>>16));
                    BufWriteUint16((uint8 **)&tmp_buf, 0);//cmd_get_data->floor_counts);//SB100_data.AppApplyData.FloorCounts);
                    BufWriteUint16((uint8 **)&tmp_buf, 0);//cmd_get_data->acute_sport_time);//SB100_data.AppApplyData.AcuteSportTimeCounts);
                    if(cmd_params.days >= 0) { // continue to send
                        cmd_group.app_ack.state = STATE_HISDATA_READ;
                        cmd_params.days--;
                    } else { // end to send
                        cmd_group.app_ack.state = STATE_INVALID;
                    }
                    break;
                default:
                    break;
            }
            break;
        case CMD_READ_STEPS_TARGET:   // 0x0E
            BufWriteUint8((uint8 **)&tmp_buf,data[0]);
            if(data[0] == 0x00) { // data & time
                BufWriteUint16((uint8 **)&tmp_buf, cmd_params.clock->year);
                BufWriteUint8((uint8 **)&tmp_buf, cmd_params.clock->month);
                BufWriteUint8((uint8 **)&tmp_buf, cmd_params.clock->day);
                BufWriteUint8((uint8 **)&tmp_buf, cmd_params.clock->hour);
                BufWriteUint8((uint8 **)&tmp_buf, cmd_params.clock->minute);
                BufWriteUint8((uint8 **)&tmp_buf, cmd_params.clock->second);
                BufWriteUint8((uint8 **)&tmp_buf, cmd_params.clock->week);
            } else if(data[0] == 0x01) { // target total steps
                BufWriteUint32((uint8 **)&tmp_buf, &cmd_group.user_info.target_steps);
            } else if(data[0] == 0xFF) { // ANCS connect state
				BufWriteUint8((uint8 **)&tmp_buf, (uint8)(g_app_data.remote_gatt_handles_present));
            }
            break;

        default:
            return 0;
    }
    length = tmp_buf - rsp_buf;
    BLE_SEND_DATA(rsp_buf, length);
    #if USE_UART_PRINT
    print((u8*)&"\xF2\xF2\xF2", 3);
    print(rsp_buf, length);
    #endif
    if((cmd_type == CMD_PAIRING_CODE) && (data[0] == 0xFF) && (data[1] == 0xFF)) {
        rsp_buf[2] = 0xFF;
        BLE_SEND_DATA(rsp_buf, length);
        #if USE_UART_PRINT
        print((u8*)&"\xF3\xF3\xF3", 3);
        print(rsp_buf, length);
        #endif
    }

    return length;
}
void cmd_parse(u8* content, u8 length)
{
	u8 i = 0;
    s16 res = 0;

	if(length == 0) {
		return;
	}

    while(cmd_list[i].cmd != CMD_APP_NONE) {
        if(cmd_list[i].cmd == content[0]) {
            res = cmd_list[i].handler(content, length);
            break;
        }
        i++;
    }
    #if USE_UART_PRINT
    print((u8*)&"\xF1\xF1\xF1", 3);
    print((unsigned char*)content, length);
    #endif

    if((cmd_list[i].cmd != CMD_APP_NONE) && (res == 0)) {
        res = cmd_cb(cmd_list[i].report, NULL);
        cmd_resp(cmd_list[i].cmd, res, &content[1]);
    }
}
cmd_group_t *cmd_get(void)
{
    return &cmd_group;
}
void cmd_check(cmd_group_t *value)
{
    if(value->user_info.target_steps == 0) {
        value->user_info.target_steps = DEFAULT_TARGET_STEPCOUNTS;
    }
    //MemCopy(&cmd_group, value, sizeof(cmd_group_t));
}
cmd_params_t* cmd_get_params(void)
{
    return &cmd_params;
}
s16 cmd_init(adapter_callback cb)
{
	cmd_cb = cb;
    MemSet(&cmd_params, 0, sizeof(cmd_params_t));
    MemSet(&cmd_group, 0, sizeof(cmd_group_t));
    cmd_params.days = -1;
    cmd_group.app_ack.state = STATE_INVALID;
    #if !USE_PARAM_STORE
    cmd_group.user_info.target_steps = DEFAULT_TARGET_STEPCOUNTS;
    #endif
    #if USE_CMD_TEST_LOG_TYPE_EN
    MemSet(log_type_en, 1, LOG_SEND_MAX);
    #endif
    
	return 0;
}
