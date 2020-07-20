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

/*
TA601
TA602
TA301
FO601
*/
#define DEVICE_MODEL_INFO       "FO601"
#define HARDWARE_REVISION       "v1.0.0"
#define FIRMWARE_REVISION       "v1.2.0"

#define USE_BLE_LOG         1
#define USE_PARAM_STORE     0
#define USE_NVM_TEST        0
#define USE_UART_PRINT      0
#define USE_PANIC_PRINT     0
#define USE_WHITELIST_ADV   0
#define USE_ADV_DATA        1  
#define USE_PAIR_CODE_0000  1 // test mode, use pair code of 0x0000 as a bypass code 
#define USE_CMD_ZERO_ADJUST 0
#define USE_NEW_DAV_NAME    0
#define USE_DEV_CALORIE     0 // calorie is calced by app

/* Timer value for starting the Discovery Procedure once the connection is
 * established. 
 */
#define DISCOVERY_START_TIMER                     (300 * MILLISECOND)

/* Extra Long button press time duration */
#define EXTRA_LONG_BUTTON_PRESS_TIMER             (5 * SECOND)

/* Short button press time duration */
#define SHORT_BUTTON_PRESS_TIMER                  (2 * SECOND)

#if USE_NEW_DAV_NAME
#define BLE_ADVERTISING_NAME    "foxter01"
#else
#define BLE_ADVERTISING_NAME    "FOXTER"
#endif

#endif /* __USER_CONFIG_H__ */
