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

#define USE_LOG_REPORT  1
#define LOG_MASK    0x0FFF
#if USE_LOG_REPORT
#define LOG_ERROR   0x1000
#define LOG_WARNING 0x2000
#define LOG_INFO    0x4000
#define LOG_DEBUG   0x8000
#else
#define LOG_ERROR   0x0000
#define LOG_WARNING 0x0000
#define LOG_INFO    0x0000
#define LOG_DEBUG   0x0000
#endif

typedef enum
{
    Serial_Service_init_ancs_service_discovering                    = (LOG_DEBUG|1),
    M_Ancs_send_data_source_request                                 = (LOG_DEBUG|2),
    M_Ancs_removed_event_occur                                      = (LOG_DEBUG|3),
    Discovered_Ancs_read_ancs_service_handles_faild                 = (LOG_DEBUG|4),
    Discovered_Ancs_read_ancs_service_handles_ok                    = (LOG_DEBUG|5),
    Discovered_Ancs_write_ancs_service_handles_ok                   = (LOG_DEBUG|6),
    Ancs_Service_Data_cat_id_reserved                               = (LOG_DEBUG|7),
    Ancs_Service_Data_cat_id_entertainment                          = (LOG_DEBUG|8),
    Ancs_Service_Data_cat_id_buf                                    = (LOG_DEBUG|9),
    Ancs_Service_Data_cat_id_hnf                                    = (LOG_DEBUG|10),
    Ancs_Service_Data_cat_id_news                                   = (LOG_DEBUG|11),
    Ancs_Service_Data_cat_id_email                                  = (LOG_DEBUG|12),
    Ancs_Service_Data_cat_id_schedule                               = (LOG_DEBUG|13),
    Ancs_Service_Data_cat_id_social                                 = (LOG_DEBUG|14),
    Ancs_Service_Data_cat_id_vmail                                  = (LOG_DEBUG|15),
    Ancs_Service_Data_cat_id_missed_call                            = (LOG_DEBUG|16),
    Ancs_Service_Data_cat_id_incoming_call                          = (LOG_DEBUG|17),
    Ancs_Service_Data_cat_id_other                                  = (LOG_DEBUG|18),
    Ancs_Service_Data_event_flags_reserved                          = (LOG_DEBUG|19),
    Ancs_Service_Data_event_flags_important                         = (LOG_DEBUG|20),
    Ancs_Service_Data_event_flags_silent                            = (LOG_DEBUG|21),
    Ancs_Service_Data_event_id_reserved                             = (LOG_DEBUG|22),
    Ancs_Service_Data_event_id_removed                              = (LOG_DEBUG|23),
    Ancs_Service_Data_event_id_modified                             = (LOG_DEBUG|24),
    Ancs_Service_Data_event_id_added                                = (LOG_DEBUG|25),
    Ancs_Service_Data_cat_id_location                               = (LOG_DEBUG|26),
    Ancs_Service_Data_attr_id_reserved                              = (LOG_DEBUG|27),
    Ancs_Service_Data_attr_id_date                                  = (LOG_DEBUG|28),
    Ancs_Service_Data_attr_id_message_size                          = (LOG_DEBUG|29),
    Ancs_Service_Data_attr_id_message                               = (LOG_DEBUG|30),
    Ancs_Service_Data_attr_id_sub_title                             = (LOG_DEBUG|31),
    Ancs_Service_Data_attr_id_title                                 = (LOG_DEBUG|32),
    Ancs_Service_Data_attr_id_app_id                                = (LOG_DEBUG|33),
    Ancs_Client_remove_bonding_ok                                   = (LOG_DEBUG|34),
    Ancs_Client_pairing_has_completed_successfully                  = (LOG_DEBUG|35),
    Ancs_Client_ancs_device_initiates_pairing                       = (LOG_DEBUG|36),
    Ancs_Client_bonding_procedure_completed                         = (LOG_DEBUG|37),
    Ancs_Client_ancs_device_re_encrypts                             = (LOG_DEBUG|38),
    Ancs_Client_link_encryption_changed                             = (LOG_DEBUG|39),
    Ancs_Client_database_registration_completed                     = (LOG_DEBUG|40),
    Ancs_Client_ble_state_disconnected                              = (LOG_INFO|41),
    Ancs_Client_ble_state_connected                                 = (LOG_INFO|42),
    Ancs_Client_ble_state_idle                                      = (LOG_INFO|43),
    Ancs_Client_ble_state_slow_advertising                          = (LOG_INFO|44),
    Ancs_Client_ble_state_fast_advertising                          = (LOG_INFO|45),
    Ancs_Client_pairing_removal_default_state                       = (LOG_INFO|46),
    Ancs_Client_pairing_removal_disconnecting_state                 = (LOG_DEBUG|47),
    Ancs_Client_pairing_removal_advertising_state                   = (LOG_DEBUG|48),
    Ancs_Client_pairing_removal_connected_state                     = (LOG_DEBUG|49),
    Ancs_Client_short_button_default_state_ignore                   = (LOG_DEBUG|50),
    Ancs_Client_short_button_idle_state_set_advertising             = (LOG_DEBUG|51),
    Ancs_Client_short_button_advertising_state_restart_advertising  = (LOG_DEBUG|52),
    Ancs_Client_short_button_fast_advertising_state_ignore          = (LOG_DEBUG|53),
    Ancs_Client_attempt_to_write_whitelist1                         = (LOG_DEBUG|54),
    Ancs_Client_something_wrong_to_disconnecting                    = (LOG_DEBUG|55),
    Ancs_Client_unable_to_retrieve_notification_data                = (LOG_DEBUG|56),
    Ancs_Client_initiate_security_request1                          = (LOG_DEBUG|57),
    Ancs_Client_start_slave_security_request                        = (LOG_DEBUG|58),
    Ancs_Client_configure_notifications_for_ancs_and_gatt           = (LOG_DEBUG|59),
    Ancs_Client_configure_for_data_source_cccd                      = (LOG_DEBUG|60),
    Ancs_Client_initiate_security_request2                          = (LOG_DEBUG|61),
    Ancs_Client_start_gatt_database_discovery                       = (LOG_DEBUG|62),
    Ancs_Client_initiate_configuring_ancs_notification_handle       = (LOG_DEBUG|63),
    Ancs_Client_store_the_irk                                       = (LOG_DEBUG|64),
    Ancs_Client_store_the_diversifier                               = (LOG_DEBUG|65),
    Ancs_Client_move_bonded                                         = (LOG_INFO|66),
    Ancs_Client_bonding_disapproved                                 = (LOG_INFO|67),
    Ancs_Client_bonding_approved                                    = (LOG_INFO|68),
    Ancs_Client_pairing_has_failed                                  = (LOG_DEBUG|69),
    Ancs_Client_attempt_to_write_whitelist2                         = (LOG_DEBUG|70),
    Ancs_Client_pairing_complete                                    = (LOG_DEBUG|71),
    Ancs_Client_pairing_request_reject_move_bonded                  = (LOG_DEBUG|72),
    Ancs_Client_pairing_request_authorise                           = (LOG_DEBUG|73),
    Ancs_Client_manual_set_to_advertising_mode1                     = (LOG_DEBUG|74),
    Ancs_Client_ancs_service_handles_is_useful                      = (LOG_DEBUG|75),
    Ancs_Client_ancs_service_discovering                            = (LOG_DEBUG|76),
    Ancs_Client_non_apple_device_initiate_security_request          = (LOG_DEBUG|77),
    Ancs_Client_device_already_be_paired_removed_later_if_need      = (LOG_DEBUG|78),
    Ancs_Client_manual_set_to_advertising_mode2                     = (LOG_DEBUG|79),
    Ancs_Client_ble_set_to_idle_mode_debug                          = (LOG_DEBUG|80),
    Ancs_Client_ble_set_to_disconnect_mode_debug                    = (LOG_DEBUG|81),
    Ancs_Client_ble_mode_no_change_debug                            = (LOG_DEBUG|82),
    Ancs_Client_ble_set_to_fast_mode_debug                          = (LOG_DEBUG|83),
    Ancs_Client_writes_the_application_data_to_nvm_debug            = (LOG_DEBUG|84),
    Ancs_Client_configure_for_gatt_service_changed_indication_debug = (LOG_DEBUG|85),
    Ancs_Client_write_bonded_flag_debug                             = (LOG_DEBUG|86),
    Ancs_Client_bonding_chance_timer_expiried_debug                 = (LOG_DEBUG|87),
    Ancs_Client_read_bonded_device_irk_debug                        = (LOG_DEBUG|88),
    Ancs_Client_write_bonded_status_to_nvm_debug                    = (LOG_DEBUG|89),
    Ancs_Client_nvm_sanity_check_failed_debug                       = (LOG_DEBUG|90),
    Ancs_Client_device_has_not_bonded_debug                         = (LOG_DEBUG|91),
    Ancs_Client_device_has_bonded_read_host_addrs_debug             = (LOG_DEBUG|92),
    Ancs_Client_read_bonding_flag_debug                             = (LOG_DEBUG|93),
    Ancs_Client_system_panic                                        = (LOG_DEBUG|94),
    Ancs_Client_system_started                                      = (LOG_DEBUG|95), 
}log_report_code;

extern void LogReport(const char* file, const char* func, unsigned line, log_report_code log_report_code);

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
