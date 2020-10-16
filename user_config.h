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

#define USE_BLE_LOG             1
#define USE_PARAM_STORE         1
#define USE_UART_PRINT          0
#define USE_PANIC_PRINT         0
#define USE_WHITELIST_ADV       0 
#define USE_ADV_DATA            1  
#define USE_NEW_DAV_NAME        1
#define USE_PAIR_CODE_0000      1 // test mode, use pair code of 0x0000 as a bypass code 
#define USE_ACTIVITY_NOTIFY     0
#define USE_WEEK_FORCE_UPDATE   1

#define USE_LOG_RCVD_DEBUG              0
#if USE_LOG_RCVD_DEBUG
#define USE_LOG_RCVD_SET_NVM            1
#define USE_LOG_RCVD_SET_ZERO_ADJUST    1
#define USE_LOG_RCVD_SET_STEP_COUNT     1
#define USE_LOG_RCVD_SET_LOG_EN         1
#define USE_LOG_RCVD_SET_CHARGE_SWING   1
#define USE_LOG_RCVD_SET_VIBRATION      1
#define USE_LOG_RCVD_SET_VIB_EN         1
#define USE_LOG_RCVD_SET_LOG_SWITCH     1
#define USE_LOG_RCVD_SET_MOTOR_REST     1
#define USE_LOG_RCVD_REQ_SYS_REBOOT     1
#define USE_LOG_RCVD_REQ_CHARGE_STA     1
#define USE_LOG_RCVD_REQ_SYSTEM_TIME    1
#define USE_LOG_RCVD_REQ_BAT_WEEK_MOTOR 1
#else
#define USE_LOG_RCVD_SET_NVM            0
#define USE_LOG_RCVD_SET_ZERO_ADJUST    0
#define USE_LOG_RCVD_SET_STEP_COUNT     0
#define USE_LOG_RCVD_SET_LOG_EN         0
#define USE_LOG_RCVD_SET_CHARGE_SWING   0
#define USE_LOG_RCVD_SET_VIBRATION      0
#define USE_LOG_RCVD_SET_VIB_EN         0
#define USE_LOG_RCVD_SET_LOG_SWITCH     0
#define USE_LOG_RCVD_SET_MOTOR_REST     0
#define USE_LOG_RCVD_REQ_SYS_REBOOT     0
#define USE_LOG_RCVD_REQ_CHARGE_STA     0
#define USE_LOG_RCVD_REQ_SYSTEM_TIME    0
#define USE_LOG_RCVD_REQ_BAT_WEEK_MOTOR 0
#endif

#define USE_LOG_SEND_DEBUG              0
#if USE_LOG_SEND_DEBUG
#define USE_LOG_SEND_STATE_MACHINE      1
#define USE_LOG_SEND_PAIR_CODE          1
#define USE_LOG_SEND_NOTIFY_TYPE        1
#define USE_LOG_SEND_ANCS_APP_ID        1
#define USE_LOG_SEND_GET_CHG_AUTO       1
#define USE_LOG_SEND_GET_CHG_MANUAL     1
#define USE_LOG_SEND_COMPASS_ANGLE      1
#define USE_LOG_SEND_SYSTEM_TIME        1
#define USE_LOG_SEND_RUN_TIME           1
#define USE_LOG_SEND_VIB_STATE          1
#define USE_LOG_SEND_BAT_WEEK_MOTOR     1
#else
#define USE_LOG_SEND_STATE_MACHINE      0
#define USE_LOG_SEND_PAIR_CODE          0
#define USE_LOG_SEND_NOTIFY_TYPE        0
#define USE_LOG_SEND_ANCS_APP_ID        0
#define USE_LOG_SEND_GET_CHG_AUTO       0
#define USE_LOG_SEND_GET_CHG_MANUAL     0
#define USE_LOG_SEND_COMPASS_ANGLE      0
#define USE_LOG_SEND_SYSTEM_TIME        0
#define USE_LOG_SEND_RUN_TIME           0
#define USE_LOG_SEND_VIB_STATE          0
#define USE_LOG_SEND_BAT_WEEK_MOTOR     0
#endif

/* Timer value for starting the Discovery Procedure once the connection is
 * established. 
 */
#define DISCOVERY_START_TIMER                     (300 * MILLISECOND)

/* Extra Long button press time duration */
#define EXTRA_LONG_BUTTON_PRESS_TIMER             (5 * SECOND)

/* Short button press time duration */
#define SHORT_BUTTON_PRESS_TIMER                  (2 * SECOND)

#if USE_NEW_DAV_NAME
//#define BLE_ADVERTISING_NAME    "foxter01"
#define BLE_ADVERTISING_NAME    "FOXTER"
#else
#define BLE_ADVERTISING_NAME    "FOXTER"
#endif

#endif /* __USER_CONFIG_H__ */
