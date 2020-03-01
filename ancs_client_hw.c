/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      ancs_client_hw.c
 *
 *  DESCRIPTION
 *      This file defines all the function which interact with hardware
 *
 *  NOTES
 *
 ******************************************************************************/
 
/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <main.h>
#include <pio_ctrlr.h>
#include <pio.h>
#include <timer.h>
#include <ls_app_if.h>
#include <mem.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "ancs_client.h"
#include "user_config.h"
#include "lcd_display.h"
/*============================================================================*
 *  Private Data types
 *============================================================================*/
/* Application Hardware data structure */
typedef struct _APP_HW_DATA_T
{
    /* Timer for button press */
    timer_id                    button_press_tid;
} APP_HW_DATA_T;

/*============================================================================*
 *  Private Data 
 *============================================================================*/
/* ANCS tag application hardware data structure */
APP_HW_DATA_T g_app_hw_data;

/* Variable to track the LCD display index */
uint16 g_current_display_index = 0;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Handle extra long button press */
static void handleExtraLongButtonPress(timer_id tid);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleExtraLongButtonPress
 *
 *  DESCRIPTION
 *      This function contains handling of  long button press, which
 *      triggers pairing / bonding removal.
 *
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleExtraLongButtonPress(timer_id tid)
{
    if(tid == g_app_hw_data.button_press_tid)
    {
        /* Re-initialise button press timer */
        g_app_hw_data.button_press_tid = TIMER_INVALID;
        
       /* Handle pairing removal */
        HandlePairingRemoval();

    } /* Else ignore timer */
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      InitAncsHardware  -  initialise application hardware
 *
 *  DESCRIPTION
 *      This function is called upon a power reset to initialise the PIOs
 *      and configure their initial states.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void InitAncsHardware(void)
{
    /* Don't wakeup on UART RX line */
    SleepWakeOnUartRX(FALSE);

    /* Setup PIOs
     * PIO11 - Button - BUTTON_PIO
     */
    PioSetModes(PIO_BIT_MASK(BUTTON_PIO), pio_mode_user);
    PioSetDir(BUTTON_PIO, PIO_DIRECTION_INPUT); /* input */
    PioSetPullModes(PIO_BIT_MASK(BUTTON_PIO), pio_mode_strong_pull_up); 
    /* Setup button on PIO11 */
    PioSetEventMask(PIO_BIT_MASK(BUTTON_PIO), pio_event_mode_both);

    /* Set the I2C pins to pull down */
    PioSetI2CPullMode(pio_i2c_pull_mode_strong_pull_down);
    
#ifdef ENABLE_LCD_DISPLAY
    /* Initialize the LCD display PIO */
	LcdDisplayInitI2c();
	LcdDisplayInitRst();

    /* Initialize the LCD display */
    LcdDisplayInit(LCD_I2C_ADDRESS);    
    
    /* Initialize the LCD display PWM */    
    LcdDisplayInitPwm();
		
#endif /*ENABLE_LCD_DISPLAY*/   
  
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppHwDataInit
 *
 *  DESCRIPTION
 *      This function Initialises the application hardware data
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void AppHwDataInit(void)
{
    /* Clear the button press timer id. */
    TimerDelete(g_app_hw_data.button_press_tid);
    g_app_hw_data.button_press_tid = TIMER_INVALID;
}  

#ifdef ENABLE_LCD_DISPLAY
/*----------------------------------------------------------------------------*
 *  NAME
 *      WriteDataToLcdDisplay
 *
 *  DESCRIPTION
 *      This function writes the data to LCD display.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void WriteDataToLcdDisplay(const char* data,
								   uint8 len,
								   bool isdatasource)
{
    if(!isdatasource)
    {
       /* Write the received ANCS notification */  
		 LcdDisplayBacklight(5, 10); /* Backlight on (dim flash). */
         LcdDisplaySetCursor(LCD_I2C_ADDRESS, 0, 0);
		 LcdDisplayI2cWriteString(LCD_I2C_ADDRESS,
                                  (const uint8 *)data);
    }
    else
    {
       /* Write the received data source notification */ 
       LcdDisplaySetCursor(LCD_I2C_ADDRESS, g_current_display_index, 1);
	   
	   /* Update the display index. If in case of partial data received,
		* We will display only the available data. The remaining data will
		* be displayed starting from the g_current_display_index.
		*/
	   g_current_display_index += len;
	   LcdDisplayI2cWriteString(LCD_I2C_ADDRESS,
                                (const uint8 *)data);
    }
}
/*----------------------------------------------------------------------------*
 *  NAME
 *      ClearLCDDisplay
 *
 *  DESCRIPTION
 *      This function clears the LCD display.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void ClearLCDDisplay(void)
{
	/* Reset the current display index to start */
	g_current_display_index = 0;
	
	/*Clear the display*/
    LcdDisplayClear(LCD_I2C_ADDRESS);
}
#endif /*ENABLE_LCD_DISPLAY */

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandlePIOChangedEvent
 *
 *  DESCRIPTION
 *      This function handles PIO Changed event
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void HandlePIOChangedEvent(uint32 pio_changed)
{
  /* PIO changed */
    uint32 pios = PioGets();

    if(pio_changed & BUTTON_PIO_MASK)
    {
        if(!(pios & BUTTON_PIO_MASK))
        {
            /* This event is triggered when a button is pressed. */

            /* Start a timer for SHORT_BUTTON_PRESS_TIMER seconds. If the
             * timer expires before the button is released an extra long button
             * press is detected. If the button is released before the timer
             * expires a short button press is detected.
             */
            TimerDelete(g_app_hw_data.button_press_tid);
            g_app_hw_data.button_press_tid = TIMER_INVALID;

            g_app_hw_data.button_press_tid =
                TimerCreate(EXTRA_LONG_BUTTON_PRESS_TIMER,
                                           TRUE, handleExtraLongButtonPress);
        }
        else
        {
            /* This event comes when a button is released. */
            if(g_app_hw_data.button_press_tid != TIMER_INVALID)
            {
               /* Timer was already running. This means it was a very short
                * button press.
                */
                TimerDelete(g_app_hw_data.button_press_tid);
                g_app_hw_data.button_press_tid = TIMER_INVALID;

                HandleShortButtonPress();
            }
       }
    }
}
