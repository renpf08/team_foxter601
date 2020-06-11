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

typedef u8 (* LFPCMDHANDLER)(u8 *buffer, u8 length);

typedef struct cmdEntry_T {
	const cmd_app_send_t cmd;
	REPORT_E report;
	LFPCMDHANDLER handler;
}CMDENTRY, *LPCMDENTRY;

cmd_group_t cmd_group;
static adapter_callback cmd_cb = NULL;

static u8 cmd_pairing_code(u8 *buffer, u8 length);
static u8 cmd_user_info(u8 *buffer, u8 length);
static u8 cmd_set_time(u8 *buffer, u8 length);
static u8 cmd_set_alarm_clock(u8 *buffer, u8 length);
static u8 cmd_notify_switch(u8 *buffer, u8 length);
static u8 cmd_sync_data(u8 *buffer, u8 length);
static u8 cmd_response(u8 *buffer, u8 length);
static u8 cmd_recv_notify(u8 *buffer, u8 length);
static u8 cmd_set_pointers(u8 *buffer, u8 length);
static u8 cmd_read_version(u8 *buffer, u8 length);
static u8 cmd_set_clock_hand(u8 *buffer, u8 length);
static u8 cmd_set_vibration(u8 *buffer, u8 length);
static u8 cmd_find_watch(u8 *buffer, u8 length);
static u8 cmd_set_ancs_bond_req(u8 *buffer, u8 length);
static u8 cmd_read_time_steps(u8 *buffer, u8 length);
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
    {CMD_READ_VERSION,      READ_VERSION,       cmd_read_version},
    {CMD_SET_CLOCK_POINTER, SET_CLOCK_POINTER,  cmd_set_clock_hand},
    {CMD_SET_VIBRATION,     SET_VIBRATION,      cmd_set_vibration},
    {CMD_SET_FIND_WATCH,    SET_FIND_WATCH,     cmd_find_watch},
    {CMD_SET_ANCS_BOND_REQ, SET_ANCS_BOND_REQ,  cmd_set_ancs_bond_req},
    {CMD_READ_TIME_STEPS,   READ_TIME_STEPS,    cmd_read_time_steps},

	{CMD_APP_NONE,          REPORT_MAX, NULL}
};

static u8 cmd_pairing_code(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.pair_code, buffer, sizeof(cmd_pairing_code_t));
    return 0;
}
static u8 cmd_user_info(u8 *buffer, u8 length)
{
    cmd_user_info_t* user_info = (cmd_user_info_t*)buffer;

    if(user_info->sex > 1) return 1;
    
    MemCopy(&cmd_group.user_info, buffer, sizeof(cmd_user_info_t));
    return 0;
}
static u8 cmd_set_time(u8 *buffer, u8 length)
{
    u8 days[13] = {0x00,0x31,0x28,0x31,0x30,0x31,0x30,0x31,0x31,0x30,0x31,0x30,0x31};
    u16 year = 0;
    volatile cmd_set_time_t* set_time = (cmd_set_time_t*)buffer;

    year = (u16)((set_time->year[0]<<8 & 0xFF00) | set_time->year[1]);
    days[2] = (0 == (year % 400) || (0 == (year % 4) && (0 != (year % 100))))?29:28;

    if(set_time->month > 0x12) return 1;
    if(set_time->day > days[set_time->month]) return 2;
    if(set_time->hour > 0x23) return 3;
    if(set_time->minute > 0x59) return 4;
    if(set_time->second > 0x59) return 5;
    if(set_time->week > 0x07) return 6;
    
    MemCopy(&cmd_group.set_time, buffer, sizeof(cmd_set_time_t));
    return 0;
}

static u8 cmd_set_alarm_clock(u8 *buffer, u8 length)
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

static u8 cmd_notify_switch(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.notify_switch, buffer, sizeof(cmd_notify_switch_t));
    return 0;
}

static u8 cmd_sync_data(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.sync_data, buffer, sizeof(cmd_sync_data_t));
    return 0;
}

static u8 cmd_response(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.send_resp, buffer, sizeof(cmd_response_t));
    return 0;
}

static u8 cmd_recv_notify(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.recv_notif, buffer, sizeof(cmd_recv_notify_t));
    return 0;
}

static u8 cmd_set_pointers(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.set_pointers, buffer, sizeof(cmd_set_pointers_t));
    return 0;
}

static u8 cmd_read_version(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.read_ver, buffer, sizeof(cmd_read_version_t));
    return 0;
}

static u8 cmd_set_clock_hand(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.set_clock_hand, buffer, sizeof(cmd_set_clock_hand_t));
    return 0;
}

static u8 cmd_set_vibration(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.set_vib, buffer, sizeof(cmd_set_vibration_t));
    return 0;
}

static u8 cmd_find_watch(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.find_watch, buffer, sizeof(cmd_find_watch_t));
    return 0;
}

static u8 cmd_set_ancs_bond_req(u8 *buffer, u8 length)
{
    if(buffer[1] == 0xAA) {
        DiscoverServices();
    }
    //MemCopy(&cmd_group.set_ancs_bond, buffer, sizeof(cmd_set_ancs_bond_req_t));  
    return 0;
}

static u8 cmd_read_time_steps(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.read_time_step, buffer, sizeof(cmd_read_time_steps_t)); 
    return 0;
}

u8 cmd_resp(cmd_app_send_t cmd_type, u8 result, u8 *data)
{
    u16 length = 0; 
    u8 rsp_buf[20];
    u8 clear_reg = 0;
    u8 *tmp_buf = rsp_buf;
    BD_ADDR_T addr;

    CSReadBdaddr(&addr);
    BufWriteUint8((uint8 **)&tmp_buf,0x00);
    BufWriteUint8((uint8 **)&tmp_buf,cmd_type);
    BufWriteUint8((uint8 **)&tmp_buf,result);
    if(cmd_type == CMD_SYNC_DATA) return 0;
    if(cmd_type == CMD_PAIRING_CODE) {
        if((data[0] == 0xFF) && (data[1] == 0xFF)) clear_reg = 1;
        BufWriteUint16((uint8 **)&tmp_buf, addr.nap);
        BufWriteUint8((uint8 **)&tmp_buf, addr.uap);

        BufWriteUint16((uint8 **)&tmp_buf,( addr.lap));
        BufWriteUint8((uint8 **)&tmp_buf,( addr.lap>>16));
    }
    length = tmp_buf - rsp_buf;
    cmd_send_data(rsp_buf, length);
    if(clear_reg == 1) {
        rsp_buf[2] = 0xFF;
        cmd_send_data(rsp_buf, length);
    }

    return length;
}

void cmd_parse(u8* content, u8 length)
{
	u8 i = 0;
    u8 res = 0;

	if(length == 0) {
		return;
	}

    while(cmd_list[i].cmd != CMD_APP_NONE) {
        if(cmd_list[i].cmd == content[0]) {
            res = cmd_list[i].handler(content, length);
            cmd_resp(cmd_list[i].cmd, res, content);
            break;
        }
        i++;
    }

    if((cmd_list[i].cmd != CMD_APP_NONE) && (res == 0)) {
        cmd_cb(cmd_list[i].report, NULL);
    }
    
    //get_driver()->uart->uart_write((unsigned char*)content, length);
}

void cmd_send_data(uint8 *data, uint16 size)
{
    size = (size >20)?20:size;
    SerialSendNotification(data, size);
}

cmd_group_t *cmd_get(void)
{
    return &cmd_group;
}

s16 cmd_init(adapter_callback cb)
{
	cmd_cb = cb;
	return 0;
}
