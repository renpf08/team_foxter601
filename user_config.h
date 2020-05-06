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

#define BLE_ADVERTISING_NAME    "foxter01"

#define DEVICE_MODEL_INFO       "foxter601"
#define HARDWARE_REVISION       "v1.0.0"
#define FIRMWARE_REVISION       "v1.2.0"

#define USE_PRINTF_MODE 1

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
