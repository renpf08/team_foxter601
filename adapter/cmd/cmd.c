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
    CMD_READ_VERSION        = 0x09, //! watch need to response to app with msg
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
    u8 clkAlarm1Switch;
    u8 week1;
    u8 hour1;
    u8 minute1;
    u8 clk2AlarmSwitch;
    u8 weed2;
    u8 hour2;
    u8 minute2;
    u8 clk3AlarmSwitch;
    u8 weed3;
    u8 hour3;
    u8 minute3;
    u8 clk4AlarmSwitch;
    u8 weed4;
    u8 hour4;
    u8 minute4;
} cmd_set_alarm_clock_t;
typedef struct { 
    u8 cmd; 
    u8 custmDisp;
    u8 clockFormat;
    u8 mainTarget;
    u8 usedHand;
} cmd_set_disp_format_t;
typedef struct { 
    u8 cmd; 
    u8 ctrlValue; 
} cmd_read_data_t;
typedef struct { 
    u8 cmd; 
    u8 watchCmd;
    u8 respValue;
} cmd_response_t;
typedef struct { 
    u8 cmd; 
    u8 notifSta;
    u8 impLevel;
    u8 msgType;
    u8 msgCnt;
} cmd_send_notify_t;
typedef struct { 
    u8 cmd; 
    u8 clkPtr1;
    u8 clkPtr1Pos;
    u8 clkPtr2;
    u8 clkPtr2Pos;
    u8 clkPtr3;
    u8 clkPtr3Pos;
    u8 clkPtr4;
    u8 clkPtr4Pos;
    u8 clkPtr5;
    u8 clkPtr5Pos;
    u8 clkPtr6;
    u8 clkPtr6Pos;
} cmd_set_time_t;
typedef struct { 
    u8 cmd; 
    u8 serialNum;
    u8 fwVersion;
    u8 systemId;
} cmd_read_version_t;
typedef struct { 
    u8 cmd; 
    u8 clkPtrNum;
    u8 clkPtrPos;
    u8 cklPtrRotation;
} cmd_set_clock_hand_t;
typedef struct { 
    u8 cmd; 
    u8 vibMode;
    u8 vibTimes;
} cmd_set_vibration_t;
typedef struct { 
    u8 cmd; 
    u8 alarmType; 
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
static u8 cmd_set_clock_pointer(u8 *buffer, u8 length);
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
    {CMD_SET_CLOCK_POINTER, cmd_set_clock_pointer},
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
    cmd_pairing_code_t* cmd_buffer = (cmd_pairing_code_t*)buffer;
    
    CMD_LOG_DEBUG("cmd_pairing_code, cmd=%02X\r\n", cmd_buffer->cmd);
    return 0;
}
static u8 cmd_user_info(u8 *buffer, u8 length)
{
    typedef struct 
    {
        u8 cmd; 
        u8 targetStep[4];
        u8 targetDist[4];
        u8 targetCalorie[4];
        u8 targetFloor[2];
        u8 targetStreExer[2];
        u8 sex;
        u8 height;
        u8 weight;
    } cmd_user_info_t;
    cmd_user_info_t* cmd_buffer = (cmd_user_info_t*)buffer;
    
    CMD_LOG_DEBUG("cmd_user_info, cmd=%02X\r\n", cmd_buffer->cmd);
    return 0;
}
static u8 cmd_sync_date(u8 *buffer, u8 length)
{
    cmd_sync_date_t* cmd_buffer = (cmd_sync_date_t*)buffer;
    
    CMD_LOG_DEBUG("cmd_sync_date, cmd=%02X\r\n", cmd_buffer->cmd);
    return 0;
}
static u8 cmd_set_alarm_clock(u8 *buffer, u8 length)
{
    cmd_set_alarm_clock_t* cmd_buffer = (cmd_set_alarm_clock_t*)buffer;
    
    CMD_LOG_DEBUG("cmd_set_alarm_clock, cmd=%02X\r\n", cmd_buffer->cmd);
    return 0;
}
static u8 cmd_set_disp_format(u8 *buffer, u8 length)
{
    cmd_set_disp_format_t* cmd_buffer = (cmd_set_disp_format_t*)buffer;
    
    CMD_LOG_DEBUG("cmd_set_disp_format, cmd=%02X\r\n", cmd_buffer->cmd);
    return 0;
}
static u8 cmd_read_data(u8 *buffer, u8 length)
{
    cmd_read_data_t* cmd_buffer = (cmd_read_data_t*)buffer;
    
    CMD_LOG_DEBUG("cmd_read_data, cmd=%02X\r\n", cmd_buffer->cmd);
    return 0;
}
static u8 cmd_response(u8 *buffer, u8 length)
{
    cmd_response_t* cmd_buffer = (cmd_response_t*)buffer;
    
    CMD_LOG_DEBUG("cmd_response, cmd=%02X\r\n", cmd_buffer->cmd);
    return 0;
}

static u8 cmd_send_notify(u8 *buffer, u8 length)
{
    cmd_send_notify_t* cmd_buffer = (cmd_send_notify_t*)buffer;
    
    CMD_LOG_DEBUG("cmd_send_notify, cmd=%02X\r\n", cmd_buffer->cmd);
    return 0;
}
static u8 cmd_set_time(u8 *buffer, u8 length)
{
    cmd_set_time_t* cmd_buffer = (cmd_set_time_t*)buffer;
    
    CMD_LOG_DEBUG("cmd_set_time, cmd=%02X\r\n", cmd_buffer->cmd);
    return 0;
}
static u8 cmd_read_version(u8 *buffer, u8 length)
{
    cmd_read_version_t* cmd_buffer = (cmd_read_version_t*)buffer;
    
    CMD_LOG_DEBUG("cmd_read_version, cmd=%02X\r\n", cmd_buffer->cmd);
    return 0;
}
static u8 cmd_set_clock_pointer(u8 *buffer, u8 length)
{
    cmd_set_clock_hand_t* cmd_buffer = (cmd_set_clock_hand_t*)buffer;
    
    CMD_LOG_DEBUG("cmd_set_clock_pointer, cmd=%02X\r\n", cmd_buffer->cmd);
    return 0;
}
static u8 cmd_set_vibration(u8 *buffer, u8 length)
{
    cmd_set_vibration_t* cmd_buffer = (cmd_set_vibration_t*)buffer;
    
    CMD_LOG_DEBUG("cmd_set_vibration, cmd=%02X\r\n", cmd_buffer->cmd);
    return 0;
}
static u8 cmd_find_watch(u8 *buffer, u8 length)
{
    cmd_find_watch_t* cmd_buffer = (cmd_find_watch_t*)buffer;
    
    CMD_LOG_DEBUG("cmd_find_watch, cmd=%02X\r\n", cmd_buffer->cmd);
    return 0;
}
static u8 cmd_set_ancs_bond_req(u8 *buffer, u8 length)
{
    cmd_set_ancs_bond_req_t* cmd_buffer = (cmd_set_ancs_bond_req_t*)buffer;
    
    /** initiate ANCS service discovering, for test purpose */
    if((length == 2) && (cmd_buffer->action == 0xAA))
    {
        CMD_LOG_DEBUG("initiate ANCS service discovering...\r\n");
        get_driver()->uart->uart_write((unsigned char*)&"\x55\xAA\x55\xAA\x55\xAA\x55", 6); // test
        DiscoverServices();
    }
            
    return 0;
}
static u8 cmd_read_time_steps(u8 *buffer, u8 length)
{
    cmd_read_time_steps_t* cmd_buffer = (cmd_read_time_steps_t*)buffer;
    
    CMD_LOG_DEBUG("cmd_read_time_steps, cmd=%02X\r\n", cmd_buffer->cmd);
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

