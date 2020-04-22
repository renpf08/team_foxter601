/**
*  @file    m_cmd.c
*  @author  maliwen
*  @date    2020/03/31
*  @history 1.0.0.0(°æ±¾ºÅ)
*           1.0.0.0: Creat              2020/03/31
*/

#include <mem.h>
#include <gatt.h>
#include "user_config.h"
#include "ancs_client.h"
#include "driver/driver.h"
#include "adapter/adapter.h"

int cmd_log(const s8* file, const s8* func, unsigned line, const s8* level, const s8 * sFormat, ...);

#define CMD_LOG_ERROR(...)    cmd_log(__FILE__, __func__, __LINE__, "<error>", __VA_ARGS__)
#define CMD_LOG_WARNING(...)  cmd_log(__FILE__, __func__, __LINE__, "<warning>", __VA_ARGS__)
#define CMD_LOG_INFO(...)     cmd_log(__FILE__, __func__, __LINE__, "<info>", __VA_ARGS__)
#define CMD_LOG_DEBUG(...)    cmd_log(__FILE__, __func__, __LINE__, "<debug>", __VA_ARGS__)

typedef u8 (* LFPCMDHANDLER)(u8 *buffer, u8 length);

typedef enum {
    CMD_PAIRING_CODE        = 0x00,
    CMD_USER_INFO           = 0x01,
    CMD_SYNC_DATE           = 0x02,
    CMD_SET_ALARM_CLOCK     = 0x03,
    CMD_SET_DISP_FORMAT     = 0x04,
    CMD_READ_DATA           = 0x05,
    CMD_RESPONSE_TO_WATCH   = 0x06,
    CMD_SEND_NOTIFY         = 0x07,
    CMD_SET_TIME            = 0x08,
    CMD_READ_VERSION        = 0x09, //! need to response
    CMD_SET_CLOCK_POINTER   = 0x0A,
    CMD_SET_VIBRATION       = 0x0B,
    CMD_SET_FIND_WATCH      = 0x0C,
    CMD_SET_ANCS_BOND_REQ   = 0x0D,
    CMD_READ_TIME_STEPS     = 0x0E,
    
    CMD_APP_NONE            = 0xFF
} cmd_app_send_t;

typedef enum {
    CMD_RESPONS_TO_APP      = 0x00,
    CMD_SEND_MEASURE_DATA   = 0x01,
    CMD_FIND_PHONE          = 0x02,
    
    CMD_WATCH_NONE          = 0xFF
} cmd_watch_send_t;

typedef struct {
    u8 cmd; 
    u8 code[2];
} cmd_pairing_code_t;
typedef struct 
{
    u8 cmd; 
    u8 target_step[4];
    u8 target_dist[4];
    u8 target_calorie[4];
    u8 target_floor[2];
    u8 target_stre_exer[2];
    u8 sex;
    u8 height;
    u8 weight;
} cmd_user_info_t;
typedef struct {
    u8 cmd; 
    u8 year[2];
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
    u8 week;
} cmd_sync_date_t;
typedef struct {
    u8 cmd;
    u8 clock1_alarm_switch;
    u8 week1;
    u8 hour1;
    u8 minute1;
    u8 clock2_alarm_switch;
    u8 weed2;
    u8 hour2;
    u8 minute2;
    u8 clock3_alarm_switch;
    u8 weed3;
    u8 hour3;
    u8 minute3;
    u8 clock4_alarm_switch;
    u8 weed4;
    u8 hour4;
    u8 minute4;
} cmd_set_alarm_clock_t;
typedef struct { 
    u8 cmd; 
    u8 custm_disp;
    u8 clock_format;
    u8 main_target;
    u8 used_hand;
} cmd_set_disp_format_t;
typedef struct { 
    u8 cmd; 
    u8 ctrl_value; 
} cmd_read_data_t;
typedef struct { 
    u8 cmd; 
    u8 watch_cmd;
    u8 resp_value;
} cmd_response_t;
typedef struct { 
    u8 cmd; 
    u8 notif_sta;
    u8 imp_level;
    u8 msg_type;
    u8 msg_cnt;
} cmd_send_notify_t;
typedef struct { 
    u8 cmd; 
    u8 clock_hand1;
    u8 clock_hand1_pos;
    u8 clock_hand2;
    u8 clock_hand2_pos;
    u8 clock_hand3;
    u8 clock_hand3_pos;
    u8 clock_hand4;
    u8 clock_hand4_pos;
    u8 clock_hand5;
    u8 clock_hand5_pos;
    u8 clock_hand6;
    u8 clock_hand6_pos;
} cmd_set_time_t;
typedef struct { 
    u8 cmd; 
    u8 serial_num;
    u8 fw_version;
    u8 system_id;
} cmd_read_version_t;
typedef struct { 
    u8 cmd; 
    u8 clock_hand_num;
    u8 clock_hand_pos;
    u8 clock_hand_rotation;
} cmd_set_clock_hand_t;
typedef struct { 
    u8 cmd; 
    u8 vib_mode;
    u8 vib_times;
} cmd_set_vibration_t;
typedef struct { 
    u8 cmd; 
    u8 alarm_type; 
} cmd_find_watch_t;
typedef struct { 
    u8 cmd; 
    u8 action; 
} cmd_set_ancs_bond_req_t;
typedef struct { 
    u8 cmd; 
    u8 type; 
} cmd_read_time_steps_t;
typedef struct {
    cmd_pairing_code_t pair_code;
    cmd_user_info_t user_info;
    cmd_sync_date_t sync_date;
    cmd_set_alarm_clock_t set_alarm_clock;
    cmd_set_disp_format_t set_disp;
    cmd_read_data_t read_data;
    cmd_response_t send_resp;
    cmd_send_notify_t send_notif;
    cmd_set_time_t set_time;
    cmd_read_version_t read_ver;
    cmd_set_clock_hand_t set_clock_hand;
    cmd_set_vibration_t set_vib;
    cmd_find_watch_t find_watch;
    cmd_set_ancs_bond_req_t set_ancs_bond;
    cmd_read_time_steps_t read_time_step;
} cmd_group_t;
typedef struct cmdEntry_T {
	const cmd_app_send_t cmd; //! ÃüÁî×Ö
	LFPCMDHANDLER handler;
}CMDENTRY, *LPCMDENTRY;

typedef struct {
	const cmd_app_send_t cmd; //! ÃüÁî×Ö
	u8* buf;
    u8 size;
}cmd_set_t;

cmd_group_t cmd_group;

cmd_set_t cmd_list[] = {
    {CMD_PAIRING_CODE,      (u8*)&cmd_group.pair_code,      sizeof(cmd_pairing_code_t)},
    {CMD_USER_INFO,         (u8*)&cmd_group.user_info,      sizeof(cmd_user_info_t)},
    {CMD_SYNC_DATE,         (u8*)&cmd_group.sync_date,      sizeof(cmd_sync_date_t)},
    {CMD_SET_ALARM_CLOCK,   (u8*)&cmd_group.set_alarm_clock,sizeof(cmd_set_alarm_clock_t)},
    {CMD_SET_DISP_FORMAT,   (u8*)&cmd_group.set_disp,       sizeof(cmd_set_disp_format_t)},
    {CMD_READ_DATA,         (u8*)&cmd_group.read_data,      sizeof(cmd_read_data_t)},
    {CMD_RESPONSE_TO_WATCH, (u8*)&cmd_group.send_resp,      sizeof(cmd_response_t)},
    {CMD_SEND_NOTIFY,       (u8*)&cmd_group.send_notif,     sizeof(cmd_send_notify_t)},
    {CMD_SET_TIME,          (u8*)&cmd_group.set_time,       sizeof(cmd_set_time_t)},
    {CMD_READ_VERSION,      (u8*)&cmd_group.read_ver,       sizeof(cmd_read_version_t)},
    {CMD_SET_CLOCK_POINTER, (u8*)&cmd_group.set_clock_hand, sizeof(cmd_set_clock_hand_t)},
    {CMD_SET_VIBRATION,     (u8*)&cmd_group.set_vib,        sizeof(cmd_set_vibration_t)},
    {CMD_SET_FIND_WATCH,    (u8*)&cmd_group.find_watch,     sizeof(cmd_find_watch_t)},
    {CMD_SET_ANCS_BOND_REQ, (u8*)&cmd_group.set_ancs_bond,  sizeof(cmd_set_ancs_bond_req_t)},
    {CMD_READ_TIME_STEPS,   (u8*)&cmd_group.read_time_step, sizeof(cmd_read_time_steps_t)},

	{CMD_APP_NONE, NULL, 0}
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// FUNCTION PROTOTYPES
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static u8 cmd_pairing_code(u8 *buffer, u8 length);
static u8 cmd_user_info(u8 *buffer, u8 length);
static u8 cmd_sync_date(u8 *buffer, u8 length);
static u8 cmd_set_alarm_clock(u8 *buffer, u8 length);
static u8 cmd_set_disp_format(u8 *buffer, u8 length);
static u8 cmd_read_data(u8 *buffer, u8 length);
static u8 cmd_response(u8 *buffer, u8 length);
static u8 cmd_send_notify(u8 *buffer, u8 length);
static u8 cmd_set_time(u8 *buffer, u8 length);
static u8 cmd_read_version(u8 *buffer, u8 length);
static u8 cmd_set_clock_hand(u8 *buffer, u8 length);
static u8 cmd_set_vibration(u8 *buffer, u8 length);
static u8 cmd_find_watch(u8 *buffer, u8 length);
static u8 cmd_set_ancs_bond_req(u8 *buffer, u8 length);
static u8 cmd_read_time_steps(u8 *buffer, u8 length);

//-----------------------------------------------------------------------------
// Name: cmdList
// Desc: app command list.
//-----------------------------------------------------------------------------
static const CMDENTRY cmdList[] =
{
    {CMD_PAIRING_CODE, cmd_pairing_code},
    {CMD_USER_INFO, cmd_user_info},
    {CMD_SYNC_DATE, cmd_sync_date},
    {CMD_SET_ALARM_CLOCK, cmd_set_alarm_clock},
    {CMD_SET_DISP_FORMAT, cmd_set_disp_format},
    {CMD_READ_DATA, cmd_read_data},
    {CMD_RESPONSE_TO_WATCH, cmd_response},
    {CMD_SEND_NOTIFY, cmd_send_notify},
    {CMD_SET_TIME, cmd_set_time},
    {CMD_READ_VERSION, cmd_read_version},
    {CMD_SET_CLOCK_POINTER, cmd_set_clock_hand},
    {CMD_SET_VIBRATION, cmd_set_vibration},
    {CMD_SET_FIND_WATCH, cmd_find_watch},
    {CMD_SET_ANCS_BOND_REQ, cmd_set_ancs_bond_req},
    {CMD_READ_TIME_STEPS, cmd_read_time_steps},

	{CMD_APP_NONE, NULL}		//! Finish here
};

int cmd_log(const s8* file, const s8* func, unsigned line, const s8* level, const s8 * sFormat, ...)
{
    return 0;
}

static u8 cmd_pairing_code(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.pair_code, buffer, sizeof(cmd_pairing_code_t));
    return 0;
}
static u8 cmd_user_info(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.user_info, buffer, sizeof(cmd_user_info_t));
    return 0;
}
static u8 cmd_sync_date(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.sync_date, buffer, sizeof(cmd_sync_date_t));
    return 0;
}
static u8 cmd_set_alarm_clock(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.set_alarm_clock, buffer, sizeof(cmd_set_alarm_clock_t));
    return 0;
}
static u8 cmd_set_disp_format(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.set_disp, buffer, sizeof(cmd_set_disp_format_t));
    return 0;
}
static u8 cmd_read_data(u8 *buffer, u8 length)
{
    MemCopy(&cmd_group.read_data, buffer, sizeof(cmd_read_data_t));
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

void cmd_dispatch(u8* content, u8 length)
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
    
    get_driver()->uart->uart_write((unsigned char*)content, length);
}

void cmd_parse(u8* content, u8 length)
{
	u8 i = 0;

	if(length == 0)
	{
		return;
	}

    while(cmd_list[i].cmd != CMD_APP_NONE)
    {
        if(cmd_list[i].cmd == content[0])
        {
            MemCopy(cmd_list[i].buf, content, cmd_list[i].size);
            break;
        }
        i++;
    }
    
    get_driver()->uart->uart_write((unsigned char*)content, length);
}

