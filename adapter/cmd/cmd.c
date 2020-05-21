/**
*  @file    m_cmd.c
*  @author  maliwen
*  @date    2020/03/31
*  @history 1.0.0.0(°æ±¾ºÅ)
*           1.0.0.0: Creat              2020/03/31
*/

#include <mem.h>
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
static u8 cmd_sync_date(u8 *buffer, u8 length);
static u8 cmd_set_alarm_clock(u8 *buffer, u8 length);
static u8 cmd_set_disp_format(u8 *buffer, u8 length);
static u8 cmd_sync_data(u8 *buffer, u8 length);
static u8 cmd_response(u8 *buffer, u8 length);
static u8 cmd_send_notify(u8 *buffer, u8 length);
static u8 cmd_set_time(u8 *buffer, u8 length);
static u8 cmd_read_version(u8 *buffer, u8 length);
static u8 cmd_set_clock_hand(u8 *buffer, u8 length);
static u8 cmd_set_vibration(u8 *buffer, u8 length);
static u8 cmd_find_watch(u8 *buffer, u8 length);
static u8 cmd_set_ancs_bond_req(u8 *buffer, u8 length);
static u8 cmd_read_time_steps(u8 *buffer, u8 length);
void cmd_parse(u8* content, u8 length);
s16 cmd_init(adapter_callback cb);

static const CMDENTRY cmdList[] =
{
    {CMD_PAIRING_CODE,      CMD_PAIRING_CODE_INCOMING,      cmd_pairing_code},
    {CMD_USER_INFO,         CMD_USER_INFO_INCOMING,         cmd_user_info},
    {CMD_SYNC_DATE,         CMD_SYNC_DATE_INCOMING,         cmd_sync_date},
    {CMD_SET_ALARM_CLOCK,   CMD_SET_ALARM_CLOCK_INCOMING,   cmd_set_alarm_clock},
    {CMD_SET_DISP_FORMAT,   CMD_SET_DISP_FORMAT_INCOMING,   cmd_set_disp_format},
    {CMD_SYNC_DATA,         CMD_SYNC_DATA_INCOMING,         cmd_sync_data},
    {CMD_RESPONSE_TO_WATCH, CMD_RESPONSE_TO_WATCH_INCOMING, cmd_response},
    {CMD_SEND_NOTIFY,       CMD_SEND_NOTIFY_INCOMING,       cmd_send_notify},
    {CMD_SET_TIME,          CMD_SET_TIME_INCOMING,          cmd_set_time},
    {CMD_READ_VERSION,      CMD_READ_VERSION_INCOMING,      cmd_read_version},
    {CMD_SET_CLOCK_POINTER, CMD_SET_CLOCK_POINTER_INCOMING, cmd_set_clock_hand},
    {CMD_SET_VIBRATION,     CMD_SET_VIBRATION_INCOMING,     cmd_set_vibration},
    {CMD_SET_FIND_WATCH,    CMD_SET_FIND_WATCH_INCOMING,    cmd_find_watch},
    {CMD_SET_ANCS_BOND_REQ, CMD_SET_ANCS_BOND_REQ_INCOMING, cmd_set_ancs_bond_req},
    {CMD_READ_TIME_STEPS,   CMD_READ_TIME_STEPS_INCOMING,   cmd_read_time_steps},

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
static u8 cmd_sync_date(u8 *buffer, u8 length)
{
    u8 days[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    u16 year = 0;
    cmd_sync_date_t* sync_date = (cmd_sync_date_t*)buffer;

    year = (u16)((sync_date->year[0]<<8 & 0xFF00) | sync_date->year[1]);
    days[2] = (0 == (year % 400) || (0 == (year % 4) && (0 != (year % 100))))?29:28;

    if(sync_date->month > 12) return 1;
    if(sync_date->day > days[sync_date->month]) return 1;
    if(sync_date->hour > 23) return 1;
    if(sync_date->minute > 59) return 1;
    if(sync_date->second > 59) return 1;
    if(sync_date->week > 7) return 1;
    
    MemCopy(&cmd_group.sync_date, buffer, sizeof(cmd_sync_date_t));
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
static u8 cmd_set_disp_format(u8 *buffer, u8 length)
{
    cmd_set_disp_format_t* disp_format = (cmd_set_disp_format_t*)buffer;

    if(disp_format->clock_format > 1) return 1;
    if(disp_format->main_target > 4) return 1;
    if(disp_format->used_hand > 1) return 1;
    
    MemCopy(&cmd_group.set_disp, buffer, sizeof(cmd_set_disp_format_t));
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
static u8 cmd_send_notify(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.send_notif, buffer, sizeof(cmd_send_notify_t));
    return 0;
}
static u8 cmd_set_time(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.set_time, buffer, sizeof(cmd_set_time_t));
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
    /** initiate ANCS service discovering, for test purpose */
    //if((length == 2) && (cmd_buffer->action == 0xAA)) DiscoverServices();
          
    MemCopy(&cmd_group.set_ancs_bond, buffer, sizeof(cmd_set_ancs_bond_req_t));  
    return 0;
}
static u8 cmd_read_time_steps(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.read_time_step, buffer, sizeof(cmd_read_time_steps_t)); 
    return 0;
}
u8 cmd_resp(cmd_app_send_t cmd_type, u8 result, u8 *buffer)
{
    u8 len = 0;
    u16 test_nap = 0x1234;
    u16 test_uap = 0x5678;
    u16 test_lap = 0x9ABC;

    buffer[len++] = 0x00;
    buffer[len++] = cmd_type;
    buffer[len++] = result;
    if(cmd_type == CMD_SYNC_DATA) return 0;
    if(cmd_type == CMD_PAIRING_CODE)
    {
        buffer[len++] = (test_nap>>8)&0x00FF;
        buffer[len++] = test_nap&0x00FF;
        buffer[len++] = (test_uap>>8)&0x00FF;
        buffer[len++] = test_uap&0x00FF;
        buffer[len++] = (test_lap>>8)&0x00FF;
        buffer[len++] = test_lap&0x00FF;
    }

    return len;
}

void cmd_parse(u8* content, u8 length)
{
	u8 i = 0;

	if(length == 0)
	{
		return;
	}

    while(cmdList[i].cmd != CMD_APP_NONE)
    {
        if(cmdList[i].cmd == content[0])
        {
            cmdList[i].handler(content, length);
            break;
        }
        i++;
    }

    if(cmdList[i].cmd != CMD_APP_NONE)
    {
        //cmd_cb_handler(...);
    }
    
    //get_driver()->uart->uart_write((unsigned char*)content, length);
}
void cmd_send_data(uint8 *data, uint16 size)
{
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

