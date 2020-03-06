/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 * FILE
 *      user_config.h
 *
 * DESCRIPTION
 *      This file contains definitions which will enable customization of the
 *      application.
 *
 ******************************************************************************/

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

/*=============================================================================*
 *  Public Definitions
 *============================================================================*/

/* The PAIRING_SUPPORT macro controls whether pairing and encryption code is
 * compiled. This flag may be disabled for the applications that do not require
 * pairing.
 */
//#define PAIRING_SUPPORT

/* This macro is required to be disabled if user does not want 
 * to see messages on UART
 */
#define ENABLE_UART

/* Macro used to enable Panic code */
#define ENABLE_DEBUG_PANIC

/* Macro used to enable LCD display */
/*#define ENABLE_LCD_DISPLAY*/

/* Timer values for fast and slow advertisements. */
//#define FAST_CONNECTION_ADVERT_TIMEOUT_VALUE      (30 * SECOND)
//#define SLOW_CONNECTION_ADVERT_TIMEOUT_VALUE      (1 * MINUTE)

/* Defines the High Baud Rate. 115K2 is the maximum supported baud rate */
#define HIGH_BAUD_RATE                   (0x01d9) /* 115K2*/

/* Defines the Low Baud Rate. */ 
#define LOW_BAUD_RATE                    (0x000a) /* 2400 */

/* Timer value for starting the Discovery Procedure once the connection is
 * established. 
 */
#define DISCOVERY_START_TIMER                     (300 * MILLISECOND)

/* Extra Long button press time duration */
#define EXTRA_LONG_BUTTON_PRESS_TIMER             (5 * SECOND)

/* Short button press time duration */
#define SHORT_BUTTON_PRESS_TIMER                  (2 * SECOND)

#ifdef ENABLE_LCD_DISPLAY
#define LCD_I2C_ADDRESS                            (0x3C)                                          
#endif /* ENABLE_LCD_DISPLAY */
#endif /* __USER_CONFIG_H__ */
