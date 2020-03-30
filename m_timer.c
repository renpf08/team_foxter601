/**
*  @file    m_timer.c
*  @author  maliwen
*  @date    2020/02/20
*  @history 1.0.0.0(Version)
*           1.0.0.0: Creat              2020/02/20
*/

#include <debug.h>          /* Simple host interface to the UART driver */
#include <timer.h>          /* Chip timer functions */
#include <panic.h>          /* Support for applications to panic */
#include <mem.h>

#include "m_timer.h"
#include "m_printf.h"
#include "user_config.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Number of timers used in this application */
//#define MAX_TIMERS 8

/* system timer timeout value: 1000ms */
#define TIMER_CLOCK_TIMEOUT (1 * SECOND)

/* ancs timer timeout value: 1000ms */
#define TIMER_ANCS_TIMEOUT (1000 * MILLISECOND)

#if USE_M_LOG
#define TIMER_LOG_ERROR(...)        M_LOG_ERROR(__VA_ARGS__)
#define TIMER_LOG_WARNING(...)      //M_LOG_WARNING(__VA_ARGS__)
#define TIMER_LOG_INFO(...)         //M_LOG_INFO(__VA_ARGS__)
#define TIMER_LOG_DEBUG(...)        //M_LOG_DEBUG(__VA_ARGS__)
#else
#define TIMER_LOG_ERROR(...)
#define TIMER_LOG_WARNING(...)
#define TIMER_LOG_INFO(...)
#define TIMER_LOG_DEBUG(...)
#endif

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* Declare timer buffer to be managed by firmware library */
//static uint16 app_timers[SIZEOF_APP_TIMER * MAX_TIMERS];
static time_t sysLocalTime;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

static void m_timer_clock_handler(timer_id const id);
void m_timer_clock_init(void);
void m_set_time(time_t *tm);
static bool m_time_check(time_t *tm);
static bool m_time_sync(time_t *tm);
static int8 m_time_cmp(time_t* curTime, time_t* sysTime);
bool m_time_formating(uint8* timeStr, time_t* tm);

uint8 days[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

time_t* m_get_time(void) { return &sysLocalTime; }
void m_set_time(time_t *tm) { MemCopy(&sysLocalTime, tm, sizeof(time_t)); }

static void m_timer_clock_handler(timer_id const id)
{
    days[2] = (0 == (sysLocalTime.year % 400) || (0 == (sysLocalTime.year % 4) && (0 != (sysLocalTime.year % 100))))?29:28;
    sysLocalTime.second++;
    if(sysLocalTime.second >= 60)
    {
        sysLocalTime.second = 0;
        sysLocalTime.minute++;
    }
    if(sysLocalTime.minute >= 60)
    {
        sysLocalTime.minute = 0;
        sysLocalTime.hour++;
    }
    if(sysLocalTime.hour >= 24)
    {
        sysLocalTime.hour = 0;
        sysLocalTime.day++;
    }
    if(sysLocalTime.day > days[sysLocalTime.month])
    {
        sysLocalTime.day = 1;
        sysLocalTime.month++;
    }
    if(sysLocalTime.month > 12)
    {
        sysLocalTime.month = 1;
        sysLocalTime.year++;
    }
    //TIMER_LOG_DEBUG("time %02d:%02d:%02d\r\n", sysLocalTime.hour, sysLocalTime.minute, sysLocalTime.second);
    m_timer_clock_init();
}

/**
* @brief timer clock
* @param [in] none
* @param [out] none
* @return none
* @data 2020/02/20 18:12
* @author maliwen
* @note none
*/
void m_timer_clock_init(void)
{
    /* Now starting a timer */
    const timer_id tmClockId = TimerCreate(TIMER_CLOCK_TIMEOUT, TRUE, m_timer_clock_handler);
    
    /* If a timer could not be created, panic to restart the app */
    if (tmClockId == TIMER_INVALID)
    {
        TIMER_LOG_ERROR("tmClockId create failed.\r\n");
        
        /* Panic with panic code 0xfe */
        //Panic(0xfe);
    }
}

/**
* @brief uart initialize
* @param [in] none
* @param [out] none
* @return none
* @data 2020/02/20 17:31
* @author maliwen
* @note none
*/
void m_timer_init(void)
{
    //TimerInit(MAX_TIMERS, (void *)app_timers);
    
    m_timer_clock_init();
}

/**
* @brief check the time format is valid
* @param [in] tm - time format to be check
* @param [out] none
* @return bool - time format correct or not
* @data 2020/03/20 10:35
* @author maliwen
* @note none
*/
static bool m_time_check(time_t *tm)
{
    time_t *sysTime = m_get_time();
    days[2] = (0 == (sysTime->year % 400) || (0 == (sysTime->year % 4) && (0 != (sysTime->year % 100))))?29:28;
    
    /** check if the time format is correct */
    if((tm->year > 2099) || (tm->month > 12) || (tm->day > days[tm->month]) || 
       (tm->hour > 23) || (tm->minute > 59) || (tm->second > 59))
        return FALSE;
    
    return TRUE;
}

/**
* @brief compare two time who is early or later
* @param [in] curTime - time needs to compare
*             sysTime - time needs to be compared(usually the system time)
* @param [out] none
* @return -2 - curTime format error
*         -1 - if curTime is earllier than sysTime(or time error)
*          0 - if curTime is equal to sysTime
*          1 - if curTime is sysTime
* @data 2020/03/19 15:47
* @author maliwen
* @note none
*/
static int8 m_time_cmp(time_t* curTime, time_t* sysTime)
{
    int8 res = -2;

    /** check if the time format is correct */
    if(m_time_check(curTime) == FALSE)
    {
        return res;
    }
    
    /** check if the time format is correct */
    if(m_time_check(sysTime) == FALSE)
    {
        return res;
    }

    if(curTime->year < sysTime->year)           { /** TIMER_LOG_WARNING("year:%04d - %04d\r\n", curTime->year, sysTime->year); */ return -1; }
    else if(curTime->year > sysTime->year)      { /** TIMER_LOG_WARNING("year:%04d - %04d\r\n", curTime->year, sysTime->year); */ return 1; }
    if(curTime->month < sysTime->month)         { /** TIMER_LOG_WARNING("month:%02d - %02d\r\n", curTime->month, sysTime->month); */ return -1; }
    else if(curTime->month > sysTime->month)    { /** TIMER_LOG_WARNING("month:%02d - %02d\r\n", curTime->month, sysTime->month); */ return 1; }
    if(curTime->day < sysTime->day)             { /** TIMER_LOG_WARNING("day:%02d - %02d\r\n", curTime->day, sysTime->day); */ return -1; }
    else if(curTime->day > sysTime->day)        { /** TIMER_LOG_WARNING("day:%02d - %02d\r\n", curTime->day, sysTime->day); */ return 1; }
    if(curTime->hour < sysTime->hour)           { /** TIMER_LOG_WARNING("hour:%02d - %02d\r\n", curTime->hour, sysTime->hour); */ return -1; }
    else if(curTime->hour > sysTime->hour)      { /** TIMER_LOG_WARNING("hour:%02d - %02d\r\n", curTime->hour, sysTime->hour); */ return 1; }
    if(curTime->minute < sysTime->minute)       { /** TIMER_LOG_WARNING("minute:%02d - %02d\r\n", curTime->minute, sysTime->minute); */ return -1; }
    else if(curTime->minute > sysTime->minute)  { /** TIMER_LOG_WARNING("minute:%02d - %02d\r\n", curTime->minute, sysTime->minute); */ return 1; }
    if(curTime->second < sysTime->second)       { /** TIMER_LOG_WARNING("second:%02d - %02d\r\n", curTime->second, sysTime->second); */ return -1; }
    else if(curTime->second > sysTime->second)  { /** TIMER_LOG_WARNING("second:%02d - %02d\r\n", curTime->second, sysTime->second); */ return 1; }
    else 
    {
        /** TIMER_LOG_WARNING("%04d/%02d/%02d %02d:%02d:%02d - %04d/%02d/%02d %02d:%02d:%02d\r\n", 
                             curTime->year, curTime->month, curTime->day, curTime->hour, curTime->minute, curTime->second,
                             sysTime->year, sysTime->month, sysTime->day, sysTime->hour, sysTime->minute, sysTime->second); */
        return 0; 
    }
}

/**
* @brief use current time to sync the system time
* @param [in] tm - current time
* @param [out] none
* @return bool time sync ok or not
* @data 2020/03/17 10:56
* @author maliwen
* @note none
*/
static bool m_time_sync(time_t *tm)
{
    time_t *sysTime = m_get_time();
    int8 res = m_time_cmp(tm, sysTime);

    /** current time is ready to sync the system time */
    if(res > 0)
    {
        m_set_time(tm);
        TIMER_LOG_DEBUG("set time ok to %02d/%02d/%02d %02d:%02d:%02d\r\n", 
                        sysTime->year, sysTime->month, sysTime->day, sysTime->hour, sysTime->minute, sysTime->second);
        return TRUE;
    }
    else
    {
        TIMER_LOG_WARNING("set time faild: %02d/%02d/%02d %02d:%02d:%02d => %02d/%02d/%02d %02d:%02d:%02d\r\n", 
                          tm->year, tm->month, tm->day, tm->hour, tm->minute, tm->second, 
                          sysTime->year, sysTime->month, sysTime->day, sysTime->hour, sysTime->minute, sysTime->second); 
        return FALSE;
    }
}

/**
* @brief formating the current time string to system time format
* @param [in] timeStr - time string
* @param [out] tm - system time format
* @return bool - whether the time convert succeed.
* @data 2020/03/20 09:22
* @author maliwen
* @note none
*/
bool m_time_formating(uint8* timeStr, time_t* tm)
{
    tm->year = (uint8)((timeStr[0]-'0')*1000 + (timeStr[1]-'0')*1000 + (timeStr[2]-'0')*10 + (timeStr[3]-'0'));
    tm->month = (uint8)((timeStr[4]-'0')*10 + (timeStr[5]-'0'));
    tm->day = (uint8)((timeStr[6]-'0')*10 + (timeStr[7]-'0'));
    tm->hour = (uint8)((timeStr[9]-'0')*10 + (timeStr[10]-'0'));
    tm->minute = (uint8)((timeStr[11]-'0')*10 + (timeStr[12]-'0'));
    tm->second = (uint8)((timeStr[13]-'0')*10 + (timeStr[14]-'0'));
        
    /** check if the time format is correct */
    if(m_time_check(tm) == FALSE)
    {
        TIMER_LOG_WARNING("time format error: %s to %02d/%02d/%02d %02d:%02d:%02d\r\n", 
                        timeStr, tm->year, tm->month, tm->day, tm->hour, tm->minute, tm->second);
        return FALSE;
    }

    return TRUE;
}

/**
* @brief use ancs massage to set the system time
* @param [in] ancs time string
* @param [out] none
* @return bool time sync ok or not
* @data 2020/03/20 10:05
* @author maliwen
* @note none
*/
bool m_ancs_set_time(uint8* timeStr)
{
    time_t tm;
        
    if(m_time_formating(timeStr, &tm) == TRUE)
    {
        return m_time_sync(&tm);
    }
    
    return FALSE;
}
