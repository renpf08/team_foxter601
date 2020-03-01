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

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* Declare timer buffer to be managed by firmware library */
//static uint16 app_timers[SIZEOF_APP_TIMER * MAX_TIMERS];

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
        DebugWriteString("\r\nFailed to start timer");
        
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
        m_printf("1 second elapsed\r\n");
    else          
    {
        writeASCIICodedNumber(elapsed);
        m_printf(" seconds elapsed\r\n");
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
        m_printf("1 second elapsed\r\n");
    else          
    {
        writeASCIICodedNumber(elapsed);
        m_printf(" seconds elapsed\r\n");
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
    m_printf("\n\nCurrent system time: ");
    writeASCIICodedNumber(now / MINUTE);
    m_printf("m ");
    writeASCIICodedNumber((now % MINUTE)/SECOND);
    m_printf("s\r\n");
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
    DebugWriteString(buffer + i);
    
    /* Return length of ASCII string sent to UART */
    return (BUFFER_SIZE - 1) - i;
}

//------------------------------------------------------------------------------
static void m_timer_clock_handler(timer_id const id);
void m_timer_clock_init(void);

static void m_timer_clock_handler(timer_id const id)
{
    m_printf("m_timer_clock_handler\r\n");
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
        m_printf("m_timer_clock_init failed\r\n");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}
//------------------------------------------------------------------------------
static void m_timer_clock_handler1(timer_id const id);
void m_timer_clock_init1(void);

static void m_timer_clock_handler1(timer_id const id)
{
    m_printf("m_timer_clock_handler1\r\n");
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
        m_printf("m_timer_clock_init failed1\r\n");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}

//------------------------------------------------------------------------------
static void m_timer_clock_handler2(timer_id const id);
void m_timer_clock_init2(void);

static void m_timer_clock_handler2(timer_id const id)
{
    m_printf("m_timer_clock_handler2\r\n");
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
        m_printf("m_timer_clock_init failed2\r\n");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}

//------------------------------------------------------------------------------
static void m_timer_clock_handler3(timer_id const id);
void m_timer_clock_init3(void);

static void m_timer_clock_handler3(timer_id const id)
{
    m_printf("m_timer_clock_handler3\r\n");
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
        m_printf("m_timer_clock_init failed3\r\n");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}

//------------------------------------------------------------------------------
static void m_timer_clock_handler4(timer_id const id);
void m_timer_clock_init4(void);

static void m_timer_clock_handler4(timer_id const id)
{
    m_printf("m_timer_clock_handler4\r\n");
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
        m_printf("m_timer_clock_init failed4\r\n");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}

//------------------------------------------------------------------------------
static void m_timer_clock_handler5(timer_id const id);
void m_timer_clock_init5(void);

static void m_timer_clock_handler5(timer_id const id)
{
    m_printf("m_timer_clock_handler5\r\n");
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
        m_printf("m_timer_clock_init failed5\r\n");
        
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}

//------------------------------------------------------------------------------
static void m_timer_clock_handler6(timer_id const id);
void m_timer_clock_init6(void);

static void m_timer_clock_handler6(timer_id const id)
{
    m_printf("m_timer_clock_handler6\r\n");
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
        m_printf("m_timer_clock_init failed6\r\n");
        
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