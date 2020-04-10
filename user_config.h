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

/** The USE_WHITELIST macro */
#define BLE_ADVERTISING_NAME    "foxter01"
#define BLE_HARDWARE_VERSION    "v1.0.2.1"

/**
*  Notification Attribute ID, use to ask for the deatil of data source from
*  notif soure
*  add by mlw at 20200319 12:47
*/
#define REQ_ANCS_NOTIF_ATT_ID_APP_ID		1
#define REQ_ANCS_NOTIF_ATT_ID_TITLE			0
#define REQ_ANCS_NOTIF_ATT_ID_SUBTITLE		0
#define REQ_ANCS_NOTIF_ATT_ID_MESSAGE		0
#define REQ_ANCS_NOTIF_ATT_ID_MESSAGE_SIZE	0
#define REQ_ANCS_NOTIF_ATT_ID_DATE			1
#define REQ_ANCS_NOTIF_ATT_ID_TOTAL         (REQ_ANCS_NOTIF_ATT_ID_APP_ID+     \
                                             REQ_ANCS_NOTIF_ATT_ID_TITLE+      \
                                             REQ_ANCS_NOTIF_ATT_ID_SUBTITLE+   \
                                             REQ_ANCS_NOTIF_ATT_ID_MESSAGE+    \
                                             REQ_ANCS_NOTIF_ATT_ID_DATE+       \
                                             REQ_ANCS_NOTIF_ATT_ID_MESSAGE_SIZE)

/**
*  Did application handle the old message from ANCS? 
*  add by mlw at 20200321 00:26
*/
#define HANDLE_OLD_MSG      0

/**
*  Original ANCS parse module is implement a little complicated, then a new one 
*  is been used
*  add by mlw at 20200318 14:40
*/
#define USE_MY_ANCS         1
#define USE_MY_ANCS_DEBUG   0

/* request bond when after connected.
*  add by mlw at 20200326 11:40
 */
#define USE_CONNECT_BONDING         0

/* The PAIRING_SUPPORT macro controls whether pairing and encryption code is
 * compiled. This flag may be disabled for the applications that do not require
 * pairing.
 */
#define USE_PAIRING_SUPPORT     0

#ifdef RELEASE_MODE
#define USE_M_LOG   1
#else
#define USE_M_LOG   0
#endif

/* Macro used to enable Panic code */
//#define ENABLE_DEBUG_PANIC

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
