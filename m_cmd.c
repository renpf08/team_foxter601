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
#include "m_cmd.h"
#include "driver/driver.h"

int cmd_log(const char* file, const char* func, unsigned line, const char* level, const char * sFormat, ...);

#define CMD_LOG_ERROR(...)    cmd_log(__FILE__, __func__, __LINE__, "<error>", __VA_ARGS__)
#define CMD_LOG_WARNING(...)  cmd_log(__FILE__, __func__, __LINE__, "<warning>", __VA_ARGS__)
#define CMD_LOG_INFO(...)     cmd_log(__FILE__, __func__, __LINE__, "<info>", __VA_ARGS__)
#define CMD_LOG_DEBUG(...)    cmd_log(__FILE__, __func__, __LINE__, "<debug>", __VA_ARGS__)

typedef enum
{
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

typedef enum
{
    CMD_RESPONS_TO_APP      = 0x00,
    CMD_SEND_MEASURE_DATA   = 0x01,
    CMD_FIND_PHONE          = 0x02,
    
    CMD_WATCH_NONE          = 0xFF
} cmd_watch_send_t;

typedef uint8 (* LFPCMDHANDLER)(char *srcStr, uint8 length);

typedef struct cmdEntry_T
{
	const cmd_app_send_t cmd; //! ÃüÁî×Ö
	LFPCMDHANDLER handler;
}CMDENTRY, *LPCMDENTRY;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// FUNCTION PROTOTYPES
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static uint8 cmd_pairing_code(char *srcStr, uint8 length);
static uint8 cmd_user_info(char *srcStr, uint8 length);
static uint8 cmd_sync_date(char *srcStr, uint8 length);
static uint8 cmd_set_alarm_clock(char *srcStr, uint8 length);
static uint8 cmd_set_disp_format(char *srcStr, uint8 length);
static uint8 cmd_read_data(char *srcStr, uint8 length);
static uint8 cmd_response(char *srcStr, uint8 length);
static uint8 cmd_send_notify(char *srcStr, uint8 length);
static uint8 cmd_set_time(char *srcStr, uint8 length);
static uint8 cmd_read_version(char *srcStr, uint8 length);
static uint8 cmd_set_clock_pointer(char *srcStr, uint8 length);
static uint8 cmd_set_vibration(char *srcStr, uint8 length);
static uint8 cmd_find_watch(char *srcStr, uint8 length);
static uint8 cmd_set_ancs_bond_req(char *srcStr, uint8 length);
static uint8 cmd_read_time_steps(char *srcStr, uint8 length);

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

int cmd_log(const char* file, const char* func, unsigned line, const char* level, const char * sFormat, ...)
{
    return 0;
}

static uint8 cmd_pairing_code(char *srcStr, uint8 length)
{
    typedef struct 
    {
        uint8 cmd; 
        uint8 code[2];
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    CMD_LOG_DEBUG("cmd_pairing_code, cmd=%02X\r\n", cmdDetail->cmd);
    return 0;
}
static uint8 cmd_user_info(char *srcStr, uint8 length)
{
    typedef struct 
    {
        uint8 cmd; 
        uint8 targetStep[4];
        uint8 targetDist[4];
        uint8 targetCalorie[4];
        uint8 targetFloor[2];
        uint8 targetStreExer[2];
        uint8 sex;
        uint8 height;
        uint8 weight;
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    CMD_LOG_DEBUG("cmd_user_info, cmd=%02X\r\n", cmdDetail->cmd);
    return 0;
}
static uint8 cmd_sync_date(char *srcStr, uint8 length)
{
    typedef struct 
    {
        uint8 cmd; 
        uint8 year[2];
        uint8 month;
        uint8 day;
        uint8 hour;
        uint8 minute;
        uint8 second;
        uint8 week;
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    CMD_LOG_DEBUG("cmd_sync_date, cmd=%02X\r\n", cmdDetail->cmd);
    return 0;
}
static uint8 cmd_set_alarm_clock(char *srcStr, uint8 length)
{
    typedef struct 
    {
        uint8 cmd;
        uint8 clkAlarm1Switch;
        uint8 week1;
        uint8 hour1;
        uint8 minute1;
        uint8 clk2AlarmSwitch;
        uint8 weed2;
        uint8 hour2;
        uint8 minute2;
        uint8 clk3AlarmSwitch;
        uint8 weed3;
        uint8 hour3;
        uint8 minute3;
        uint8 clk4AlarmSwitch;
        uint8 weed4;
        uint8 hour4;
        uint8 minute4;
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    CMD_LOG_DEBUG("cmd_set_alarm_clock, cmd=%02X\r\n", cmdDetail->cmd);
    return 0;
}
static uint8 cmd_set_disp_format(char *srcStr, uint8 length)
{
    typedef struct 
    { 
        uint8 cmd; 
        uint8 custmDisp;
        uint8 clockFormat;
        uint8 mainTarget;
        uint8 usedHand;
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    CMD_LOG_DEBUG("cmd_set_disp_format, cmd=%02X\r\n", cmdDetail->cmd);
    return 0;
}
static uint8 cmd_read_data(char *srcStr, uint8 length)
{
    typedef struct 
    { 
        uint8 cmd; 
        uint8 ctrlValue; 
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    CMD_LOG_DEBUG("cmd_read_data, cmd=%02X\r\n", cmdDetail->cmd);
    return 0;
}
static uint8 cmd_response(char *srcStr, uint8 length)
{
    typedef struct 
    { 
        uint8 cmd; 
        uint8 watchCmd;
        uint8 respValue;
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    CMD_LOG_DEBUG("cmd_response, cmd=%02X\r\n", cmdDetail->cmd);
    return 0;
}

static uint8 cmd_send_notify(char *srcStr, uint8 length)
{
    typedef struct 
    { 
        uint8 cmd; 
        uint8 notifSta;
        uint8 impLevel;
        uint8 msgType;
        uint8 msgCnt;
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    CMD_LOG_DEBUG("cmd_send_notify, cmd=%02X\r\n", cmdDetail->cmd);
    return 0;
}
static uint8 cmd_set_time(char *srcStr, uint8 length)
{
    typedef struct 
    { 
        uint8 cmd; 
        uint8 clkPtr1;
        uint8 clkPtr1Pos;
        uint8 clkPtr2;
        uint8 clkPtr2Pos;
        uint8 clkPtr3;
        uint8 clkPtr3Pos;
        uint8 clkPtr4;
        uint8 clkPtr4Pos;
        uint8 clkPtr5;
        uint8 clkPtr5Pos;
        uint8 clkPtr6;
        uint8 clkPtr6Pos;
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    CMD_LOG_DEBUG("cmd_set_time, cmd=%02X\r\n", cmdDetail->cmd);
    return 0;
}
static uint8 cmd_read_version(char *srcStr, uint8 length)
{
    typedef struct 
    { 
        uint8 cmd; 
        uint8 serialNum;
        uint8 fwVersion;
        uint8 systemId;
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    CMD_LOG_DEBUG("cmd_read_version, cmd=%02X\r\n", cmdDetail->cmd);
    return 0;
}
static uint8 cmd_set_clock_pointer(char *srcStr, uint8 length)
{
    typedef struct 
    { 
        uint8 cmd; 
        uint8 clkPtrNum;
        uint8 clkPtrPos;
        uint8 cklPtrRotation;
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    CMD_LOG_DEBUG("cmd_set_clock_pointer, cmd=%02X\r\n", cmdDetail->cmd);
    return 0;
}
static uint8 cmd_set_vibration(char *srcStr, uint8 length)
{
    typedef struct 
    { 
        uint8 cmd; 
        uint8 vibMode;
        uint8 vibTimes;
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    CMD_LOG_DEBUG("cmd_set_vibration, cmd=%02X\r\n", cmdDetail->cmd);
    return 0;
}
static uint8 cmd_find_watch(char *srcStr, uint8 length)
{
    typedef struct 
    { 
        uint8 cmd; 
        uint8 alarmType; 
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    CMD_LOG_DEBUG("cmd_find_watch, cmd=%02X\r\n", cmdDetail->cmd);
    return 0;
}
static uint8 cmd_set_ancs_bond_req(char *srcStr, uint8 length)
{
    typedef struct 
    { 
        uint8 cmd; 
        uint8 action; 
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    /** initiate ANCS service discovering, for test purpose */
    if((length == 2) && (cmdDetail->action == 0xAA))
    {
        CMD_LOG_DEBUG("initiate ANCS service discovering...\r\n");
        get_driver()->uart->uart_write((unsigned char*)&"\x55\xAA\x55\xAA\x55\xAA\x55", 6); // test
        DiscoverServices();
    }
            
    return 0;
}
static uint8 cmd_read_time_steps(char *srcStr, uint8 length)
{
    typedef struct 
    { 
        uint8 cmd; 
        uint8 type; 
    } cmd_detail_t;
    cmd_detail_t* cmdDetail = (cmd_detail_t*)srcStr;
    
    CMD_LOG_DEBUG("cmd_read_time_steps, cmd=%02X\r\n", cmdDetail->cmd);
    return 0;
}

void cmd_dispatch(char* content, uint8 length)
{
	uint8 i = 0;

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

