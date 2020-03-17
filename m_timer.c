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

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Number of timers used in this application */
#define MAX_TIMERS 8

/* First timeout at which the timer has to fire a callback */
#define TIMER_TIMEOUT1 (1 * SECOND)

/* Subsequent timeout at which the timer has to fire next callback */
#define TIMER_TIMEOUT2 (3 * SECOND)

#define TIMER_LOG_ERROR(...)        M_LOG_ERROR(__VA_ARGS__)
#define TIMER_LOG_WARNING(...)      M_LOG_WARNING(__VA_ARGS__)
#define TIMER_LOG_INFO(...)         M_LOG_INFO(__VA_ARGS__)
#define TIMER_LOG_DEBUG(...)        M_LOG_DEBUG(__VA_ARGS__)

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* Declare timer buffer to be managed by firmware library */
//static uint16 app_timers[SIZEOF_APP_TIMER * MAX_TIMERS];
static time_t time;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Start timer */
static void startTimer(uint32 timeout, timer_callback_arg handler);

/* Callback after first timeout */
static void timerCallback1(timer_id const id);

/* Callback after second timeout */
static void timerCallback2(timer_id const id);

/* Read the current system time and write to UART */
static void printCurrentTime(void);

/* Convert an integer value into an ASCII string and send to the UART */
static uint8 writeASCIICodedNumber(uint32 value);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      startTimer
 *
 *  DESCRIPTION
 *      Start a timer
 *
 * PARAMETERS
 *      timeout [in]    Timeout period in seconds
 *      handler [in]    Callback handler for when timer expires
 *
 * RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void startTimer(uint32 timeout, timer_callback_arg handler)
{
    /* Now starting a timer */
    const timer_id tId = TimerCreate(timeout, TRUE, handler);
    
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID)
    {
        TIMER_LOG_DEBUG("\r\nFailed to start timer");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      timerCallback1
 *
 *  DESCRIPTION
 *      This function is called when the timer created by TimerCreate expires.
 *      It creates a new timer that will expire after the second timer interval.
 *
 * PARAMETERS
 *      id [in]     ID of timer that has expired
 *
 * RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void timerCallback1(timer_id const id)
{
    const uint32 elapsed = TIMER_TIMEOUT1 / SECOND;
    
    if (elapsed == 1)
        TIMER_LOG_DEBUG("1 second elapsed\r\n");
    else          
    {
        writeASCIICodedNumber(elapsed);
        TIMER_LOG_DEBUG(" seconds elapsed\r\n");
    }

    /* Now start a new timer for second callback */
    startTimer((TIMER_TIMEOUT2 - TIMER_TIMEOUT1), timerCallback2);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      timerCallback2
 *
 *  DESCRIPTION
 *      This function is called when the timer created by TimerCreate expires.
 *      It creates a new timer that will expire after the first timer interval.
 *
 * PARAMETERS
 *      id [in]     ID of timer that has expired
 *
 * RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void timerCallback2(timer_id const id)
{
    const uint32 elapsed = TIMER_TIMEOUT2 / SECOND;
    
    if (elapsed == 1)
        TIMER_LOG_DEBUG("1 second elapsed\r\n");
    else          
    {
        writeASCIICodedNumber(elapsed);
        TIMER_LOG_DEBUG(" seconds elapsed\r\n");
    }

    /* Report current system time */
    printCurrentTime();
    
    /* Now start a new timer for first callback */
    startTimer(TIMER_TIMEOUT1, timerCallback1);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      printCurrentTime
 *
 *  DESCRIPTION
 *      Read the current system time and write to UART.
 *
 * PARAMETERS
 *      None
 *
 * RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void printCurrentTime(void)
{
    /* Read current system time */
    const uint32 now = TimeGet32();
    
    /* Report current system time */
    TIMER_LOG_DEBUG("\n\nCurrent system time: ");
    writeASCIICodedNumber(now / MINUTE);
    TIMER_LOG_DEBUG("m ");
    writeASCIICodedNumber((now % MINUTE)/SECOND);
    TIMER_LOG_DEBUG("s\r\n");
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      writeASCIICodedNumber
 *
 *  DESCRIPTION
 *      Convert an integer value into an ASCII encoded string and send to the
 *      UART
 *
 * PARAMETERS
 *      value [in]     Integer to convert to ASCII and send over UART
 *
 * RETURNS
 *      Number of characters sent to the UART.
 *----------------------------------------------------------------------------*/
static uint8 writeASCIICodedNumber(uint32 value)
{
#define BUFFER_SIZE 11          /* Buffer size required to hold maximum value */
    
    uint8  i = BUFFER_SIZE;     /* Loop counter */
    uint32 remainder = value;   /* Remaining value to send */
    char   buffer[BUFFER_SIZE]; /* Buffer for ASCII string */

    /* Ensure the string is correctly terminated */    
    buffer[--i] = '\0';
    
    /* Loop at least once and until the whole value has been converted */
    do
    {
        /* Convert the unit value into ASCII and store in the buffer */
        buffer[--i] = (remainder % 10) + '0';
        
        /* Shift the value right one decimal */
        remainder /= 10;
    } while (remainder > 0);

    /* Send the string to the UART */
    TIMER_LOG_DEBUG(buffer + i);
    
    /* Return length of ASCII string sent to UART */
    return (BUFFER_SIZE - 1) - i;
}

//------------------------------------------------------------------------------
static void m_timer_clock_handler(timer_id const id);
void m_timer_clock_init(void);
time_t* m_get_time(void) { return &time; }
uint8 days[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
static void m_timer_clock_handler(timer_id const id)
{
    days[2] = (0 == (time.year % 400) || (0 == (time.year % 4) && (0 != (time.year % 100))))?29:28;
    time.second++;
    if(time.second >= 60)
    {
        time.second = 0;
        time.minute++;
    }
    if(time.minute >= 60)
    {
        time.minute = 0;
        time.hour++;
    }
    if(time.hour >= 24)
    {
        time.hour = 0;
        time.day++;
    }
    if(time.day > days[time.month])
    {
        time.day = 1;
        time.month++;
    }
    if(time.month > 12)
    {
        time.month = 1;
        time.year++;
    }
    //TIMER_LOG_DEBUG("time %02d:%02d:%02d\r\n", time.hour, time.minute, time.second);
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
    const timer_id tId = TimerCreate(TIMER_TIMEOUT1, TRUE, m_timer_clock_handler);
    
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID)
    {
        TIMER_LOG_DEBUG("m_timer_clock_init failed\r\n");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}
//------------------------------------------------------------------------------
static void m_timer_clock_handler1(timer_id const id);
void m_timer_clock_init1(void);

static void m_timer_clock_handler1(timer_id const id)
{
    TIMER_LOG_DEBUG("m_timer_clock_handler1\r\n");
    m_timer_clock_init1();
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
void m_timer_clock_init1(void)
{
    /* Now starting a timer */
    const timer_id tId = TimerCreate(TIMER_TIMEOUT1, TRUE, m_timer_clock_handler1);
    
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID)
    {
        TIMER_LOG_DEBUG("m_timer_clock_init failed1\r\n");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}

//------------------------------------------------------------------------------
static void m_timer_clock_handler2(timer_id const id);
void m_timer_clock_init2(void);

static void m_timer_clock_handler2(timer_id const id)
{
    TIMER_LOG_DEBUG("m_timer_clock_handler2\r\n");
    m_timer_clock_init2();
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
void m_timer_clock_init2(void)
{
    /* Now starting a timer */
    const timer_id tId = TimerCreate(TIMER_TIMEOUT1, TRUE, m_timer_clock_handler2);
    
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID)
    {
        TIMER_LOG_DEBUG("m_timer_clock_init failed2\r\n");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}

//------------------------------------------------------------------------------
static void m_timer_clock_handler3(timer_id const id);
void m_timer_clock_init3(void);

static void m_timer_clock_handler3(timer_id const id)
{
    TIMER_LOG_DEBUG("m_timer_clock_handler3\r\n");
    m_timer_clock_init3();
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
void m_timer_clock_init3(void)
{
    /* Now starting a timer */
    const timer_id tId = TimerCreate(TIMER_TIMEOUT1, TRUE, m_timer_clock_handler3);
    
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID)
    {
        TIMER_LOG_DEBUG("m_timer_clock_init failed3\r\n");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}

//------------------------------------------------------------------------------
static void m_timer_clock_handler4(timer_id const id);
void m_timer_clock_init4(void);

static void m_timer_clock_handler4(timer_id const id)
{
    TIMER_LOG_DEBUG("m_timer_clock_handler4\r\n");
    m_timer_clock_init4();
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
void m_timer_clock_init4(void)
{
    /* Now starting a timer */
    const timer_id tId = TimerCreate(TIMER_TIMEOUT1, TRUE, m_timer_clock_handler4);
    
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID)
    {
        TIMER_LOG_DEBUG("m_timer_clock_init failed4\r\n");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}

//------------------------------------------------------------------------------
static void m_timer_clock_handler5(timer_id const id);
void m_timer_clock_init5(void);

static void m_timer_clock_handler5(timer_id const id)
{
    TIMER_LOG_DEBUG("m_timer_clock_handler5\r\n");
    m_timer_clock_init5();
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
void m_timer_clock_init5(void)
{
    /* Now starting a timer */
    const timer_id tId = TimerCreate(TIMER_TIMEOUT1, TRUE, m_timer_clock_handler5);
    
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID)
    {
        TIMER_LOG_DEBUG("m_timer_clock_init failed5\r\n");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}

//------------------------------------------------------------------------------
static void m_timer_clock_handler6(timer_id const id);
void m_timer_clock_init6(void);

static void m_timer_clock_handler6(timer_id const id)
{
    TIMER_LOG_DEBUG("m_timer_clock_handler6\r\n");
    m_timer_clock_init6();
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
void m_timer_clock_init6(void)
{
    /* Now starting a timer */
    const timer_id tId = TimerCreate(TIMER_TIMEOUT1, TRUE, m_timer_clock_handler6);
    
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID)
    {
        TIMER_LOG_DEBUG("m_timer_clock_init failed6\r\n");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
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
    /*m_timer_clock_init1();
    m_timer_clock_init2();
    m_timer_clock_init3();
    m_timer_clock_init4();
    m_timer_clock_init5();
    m_timer_clock_init6();*/

    /* Report current time */
    //printCurrentTime();

    /* Start the first timer */
    //startTimer(TIMER_TIMEOUT1, timerCallback1);
}

/**
* @brief use ancs massage to set the system time
* @param [in] ancs time string
* @param [out] none
* @return none
* @data 2020/03/17 10:56
* @author maliwen
* @note none
*/
void m_timer_set(uint8* timeStr)
{
    time_t tm;
    bool bReFuesh = TRUE;
    
    tm.year = (uint8)((timeStr[0]-'0')*1000 + (timeStr[1]-'0')*1000 + (timeStr[2]-'0')*10 + (timeStr[3]-'0'));
    tm.month = (uint8)((timeStr[4]-'0')*10 + (timeStr[5]-'0'));
    tm.day = (uint8)((timeStr[6]-'0')*10 + (timeStr[7]-'0'));
    tm.hour = (uint8)((timeStr[9]-'0')*10 + (timeStr[10]-'0'));
    tm.minute = (uint8)((timeStr[11]-'0')*10 + (timeStr[12]-'0'));
    tm.second = (uint8)((timeStr[13]-'0')*10 + (timeStr[14]-'0'));
    
    if(tm.year <= time.year)
        if(tm.month <= time.month)
            if(tm.day <= time.day)
                if(tm.hour <= time.hour)
                    if(tm.minute <= time.minute)
                        if(tm.second <= time.second)
                            bReFuesh = FALSE;
    
    if((tm.year > 2099) || (tm.month > 12) || (tm.day > 31) || 
       (tm.hour > 23) || (tm.minute > 59) || (tm.second > 59))
        bReFuesh = FALSE;
    
    if(bReFuesh == TRUE)
    {
        MemCopy(&time, &tm, sizeof(time_t));
        TIMER_LOG_DEBUG("set time ok from %s to %02d/%02d/%02d %02d:%02d:%02d\r\n", 
                        timeStr, time.year, time.month, time.day, time.hour, time.minute, time.second);
    }
    else
    {
        TIMER_LOG_DEBUG("set time faild from %s to %02d/%02d/%02d %02d:%02d:%02d\r\n", 
                        timeStr, time.year, time.month, time.day, time.hour, time.minute, time.second);
    }
    
}