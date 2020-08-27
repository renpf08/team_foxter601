/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      app_gatt.h
 *
 *  DESCRIPTION
 *      Header definitions for common application attributes
 *
 ******************************************************************************/
#ifndef __APP_GATT_H__
#define __APP_GATT_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <panic.h>
#include <time.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/

#include "user_config.h"

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Time out values for fast advertisements */
#define FAST_CONNECTION_ADVERT_TIMEOUT_VALUE        (30 * SECOND)

/* Time out values for slow advertisements */
#define SLOW_CONNECTION_ADVERT_TIMEOUT_VALUE        (0 * MINUTE)

/* Maximum length of Device Name
 * Note: Do not increase device name length beyond (DEFAULT_ATT_MTU -3 = 20)
 * octets as GAP service at the moment doesn't support handling of Prepare
 * write and Execute write procedures.
 */
#define DEVICE_NAME_MAX_LENGTH                      (20)

/* Maximum number of words in central device IRK */
#define MAX_WORDS_IRK                               (8)

/*Number of IRKs that application can store */
#define MAX_NUMBER_IRK_STORED                       (1)

/* Maximum length of the data to be sent to the peer device */
#define SERIAL_RX_DATA_LENGTH           (20)

/* AD Type for Appearance */
#define AD_TYPE_APPEARANCE                          (0x19)

/* This constant is used in defining  some arrays so it should always be large
 * enough to hold the advertisement data.
 */
#define MAX_ADV_DATA_LEN                            (31)


#define GAP_CONN_PARAM_TIMEOUT                      (30 * SECOND)


/* Extract low order byte of 16-bit  */
#define LE8_L(x)                                    ((x) & 0xff)

/* Extract high order byte of 16-bit  */
#define LE8_H(x)                                    (((x) >> 8) & 0xff)

/* TGAP(conn_pause_peripheral) defined in Core Specification Addendum 3 Revision
 * 2. A Peripheral device should not perform a Connection Parameter Update proc-
 * -edure within TGAP(conn_pause_peripheral) after establishing a connection.
 */
#define TGAP_CPP_PERIOD                             (5 * SECOND)

/* TGAP(conn_pause_central) defined in Core Specification Addendum 3 Revision 2.
 * After the Peripheral device has no further pending actions to perform and the
 * Central device has not initiated any other actions within TGAP(conn_pause_ce-
 * -ntral), then the Peripheral device may perform a Connection Parameter Update
 * procedure.
 */
#define TGAP_CPC_PERIOD                             (1 * SECOND)


/* Slave device is not allowed to transmit another Connection Parameter
 * Update request till time TGAP(conn_param_timeout). Refer to section 9.3.9.2,
 * Vol 3, Part C of the Core 4.0 BT spec. The application should retry the
 * 'Connection Parameter Update' procedure after time TGAP(conn_param_timeout)
 * which is 30 seconds.
 */
#define GAP_CONN_PARAM_TIMEOUT                      (30 * SECOND)

/* Timer value for remote device to re-encrypt the link using old keys */
#define BONDING_CHANCE_TIMER                        (30*SECOND)

/* Timer value for remote device to retry pairing */
#define PAIRING_WAIT_TIMER             (2 * MINUTE)

/* Invalid UCID indicating application is not connected to any peer device */
#define GATT_INVALID_UCID                           (0xFFFF)

/* Invalid Attribute Handle */
#define INVALID_ATT_HANDLE                          (0x0000)

/* GATT ERROR codes:
 * Going forward the following codes will be included in the firmware APIs.
 */

/* This error codes should be returned when a remote connected device writes a
 * configuration which the application does not support.
 */
#define gatt_status_desc_improper_config    \
                                                    (0xFD| gatt_status_app_mask)

/* The following error codes shall be returned when a procedure is already
 * ongoing and the remote connected device request for the same procedure
 * again.
 */
#define gatt_status_proc_in_progress                (0xFE| gatt_status_app_mask)

/* This error code shall be returned if the written value is out of the
 * supported range.
 */
#define gatt_status_att_val_oor                     (0xFF| gatt_status_app_mask)


/* UUID for client configuration descriptor */
#define UUID_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC \
                                                    (0x2902)

/* Highest possible handle for ATT database. */
#define ATT_HIGHEST_POSSIBLE_HANDLE                 (0xFFFF)

/* Extract 3rd byte (bits 16-23) of a uint24 variable */
#define THIRD_BYTE(x)                   \
                                       ((uint8)((((uint24)x) >> 16) & 0x0000ff))

/* Extract 2rd byte (bits 8-15) of a uint24 variable */
#define SECOND_BYTE(x)                  \
                                       ((uint8)((((uint24)x) >> 8) & 0x0000ff))

/* Extract least significant byte (bits 0-7) of a uint24 variable */
#define FIRST_BYTE(x)                  ((uint8)(((uint24)x) & 0x0000ff))

/* Convert a word-count to bytes */
#define WORDS_TO_BYTES(_w_)                         (_w_ << 1)

/* Convert bytes to word-count*/
#define BYTES_TO_WORDS(_b_)                         (((_b_) + 1) >> 1)

/* The Maximum Transmission Unit length supported by this device. */
#define ATT_MTU                                     (23)

/* The maximum user data that can be carried in each radio packet.
 * MTU minus 3 bytes header
 */
#define MAX_DATA_LENGTH                             (ATT_MTU-3)

/*============================================================================*
 *  Public Data Types
 *============================================================================*/

/* GATT client characteristic configuration value [Ref GATT spec, 3.3.3.3]*/
typedef enum
{
    gatt_client_config_none = 0x0000,
    gatt_client_config_notification = 0x0001,
    gatt_client_config_indication = 0x0002,
    gatt_client_config_reserved = 0xFFF4
} gatt_client_config;


/* Application defined panic codes
 * Persistent storage which is used to hold panic code is initialised to zero,
 * so the application shall not use 0 for panic codes
 */
typedef enum
{
    /* Failure while setting advertisement parameters */
    app_panic_set_advert_params = 1,

    /* Failure while setting advertisement data */
    app_panic_set_advert_data,

    /* Failure while setting scan response data */
    app_panic_set_scan_rsp_data,

    /* Failure while establishing connection */
    app_panic_connection_est,

    /* Failure while registering GATT DB with firmware */
    app_panic_db_registration,

    /* Failure while reading NVM */
    app_panic_nvm_read,

    /* Failure while writing NVM */
    app_panic_nvm_write,

    /* Failure while reading Tx Power Level */
    app_panic_read_tx_pwr_level,

    /* Failure while deleting device from whitelist */
    app_panic_delete_whitelist,

    /* Failure while adding device to whitelist */
    app_panic_add_whitelist,

    /* Failure while triggering connection parameter update procedure */
    app_panic_con_param_update,

    /* Event received in an unexpected application state */
    app_panic_invalid_state,

    /* Unexpected beep type */
    app_panic_unexpected_beep_type,

    /* Failure while setting advertisement parameters */
    app_panic_gap_set_mode,

    /* Not supported UUID */
    app_panic_uuid_not_supported,

    /* Failure while setting scan parameters */
    app_panic_set_scan_params,

    /* If there is an error returned while initiating a gatt procedure */
    app_panic_gatt_proc_fail,

    /* Unknown application panic */
    app_panic_unknown,
    
    /* Failure while erasing NVM */
    app_panic_nvm_erase,

	/* Timer create fail*/
	app_timer_create_fail,
}app_panic_code;


typedef enum app_state_tag
{
    /* Initial state */
    app_init = 0,

    /* Fast undirected advertisements configured */
    app_fast_advertising = 1,

    /* Slow undirected advertisements configured */
    app_slow_advertising = 2,

     /* Enters when application is in connected state */
    app_connected = 3,

    /* Enters when disconnect is initiated by the application */
    app_disconnecting = 4,

    /** add by mlw, 20200528 11:21 */
    app_pairing = 5,
    app_pairing_ok = 6,
    app_advertising = 7,
	
    /* Idle state */
    app_idle = 8,

    /* Unknown state */
    app_state_unknown

} app_state;


typedef enum
{
    /* The status of a requested command is successful. */
    error_success,

    /* The requested command is not allowed. */
    error_cmd_not_allowed,

    /* The requested command cannot be processed as the device is not ready
     * i.e. maybe not connected.
     */
    error_not_ready,

    /* The requested command cannot be processed as another command is already
     * in progress.
     */
    error_cmd_inprogress,

}app_error_code;

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/
/* This function checks if application is bonded to any device or not */
extern bool AppIsDeviceBonded(void);

/* Returns the connection ID of the application */
extern uint16 GetConnectionID(void);

/* This function returns the current state of the device. */
extern app_state AppGetState(void);

/* This function handles read operation on attributes (as received in
 * GATT_ACCESS_IND message) maintained by the application
 */
extern void HandleAccessRead(GATT_ACCESS_IND_T *p_ind);

/* This function handles Write operation on attributes (as received in
 * GATT_ACCESS_IND message) maintained by the application.
 */
extern void HandleAccessWrite(GATT_ACCESS_IND_T *p_ind);

/* This function starts advertising */
extern void GattStartAdverts(bool fast_connection);

/* This function stops advertising */
extern void GattStopAdverts(void);

/* This function checks if the argument address is resolvable or not */
extern bool GattIsAddressResolvableRandom(TYPED_BD_ADDR_T *p_addr);

/* Triggers fast advertising */
extern void GattTriggerFastAdverts(void);

#ifdef NVM_TYPE_FLASH
/* This function writes the application data to NVM. This function should
 * be called on getting nvm_status_needs_erase
 */
extern void WriteApplicationAndServiceDataToNVM(void);
#endif /* NVM_TYPE_FLASH */

/* This function sets the temporary handle to the parameter value. */
extern void SetTempReadWriteHandle(uint16 handle);

/* This function gets the temporary handle which the application maintains
 * while performing reads and writes.
 */
extern uint16 GetTempReadWriteHandle(void);


/* This function handles connect request requested from a host application */
extern void HandleConnectReq(void);

/* This function handles disconnect request requested from a host application */
extern void HandleDisconnectReq(void);


#endif /* __APP_GATT_H__ */
