/**
*  @file    m_cmd.c
*  @author  maliwen
*  @date    2020/03/31
*  @history 1.0.0.0(°æ±¾ºÅ)
*           1.0.0.0: Creat              2020/03/31
*/

#include <mem.h>
#include <buf_utils.h>
#include "serial_service.h"
#include "adapter/adapter.h"

typedef s16 (* LFPCMDHANDLER)(u8 *buffer, u8 length);

typedef struct cmdEntry_T {
	const cmd_app_send_t cmd;
	REPORT_E report;
	LFPCMDHANDLER handler;
}CMDENTRY, *LPCMDENTRY;

cmd_group_t cmd_group;
static adapter_callback cmd_cb = NULL;
static clock_t *cmd_time;

static s16 cmd_pairing_code(u8 *buffer, u8 length);
static s16 cmd_user_info(u8 *buffer, u8 length);
static s16 cmd_set_time(u8 *buffer, u8 length);
static s16 cmd_set_alarm_clock(u8 *buffer, u8 length);
static s16 cmd_notify_switch(u8 *buffer, u8 length);
static s16 cmd_sync_data(u8 *buffer, u8 length);
static s16 cmd_response(u8 *buffer, u8 length);
static s16 cmd_recv_notify(u8 *buffer, u8 length);
static s16 cmd_set_pointers(u8 *buffer, u8 length);
static s16 cmd_read_version(u8 *buffer, u8 length);
static s16 cmd_set_clock_hand(u8 *buffer, u8 length);
static s16 cmd_set_vibration(u8 *buffer, u8 length);
static s16 cmd_find_watch(u8 *buffer, u8 length);
static s16 cmd_set_ancs_bond_req(u8 *buffer, u8 length);
static s16 cmd_read_time_steps(u8 *buffer, u8 length);
void cmd_parse(u8* content, u8 length);
s16 cmd_init(adapter_callback cb);

static const CMDENTRY cmd_list[] =
{
    {CMD_PAIRING_CODE,      BLE_PAIR,           cmd_pairing_code},
    {CMD_USER_INFO,         USER_INFO,          cmd_user_info},
    {CMD_SET_TIME,          SET_TIME,           cmd_set_time},
    {CMD_SET_ALARM_CLOCK,   SET_ALARM_CLOCK,    cmd_set_alarm_clock},
    {CMD_NOTIFY_SWITCH,     NOTIFY_SWITCH,      cmd_notify_switch},
    {CMD_SYNC_DATA,         SYNC_DATA,          cmd_sync_data},
    {CMD_RESPONSE_TO_WATCH, RESPONSE_TO_WATCH,  cmd_response},
    {CMD_RECV_NOTIFY,       ANDROID_NOTIFY,     cmd_recv_notify},
    {CMD_SET_POINTERS,      SET_POINTERS,       cmd_set_pointers},
    {CMD_READ_VERSION,      READ_VERSION,       cmd_read_version}, // not use, has moved to DEVICE_INF_SERVICE
    {CMD_SET_CLOCK_POINTER, SET_CLOCK_POINTER,  cmd_set_clock_hand},
    {CMD_SET_VIBRATION,     SET_VIBRATION,      cmd_set_vibration},
    {CMD_SET_FIND_WATCH,    SET_FIND_WATCH,     cmd_find_watch},
    {CMD_SET_ANCS_BOND_REQ, SET_ANCS_BOND_REQ,  cmd_set_ancs_bond_req},
    {CMD_READ_TIME_STEPS,   READ_TIME_STEPS,    cmd_read_time_steps},

	{CMD_APP_NONE,          REPORT_MAX, NULL}
};

static s16 cmd_pairing_code(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.pair_code, buffer, sizeof(cmd_pairing_code_t));
    return 0;
}
static s16 cmd_user_info(u8 *buffer, u8 length)
{
    cmd_user_info_t* user_info = (cmd_user_info_t*)buffer;

    if(user_info->gender > 1) return 1;
    
    MemCopy(&cmd_group.user_info, buffer, sizeof(cmd_user_info_t));
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

    if(alarm_clock->clock1_alarm_switch > 1) return 1;
    //if(alarm_clock->clock1_week.week > 7) return 1;
    if(alarm_clock->clock1_hour > 23) return 1;
    if(alarm_clock->clock1_minute > 59) return 1;
    if(alarm_clock->clock2_alarm_switch > 1) return 1;
    //if(alarm_clock->clock2_week.week > 7) return 1;
    if(alarm_clock->clock2_hour > 23) return 1;
    if(alarm_clock->clock2_minute > 59) return 1;
    if(alarm_clock->clock3_alarm_switch > 1) return 1;
    //if(alarm_clock->clock3_week.week > 7) return 1;
    if(alarm_clock->clock3_hour > 23) return 1;
    if(alarm_clock->clock3_minute > 59) return 1;
    if(alarm_clock->clock4_alarm_switch > 1) return 1;
    //if(alarm_clock->clock4_week.week > 7) return 1;
    if(alarm_clock->clock4_hour > 23) return 1;
    if(alarm_clock->clock4_minute > 59) return 1;
    
    MemCopy(&cmd_group.set_alarm_clock, buffer, sizeof(cmd_set_alarm_clock_t));
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
    return 0;
}

static s16 cmd_response(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.send_resp, buffer, sizeof(cmd_response_t));
    return 0;
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

static s16 cmd_read_time_steps(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.read_time_step, buffer, sizeof(cmd_read_time_steps_t)); 
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
        case CMD_RESPONSE_TO_WATCH: // 0x06
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
            tmp_buf--; // no need to send "result" msg
            BufWriteUint8((uint8 **)&tmp_buf, 0x00); // dummy response: 0x00, ended to send...
            break;
        case CMD_READ_TIME_STEPS:   // 0x0E
            BufWriteUint8((uint8 **)&tmp_buf,data[0]);
            if(data[0] == 0x00) { // data & time
                BufWriteUint16((uint8 **)&tmp_buf, cmd_time->year);
                BufWriteUint8((uint8 **)&tmp_buf, cmd_time->month);
                BufWriteUint8((uint8 **)&tmp_buf, cmd_time->day);
                BufWriteUint8((uint8 **)&tmp_buf, cmd_time->hour);
                BufWriteUint8((uint8 **)&tmp_buf, cmd_time->minute);
                BufWriteUint8((uint8 **)&tmp_buf, cmd_time->second);
                BufWriteUint8((uint8 **)&tmp_buf, cmd_time->week);
            } else if(data[0] == 0x01) { // target total steps
                *tmp_buf++ = cmd_group.user_info.target_steps[3];
                *tmp_buf++ = cmd_group.user_info.target_steps[2];
                *tmp_buf++ = cmd_group.user_info.target_steps[1];
                *tmp_buf++ = cmd_group.user_info.target_steps[0];
            } else if(data[0] == 0xFF) { // ANCS connect state
				BufWriteUint8((uint8 **)&tmp_buf, (uint8)(g_app_data.remote_gatt_handles_present));
            }
            break;

        default:
            return 0;
    }
    length = tmp_buf - rsp_buf;
    BLE_SEND_DATA(rsp_buf, length);
    if((cmd_type == CMD_PAIRING_CODE) && (data[0] == 0xFF) && (data[1] == 0xFF)) {
        rsp_buf[2] = 0xFF;
        BLE_SEND_DATA(rsp_buf, length);
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

    if((cmd_list[i].cmd != CMD_APP_NONE) && (res == 0)) {
        res = cmd_cb(cmd_list[i].report, NULL);
        cmd_resp(cmd_list[i].cmd, res, &content[1]);
    }
    
    //get_driver()->uart->uart_write((unsigned char*)content, length);
}

cmd_group_t *cmd_get(void)
{
    return &cmd_group;
}

s16 cmd_refresh_time(clock_t *ck)
{
    cmd_time = ck;
	return 0;
}

s16 cmd_init(adapter_callback cb)
{
	cmd_cb = cb;
	return 0;
}
