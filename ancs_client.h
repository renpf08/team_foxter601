/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      ancs_client.h
 *
 *  DESCRIPTION
 *      Header file for the application
 *
 *****************************************************************************/

#ifndef __ANCS_CLIENT_H__
#define __ANCS_CLIENT_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <types.h>
#include <bluetooth.h>
#include <timer.h>
#include <config_store.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "ancs_client_gatt.h"
#include "ancs_client_hw.h"
#include "app_gatt.h"

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Timer value for OTA update */
#define OTA_WAIT_TIME                      (15 * MINUTE)

/*============================================================================*
 *  Public Data Types
 *============================================================================*/
/* Structure defined for central device IRK */
typedef struct
{
    uint16                                 irk[MAX_WORDS_IRK];

} CENTRAL_DEVICE_IRK_T;


typedef struct
{

    app_state                               state;

    /* Value for which advertisement timer needs to be started. 
     *
     * The timer is initially started for 30 seconds 
     * to enable fast connections from any device in the vicinity.
     * This is then followed by reduced power advertisements.
     */
    uint32                                  advert_timer_value;

    /* This timer will be used for advertising timeout in advertising state and 
     * for discovery procedure in connected state.
     */
    timer_id                                app_tid;

    /* TYPED_BD_ADDR_T of the host to which application is connected */
    TYPED_BD_ADDR_T                         con_bd_addr;

    /* Track the UCID as clients connect and disconnect */
    uint16                                  st_ucid;

    /* Flag to track the authentication failure during the disconnection */
    bool                                    auth_failure;

    /* Boolean flag to indicated whether the device is bonded */
    bool                                    bonded;

    /* TYPED_BD_ADDR_T of the host to which application is bonded.*/
    TYPED_BD_ADDR_T                         bonded_bd_addr;

    /* variable to keep track of number of connection parameter update 
     * request made 
     */
    uint8                                   num_conn_update_req;

    /* Timer to hold the time elapsed after the last 
     * L2CAP_CONNECTION_PARAMETER_UPDATE Request failed.
     */
    timer_id                                conn_param_update_tid;

    /* Connection Parameter Update timer value. Upon a connection, it's started
     * for a period of TGAP_CPP_PERIOD, upon the expiry of which it's restarted
     * for TGAP_CPC_PERIOD. When this timer is running, if a GATT_ACCESS_IND is
     * received, it means, the central device is still doing the service discov-
     * -ery procedure. So, the connection parameter update timer is deleted and
     * recreated. Upon the expiry of this timer, a connection parameter update
     * request is sent to the central device.
     */
     uint32                                 cpu_timer_value;

    /* Diversifier associated with the LTK of the bonded device */
    uint16                                  diversifier;

    /* Central private address resolution IRK will only be used when
     * central device used resolvable random address. 
     */
    CENTRAL_DEVICE_IRK_T                    central_device_irk;

    /* This boolean variable will be used to keep track of pairing remove 
     * button press.In this application, it will be set to TRUE when user 
     * performs an extra long button press.
     */
    bool                                    pairing_remove_button_pressed;

    /* This timer will be used if the application is already bonded to the 
     * remote host address but the remote device wanted to rebond which we had 
     * declined. In this scenario, we give ample time to the remote device to 
     * encrypt the link using old keys. If remote device doesn't encrypt the 
     * link, we will disconnect the link on this timer expiry.
     */
    timer_id                                bonding_reattempt_tid;

    /* Variable to store the current connection interval being used. */
    uint16                                  conn_interval;

    /* Variable to store the current slave latency. */
    uint16                                  conn_latency;

    /*Variable to store the current connection timeout value. */
    uint16                                  conn_timeout;

  
    /* Variabe to store the attribute handle value which is being 
     * written or read. 
     */
    uint16                     temp_handle;
    
    /* Boolean flag to indicate whether the discovered handles are present in 
     * NVM or not
     */
    bool                       remote_gatt_handles_present;
    
    /* Boolean flag to check if notifications are being configured */
    bool                       notif_configuring;
    
    bool                        pairing_in_progress;
    
    /* OTA connection timeout timer */
    timer_id                                ota_wait_tid;
    
} APP_DATA_T;

/*============================================================================*
 *  Public Data Declarations
 *============================================================================*/
/* Application global data structure */
extern APP_DATA_T g_app_data;

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/
/* This function is used to set the state of the application*/
extern void AppSetState(app_state new_state, uint8 caller);

/* HandleShortButtonPress handles short button presses */
extern void HandleShortButtonPress(void);

/* HandlePairingRemoval clears the cached pairing information */
extern void HandlePairingRemoval(void);

/* This function starts a timer, expiry of which triggers GATT database 
 * discovery procedure. 
 */
extern void DiscoverServices(void);

/* This function gets called on expiry of OTA wait timer. */
extern void OtaTimerHandler(timer_id tid);

#endif /* __ANCS_CLIENT_H__ */
