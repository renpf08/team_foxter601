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

typedef enum
{
    Serial_Service_init_ancs_service_discovering                = 0x0001,
    M_Ancs_send_data_source_request                             = 0x0002,
    M_Ancs_removed_event_occur                                  = 0x0003,
    Discovered_Ancs_read_ancs_service_handles_faild,
    Discovered_Ancs_read_ancs_service_handles_ok,
    Discovered_Ancs_write_ancs_service_handles_ok,
    Ancs_Service_Data_cat_id_reserved,
    Ancs_Service_Data_cat_id_entertainment,
    Ancs_Service_Data_cat_id_buf,
    Ancs_Service_Data_cat_id_hnf,
    Ancs_Service_Data_cat_id_news,
    Ancs_Service_Data_cat_id_email,
    Ancs_Service_Data_cat_id_schedule,
    Ancs_Service_Data_cat_id_social,
    Ancs_Service_Data_cat_id_vmail,
    Ancs_Service_Data_cat_id_missed_call,
    Ancs_Service_Data_cat_id_incoming_call,
    Ancs_Service_Data_cat_id_other,
    Ancs_Service_Data_event_flags_reserved,
    Ancs_Service_Data_event_flags_important,
    Ancs_Service_Data_event_flags_silent,
    Ancs_Service_Data_event_id_reserved,
    Ancs_Service_Data_event_id_removed,
    Ancs_Service_Data_event_id_modified,
    Ancs_Service_Data_event_id_added,
    Ancs_Service_Data_cat_id_location,
    Ancs_Service_Data_attr_id_reserved,
    Ancs_Service_Data_attr_id_date,
    Ancs_Service_Data_attr_id_message_size,
    Ancs_Service_Data_attr_id_message,
    Ancs_Service_Data_attr_id_sub_title,
    Ancs_Service_Data_attr_id_title,
    Ancs_Service_Data_attr_id_app_id,
    Ancs_Client_remove_bonding_ok,
    Ancs_Client_pairing_has_completed_successfully,
    Ancs_Client_ancs_device_initiates_pairing,
    Ancs_Client_bonding_procedure_completed,
    Ancs_Client_ancs_device_re_encrypts,
    Ancs_Client_link_encryption_changed,
    Ancs_Client_database_registration_completed,
    Ancs_Client_ble_state_disconnected,
    Ancs_Client_ble_state_connected,
    Ancs_Client_ble_state_idle,
    Ancs_Client_ble_state_slow_advertising,
    Ancs_Client_ble_state_fast_advertising,
    Ancs_Client_pairing_removal_default_state,
    Ancs_Client_pairing_removal_disconnecting_state,
    Ancs_Client_pairing_removal_advertising_state,
    Ancs_Client_pairing_removal_connected_state,
    Ancs_Client_short_button_default_state_ignore,
    Ancs_Client_short_button_idle_state_set_advertising,
    Ancs_Client_short_button_advertising_state_restart_advertising,
    Ancs_Client_short_button_fast_advertising_state_ignore,
    Ancs_Client_attempt_to_write_whitelist1,
    Ancs_Client_something_wrong_to_disconnecting,
    Ancs_Client_unable_to_retrieve_notification_data,
    Ancs_Client_initiate_security_request1,
    Ancs_Client_start_slave_security_request,
    Ancs_Client_configure_notifications_for_ancs_and_gatt,
    Ancs_Client_configure_for_data_source_cccd,
    Ancs_Client_initiate_security_request2,
    Ancs_Client_start_gatt_database_discovery,
    Ancs_Client_initiate_configuring_ancs_notification_handle,
    Ancs_Client_store_the_irk,
    Ancs_Client_store_the_diversifier,
    Ancs_Client_move_bonded,
    Ancs_Client_bonding_disapproved,
    Ancs_Client_bonding_approved,
    Ancs_Client_pairing_has_failed,
    Ancs_Client_attempt_to_write_whitelist2,
    Ancs_Client_pairing_complete,
    Ancs_Client_pairing_request_reject_move_bonded,
    Ancs_Client_pairing_request_authorise,
    Ancs_Client_manual_set_to_advertising_mode1,
    Ancs_Client_ancs_service_handles_is_useful,
    Ancs_Client_ancs_service_discovering,
    Ancs_Client_non_apple_device_initiate_security_request,
    Ancs_Client_device_already_be_paired_removed_later_if_need,
    Ancs_Client_manual_set_to_advertising_mode2,
    Ancs_Client_ble_set_to_idle_mode_debug,
    Ancs_Client_ble_set_to_disconnect_mode_debug,
    Ancs_Client_ble_mode_no_change_debug,
    Ancs_Client_ble_set_to_fast_mode_debug,
    Ancs_Client_writes_the_application_data_to_nvm_debug,
    Ancs_Client_configure_for_gatt_service_changed_indication_debug,
    Ancs_Client_write_bonded_flag_debug,
    Ancs_Client_bonding_chance_timer_expiried_debug,
    Ancs_Client_read_bonded_device_irk_debug,
    Ancs_Client_write_bonded_status_to_nvm_debug,
    Ancs_Client_nvm_sanity_check_failed_debug,
    Ancs_Client_device_has_not_bonded_debug,
    Ancs_Client_device_has_bonded_read_host_addrs_debug,
    Ancs_Client_read_bonding_flag_debug,
    Ancs_Client_system_panic,
    Ancs_Client_system_started,
    
}log_report_code;

extern void LogReport(const char* file, const char* func, unsigned line, log_report_code log_report_code);
extern int csr_sprintf(char *buf, const char * sFormat, ...);

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
