/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      ancs_client.c
 *
 *  DESCRIPTION
 *      This file keeps information related to Apple Notification Center
 *      Service 
 *
 *  NOTES
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <pio.h>
#include <main.h>
#include <ls_app_if.h>
#include <nvm.h>
#include <security.h>
#include <mem.h>
#include <macros.h>
#include <csr_ota.h>
#include <buf_utils.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "ancs_client.h"
#include "nvm_access.h"
#include "app_gatt_db.h"
#include "gap_service.h"
#include "battery_service.h"
#include "ancs_service_data.h"
#include "gap_uuids.h"
#include "gatt_uuids.h"
#include "app_gatt.h"
#include "app_procedure_interface.h"
#include "discovery.h"
#include "ancs_service_data.h"
#include "discovered_ancs_service.h"
#include "discovered_gatt_service.h"
#include "user_config.h"
#include "gatt_service.h"
#include "csr_ota_service.h"
#include "m_timer.h"
#include "m_uart.h"
#include "m_printf.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/
/* Maximum number of timers. Two timer will be used for normal application
 * functioning,one in discovery procedure call, one for button press and one for
 * bonding re-attempt if it fails.
 * One extra timer will be used as OTA wait timer if OTA is enabled.OTA wait
 * timer allows the device to update the application software on it if required
 * using OTA service.If no OTA update is there for OTA_WAIT_TIME,disconnect and
 * wait for some one else to connect.
 */
#define MAX_APP_TIMERS                 (6)

/* Magic value to check the sanity of NVM region used by the application */
#define NVM_SANITY_MAGIC               (0xAB1B)

/* NVM offset for NVM sanity word */
#define NVM_OFFSET_SANITY_WORD         (0)

/* NVM offset for bonded flag */
#define NVM_OFFSET_BONDED_FLAG         (NVM_OFFSET_SANITY_WORD + 1)

/* NVM offset for bonded device Bluetooth address */
#define NVM_OFFSET_BONDED_ADDR         (NVM_OFFSET_BONDED_FLAG + \
                                        sizeof(g_app_data.bonded))

/* NVM offset for diversifier */
#define NVM_OFFSET_SM_DIV              (NVM_OFFSET_BONDED_ADDR + \
                                        sizeof(g_app_data.bonded_bd_addr))

/* NVM offset for IRK */
#define NVM_OFFSET_SM_IRK              (NVM_OFFSET_SM_DIV + \
                                        sizeof(g_app_data.diversifier))

/** NVM offset for single bonded flag, add by mlw at 20200313 16:49 */
#define NVM_OFFSET_SINGLE_BONDED_FLAG   (NVM_OFFSET_SM_IRK + \
                                        sizeof(g_app_data.single_bonded))


/* NVM offset where the application stores the boolean flag which tells if the 
 * remote GATT database handles are present or not.
 */
#define NVM_OFFSET_DISC_HANDLES_PRESENT (NVM_OFFSET_SM_IRK + \
                                             sizeof(CENTRAL_DEVICE_IRK_T))

/* Number of words of NVM used by application. Memory used by supported 
 * services is not taken into consideration here.
 */
#define NVM_MAX_APP_MEMORY_WORDS (NVM_OFFSET_DISC_HANDLES_PRESENT + \
                                 sizeof(g_app_data.remote_gatt_handles_present))

/* Macro value may need to be changed if 0x0000 UUID 
 * is assigned by Bluetooth SIG 
 */
#define UUID_INVALID             (0x0000)

#define ANCSC_LOG_ERROR(...)        M_LOG_ERROR(__VA_ARGS__)
#define ANCSC_LOG_WARNING(...)      M_LOG_WARNING(__VA_ARGS__)
#define ANCSC_LOG_INFO(...)         M_LOG_INFO(__VA_ARGS__)
#define ANCSC_LOG_DEBUG(...)        //! M_LOG_DEBUG(__VA_ARGS__)

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* Declare space for application timers. */
static uint16 app_timers[SIZEOF_APP_TIMER * MAX_APP_TIMERS];

/*============================================================================*
 *  Public Data
 *============================================================================*/

/* ANCS application data structure */
APP_DATA_T g_app_data;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Initialises the application data */
static void appDataInit(void);

/* This function reads the NVM */
static void readPersistentStore(void);

/* This function registers  the notifications for discovered services */
static void appConfigureNotifications(uint16 ucid,bool datasource);

/* This function handles the LM_EV_CONNECTION_COMPLETE event*/
static void handleSignalLmEvConnectionComplete(
        LM_EV_CONNECTION_COMPLETE_T *p_event_data);

/* This function handles the GATT_CANCEL_CONNECT_CFM event */
static void handleSignalGattCancelConnectCfm(
        GATT_CANCEL_CONNECT_CFM_T *p_event_data);

/* This function handles the GATT_CONNECT_CFM event */
static void handleSignalGattConnectCfm(GATT_CONNECT_CFM_T *p_event_data);

/* This function is called to request remote master to update the connection 
 * parameters.
 */
static void requestConnParamUpdate(void);

/* This funcrion handles the discovery timer expiry and trigger GATT database 
 * discovery procedure.
 */
static void handleDiscoveryTimerExpiry(timer_id tid);

/* This function is used to request a connection parameter update upon a timer
 * expiry.
 */
static void handleConnParamUpdateRequestTimerExpiry(timer_id tid);

/* This function is used to handle the bonding chance timer expiry. */
static void handleBondingChanceTimerExpiry(timer_id tid);

/* This function handles the LS_CONNECTION_PARAM_UPDATE_CFM event received for 
 * the request made by application.
 */
static void handleSignalLsConnUpdateSignalCfm(
        LS_CONNECTION_PARAM_UPDATE_CFM_T *p_event_data);

/* This function handles the expiry of TGAP(conn_pause_peripheral) timer.
 */
static void handleGapCppTimerExpiry(timer_id tid);

/* This function handles the LM_EV_CONNECTION_UPDATE event */
static void handleSignalLmConnectionUpdate(
        LM_EV_CONNECTION_UPDATE_T* p_event_data);

/* This function handles the LS_CONNECTION_PARAM_UPDATE_IND event received. */
static void handleSignalLsConnParamUpdateInd(
        LS_CONNECTION_PARAM_UPDATE_IND_T *p_event_data);

/* This function handles GATT_ACCESS_IND message for attributes maintained by 
 * the application. 
 */
static void handleSignalGattAccessInd(GATT_ACCESS_IND_T *p_event_data);

/* This function is invoked on reception of LM_EV_DISCONNECT_COMPLETE event.*/
static void handleSignalLmDisconnectComplete(
        LM_EV_DISCONNECT_COMPLETE_T *p_event_data);

/*This function handles the SM_SIMPLE_PAIRING_COMPLETE_IND event */
static void handleSignalSmSimplePairingCompleteInd(
        SM_SIMPLE_PAIRING_COMPLETE_IND_T *p_event_data);

/* This function handles the GATT_ADD_DB_CFM event and starts advertising 
 */
static void handleSignalGattDbCfm(GATT_ADD_DB_CFM_T *p_event_data);

/* This function handles the SM_DIV_APPROVE_IND event*/
static void handleSignalSmDivApproveInd(SM_DIV_APPROVE_IND_T *p_event_data);

/* This function handles the SM_KEYS_IND event*/
static void handleSignalSmKeysInd(SM_KEYS_IND_T *p_event_data);

/* This function handles the signal SM_PAIRING_AUTH_IND event*/
static void handleSignalSmPairingAuthInd(SM_PAIRING_AUTH_IND_T *p_event_data);

/* This function handles LM_EV_ENCRYPTION_CHANGE event received from firmware*/
static void handleSignalLMEncryptionChange(
        LM_EV_ENCRYPTION_CHANGE_T *p_event_data);

/* This function handles the GATT_READ_CHAR_VAL_CFM event */
static void handleGattReadCharValCfm(GATT_READ_CHAR_VAL_CFM_T *p_event_data);

/* This function handles the GATT_WRITE_CHAR_VAL_CFM event */
static void handleGattWriteCharValCfm(GATT_WRITE_CHAR_VAL_CFM_T *p_event_data);

/* This function gets invoked on exiting the app_init state.*/
static void appInitExit(void);

/* This function handles all registered notifications from peer device */
static void handleNotificationData(GATT_CHAR_VAL_IND_T *p_event_data);

/* This function configures the discovered GATT service changed characteristic*/
static sys_status ConfigureGattIndications(void);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      appDataInit
 *
 *  DESCRIPTION
 *      This function is used to initialise ANCS application data 
 *      structure.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void appDataInit(void)
{
   
    /* Make connection Id invalid */
    g_app_data.st_ucid = GATT_INVALID_UCID;
    
    /* Reset the authentication failure flag */
    g_app_data.auth_failure = FALSE;

    /* Reset the application timer */
    TimerDelete(g_app_data.app_tid);
    g_app_data.app_tid = TIMER_INVALID;

    /* Reset the connection parameter update timer. */
    TimerDelete(g_app_data.conn_param_update_tid);
    g_app_data.conn_param_update_tid = TIMER_INVALID;
    g_app_data.cpu_timer_value = 0;

    /* Delete the bonding chance timer */
    TimerDelete(g_app_data.bonding_reattempt_tid);
    g_app_data.bonding_reattempt_tid = TIMER_INVALID;

    /* Reset advertising timer value */
    g_app_data.advert_timer_value = 0;

    /* Reset the connection parameter variables. */
    g_app_data.conn_interval = 0;
    g_app_data.conn_latency = 0;
    g_app_data.conn_timeout = 0;
    
    /* Reset the OTA wait timer */
    TimerDelete(g_app_data.ota_wait_tid);
    g_app_data.ota_wait_tid = TIMER_INVALID;
    
    /* Initialise the hardware data */
    AppHwDataInit();

    /* Initialise GAP Data structure */
    GapDataInit();

    /* Battery Service data initialisation */
    BatteryDataInit();

    /* Initialise GATT Data structure */
    GattDataInit();

    /* OTA Service data initialisation */
    OtaDataInit();
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleDiscoveryTimerExpiry
 *
 *  DESCRIPTION
 *      This function handles the expiry of discovery timer and initiates the 
 *      GATT database discovery procedure.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleDiscoveryTimerExpiry(timer_id tid)
{
    if(tid == g_app_data.app_tid)
    {
        /* Timer has just expired, so mark it as invalid */
        g_app_data.app_tid = TIMER_INVALID;

        /* Start GATT database discovery */
        StartGattDatabaseDiscovery(g_app_data.st_ucid);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      readPersistentStore
 *
 *  DESCRIPTION
 *      This function is used to initialise and read NVM data
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void readPersistentStore(void)
{
    /* NVM offset for supported services */
    uint16 nvm_offset = NVM_MAX_APP_MEMORY_WORDS;
    uint16 nvm_sanity = 0xffff;

    /* Read persistent storage to know if the device was last bonded.
     * If the device was bonded, trigger fast undirected advertisements by
     * setting the white list for bonded host. If the device was not bonded,
     * trigger undirected advertisements for any host to connect.
     */

    Nvm_Read(&nvm_sanity, sizeof(nvm_sanity), NVM_OFFSET_SANITY_WORD);
    ANCSC_LOG_DEBUG("read bonding flag(bond flag: %d).\r\n", g_app_data.bonded);

    if(nvm_sanity == NVM_SANITY_MAGIC)
    {
        /* Read bonded flag from NVM */
        Nvm_Read((uint16*)&g_app_data.bonded, sizeof(g_app_data.bonded),
                           NVM_OFFSET_BONDED_FLAG);
        
        #if !USE_WHITELIST
        /* Read single bonded flag from NVM */
        Nvm_Read((uint16*)&g_app_data.single_bonded, sizeof(g_app_data.single_bonded), NVM_OFFSET_SINGLE_BONDED_FLAG);
        #endif

        if(g_app_data.bonded)
        {

            /* Bonded host BD address will only be stored if bonded flag
             * is set to TRUE.
             */
            Nvm_Read((uint16*)&g_app_data.bonded_bd_addr, 
                       sizeof(TYPED_BD_ADDR_T),
                       NVM_OFFSET_BONDED_ADDR);
            ANCSC_LOG_DEBUG("device has bonded, read host addrs(bond flag: %d).\r\n", g_app_data.bonded);
        }

        else /* Case when we have only written NVM_SANITY_MAGIC to NVM but 
              * application didn't get bonded to any host in the last powered 
              * session.
              */
        {
            g_app_data.bonded = FALSE;       
            #if !USE_WHITELIST
            g_app_data.single_bonded = FALSE;
            #endif
            ANCSC_LOG_DEBUG("device has not bonded.\r\n");
        }

        /* Read the diversifier associated with the presently bonded/last 
         * bonded device.
         */
        Nvm_Read(&g_app_data.diversifier, 
                 sizeof(g_app_data.diversifier),
                 NVM_OFFSET_SM_DIV);
        
        /* Check if there are any valid discovered handles stored in NVM */
        Nvm_Read((uint16 *)&g_app_data.remote_gatt_handles_present, 
                            sizeof(g_app_data.remote_gatt_handles_present), 
                            NVM_OFFSET_DISC_HANDLES_PRESENT);
        
        /* If there are any valid handles stored in NVM, the following function 
         * will read those handles.
         * If there are no valid handles present in NVM, the following function 
         * will only initialise the NVM offsets
         */
        ReadDiscoveredGattDatabaseFromNVM(&nvm_offset, 
                                       g_app_data.remote_gatt_handles_present);


        /* Read device name and length from NVM */
        GapReadDataFromNVM(&nvm_offset);
    }
    else /* NVM sanity check failed means either the device is being brought up 
          * for the first time or memory has got corrupted in which case 
          * discard the data and start fresh.
          */
    {
        ANCSC_LOG_DEBUG("NVM sanity check failed.\r\n");
        nvm_sanity = NVM_SANITY_MAGIC;

        /* Write NVM sanity word to the NVM */
        Nvm_Write(&nvm_sanity, sizeof(nvm_sanity), NVM_OFFSET_SANITY_WORD);

        /* The device will not be bonded as it is coming up for the first time
         */
        g_app_data.bonded = FALSE;

        /* Write bonded status to NVM */
        Nvm_Write((uint16*)&g_app_data.bonded, sizeof(g_app_data.bonded), 
                            NVM_OFFSET_BONDED_FLAG);
        ANCSC_LOG_DEBUG("Write bonded status to NVM(bond flag: %d).\r\n", g_app_data.bonded);
        
        #if !USE_WHITELIST
        g_app_data.single_bonded = FALSE;
        /* Write one word single bonded flag */
        Nvm_Write((uint16*)&g_app_data.single_bonded, sizeof(g_app_data.single_bonded), NVM_OFFSET_SINGLE_BONDED_FLAG);
        #endif
                
        /* When the application is coming up for the first time after flashing 
         * the image to it, it will not have bonded to any device. So, no LTK 
         * will be associated with it. Hence, set the diversifier to 0.
         */
        g_app_data.diversifier = 0;

        /* Write the same to NVM. */
        Nvm_Write(&g_app_data.diversifier, 
                  sizeof(g_app_data.diversifier),
                  NVM_OFFSET_SM_DIV);
 
        /* The application is being initialised for the first time. There are 
         * no discovered handles.
         */
        g_app_data.remote_gatt_handles_present = FALSE;
        Nvm_Write((uint16 *)&g_app_data.remote_gatt_handles_present, 
                            sizeof(g_app_data.remote_gatt_handles_present), 
                            NVM_OFFSET_DISC_HANDLES_PRESENT);
        
        /* Initialise the NVM offsets for the discovered remote database 
         * attribute handles.
         */
        ReadDiscoveredGattDatabaseFromNVM(&nvm_offset, 
                                       g_app_data.remote_gatt_handles_present);

        /* Initialise the NVM offset for the gap service data and store GAP 
         * data in NVM 
         */
        GapInitWriteDataToNVM(&nvm_offset);
    }
    
    /* If device is bonded and bonded address is resolvable then read the 
     * bonded device's IRK
     */
    if(g_app_data.bonded == TRUE && 
        GattIsAddressResolvableRandom(&g_app_data.bonded_bd_addr))
    {
        Nvm_Read((uint16*)g_app_data.central_device_irk.irk,
                            MAX_WORDS_IRK,
                            NVM_OFFSET_SM_IRK);
        ANCSC_LOG_DEBUG("read the bonded device's IRK.\r\n");
    }
    /* Read Battery service data from NVM if the devices are bonded and  
     * update the offset with the number of word of NVM required by 
     * this service
     */
    BatteryReadDataFromNVM(&nvm_offset);
  
    GattReadDataFromNVM(&nvm_offset);
}

/*---------------------------------------------------------------------------
 *  NAME
 *      ConfigureGattIndications
 *
 *  DESCRIPTION
 *      This function configures indications on the GATT service changed 
 *      Characteristics.
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static sys_status ConfigureGattIndications(void)
{
    uint8 val[2], *p_val;
    p_val = val;
    BufWriteUint16(&p_val, gatt_client_config_indication);

    /* Store the ANCS Notification Characteristic Client Configuration in 
     * the temporary handle. Write confirmation event does not contain any 
     * handle value. This handle will tell the application which attribute was 
     * being written.
     */
    SetTempReadWriteHandle(GetGattNotificationCCDHandle());

    return GattWriteCharValueReq(g_app_data.st_ucid, 
                          GATT_WRITE_REQUEST, 
                          GetGattNotificationCCDHandle(), 
                          2, 
                          val);
}
/*----------------------------------------------------------------------------*
 *  NAME
 *      requestConnParamUpdate
 *
 *  DESCRIPTION
 *      This function is used to send L2CAP_CONNECTION_PARAMETER_UPDATE_REQUEST
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void requestConnParamUpdate(void)
{
    ble_con_params app_pref_conn_param;
   
    app_pref_conn_param.con_max_interval = APPLE_MAX_CON_INTERVAL;
    app_pref_conn_param.con_min_interval = APPLE_MIN_CON_INTERVAL;
    app_pref_conn_param.con_slave_latency = APPLE_SLAVE_LATENCY;
    app_pref_conn_param.con_super_timeout = APPLE_SUPERVISION_TIMEOUT;
    
    /* Send connection parameter update request. */
    if(LsConnectionParamUpdateReq(&(g_app_data.con_bd_addr), 
                                     &app_pref_conn_param) != ls_err_none)
    {
        /* Connection parameter update request should not have failed.
         * Report panic 
         */
        ReportPanic(app_panic_con_param_update);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      appConfigureNotifications
 *
 *  DESCRIPTION
 *      This function is used to register the (all)notifications of the
 *      discovered service
 *
 *  RETURNS/MODIFIES
 *      none
 *
 *---------------------------------------------------------------------------*/
static void appConfigureNotifications(uint16 ucid,bool datasource)
{
    if(!datasource)
    {
        /* Configure for ANCS Notification */
        ConfigureAncsNotifications(ucid);
    }
    else
    {
        /* Configure for ANCS Data Source Notification */
        ConfigureAncsDataSourceNotification(ucid);
    }
}

/*---------------------------------------------------------------------------
 *
 *  NAME
 *      handleSignalLmEvConnectionComplete
 *
 *  DESCRIPTION
 *      This function handles the LM_EV_CONNECTION_COMPLETE event
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalLmEvConnectionComplete(
        LM_EV_CONNECTION_COMPLETE_T *p_event_data)
{
    /* Store the connection parameters. */
    g_app_data.conn_interval = p_event_data->data.conn_interval;
    g_app_data.conn_latency = p_event_data->data.conn_latency;
    g_app_data.conn_timeout = p_event_data->data.supervision_timeout;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleConnParamUpdateRequestTimerExpiry
 *
 *  DESCRIPTION
 *      This function is used to request a connection parameter update upon a 
 *      timer expiry.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleConnParamUpdateRequestTimerExpiry(timer_id tid)
{
    if(g_app_data.conn_param_update_tid == tid)
    {
        g_app_data.conn_param_update_tid = TIMER_INVALID;
        g_app_data.cpu_timer_value = 0;

        /* Increment the counter */
        g_app_data.num_conn_update_req++;

        /* Request connection parameter update. */
        requestConnParamUpdate();

    }/* Else it may be due to some race condition. Ignore it. */
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      handleGapCppTimerExpiry
 *
 *  DESCRIPTION
 *      This function handles the expiry of TGAP(conn_pause_peripheral) timer.
 *      It starts the TGAP timer, during which, if no activity is detected from
 *      the central device, a connection parameter update request is sent.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleGapCppTimerExpiry(timer_id tid)
{
    if(g_app_data.conn_param_update_tid == tid)
    {
        g_app_data.conn_param_update_tid = 
                           TimerCreate(TGAP_CPC_PERIOD, TRUE,
                                       handleConnParamUpdateRequestTimerExpiry);
        g_app_data.cpu_timer_value = TGAP_CPC_PERIOD;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleBondingChanceTimerExpiry
 *
 *  DESCRIPTION
 *      This function handles the expiry of bonding chance timer.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleBondingChanceTimerExpiry(timer_id tid)
{
    if(g_app_data.bonding_reattempt_tid== tid)
    {
        g_app_data.bonding_reattempt_tid= TIMER_INVALID;
        /* The bonding chance timer has expired. This means the remote has not
         * encrypted the link using old keys. Disconnect the link.
         */
        AppSetState(app_disconnecting);
        ANCSC_LOG_DEBUG("bonding chance timer expiried.\r\n");
    }/* else it may be due to some race condition. Ignore it. */
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattCancelConnectCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_CANCEL_CONNECT_CFM event
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalGattCancelConnectCfm(GATT_CANCEL_CONNECT_CFM_T 
                                             *p_event_data)
{
    if(p_event_data->result != sys_status_success)
    {
        return;
    }
    if(g_app_data.pairing_remove_button_pressed)
    {
        /* Case when user performed an extra long button press for removing
         * pairing.
         */
        g_app_data.pairing_remove_button_pressed = FALSE;

        /* Reset and clear the whitelist */
        LsResetWhiteList();

        /* Trigger fast advertisements */
        if(g_app_data.state == app_fast_advertising)
        {
            GattTriggerFastAdverts();
        }
        else
        {
            AppSetState(app_fast_advertising);
        }
    }
    else
    {
        /*Handling signal as per current state */
        switch(g_app_data.state)
        {
            /* In case of fast advertising, Start slow advertising
             * In case of slow advertising, stop advertising 
             */
            case app_fast_advertising:
            {
                AppSetState(app_slow_advertising);
            }
            break;
            case app_slow_advertising:
            {
                AppSetState(app_idle);
            }
            break;
            default:
            {
                /* Control should never come here */
                ReportPanic(app_panic_invalid_state);
            }
            break;
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLmDisconnectComplete
 *
 *  DESCRIPTION
 *      This function handles LM Disconnect Complete event which is received
 *      at the completion of disconnect procedure triggered either by the 
 *      device or remote host or because of link loss 
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

static void handleSignalLmDisconnectComplete(
        LM_EV_DISCONNECT_COMPLETE_T *p_event_data)
{
    if(OtaResetRequired())
    {
        OtaReset();
        /* The OtaReset function does not return */
    }
    else
    {
        /* Reset the connection parameter variables. */
        g_app_data.conn_interval = 0;
        g_app_data.conn_latency = 0;
        g_app_data.conn_timeout = 0;

        /* LM_EV_DISCONNECT_COMPLETE event can have following disconnect 
         * reasons:
         *
         * HCI_ERROR_CONN_TIMEOUT - Link Loss case
         * HCI_ERROR_CONN_TERM_LOCAL_HOST - Disconnect triggered by device
         * HCI_ERROR_OETC_* - Other end (i.e., remote host) terminated 
         * connection
         */
        /*Handling signal as per current state */
        switch(g_app_data.state)
        {
            case app_connected: /* FALLTHROUGH */ 
            case app_disconnecting:
            {
                g_app_data.notif_configuring = FALSE;
                appDataInit();
                /* Restart advertising */
                AppSetState(app_fast_advertising);

            }
            break;
            default:
            {
                /* Control should never come here */
                ReportPanic(app_panic_invalid_state);
            }
            break;
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattConnectCfm
 *
 *  DESCRIPTION
 *      This function handles the GATT_CONNECT_CFM event
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalGattConnectCfm(GATT_CONNECT_CFM_T *p_event_data)
{
    /* Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_fast_advertising: /* FALLTHROUGH */ 
        case app_slow_advertising:
        {
            if(p_event_data->result == sys_status_success)
            {
                /* Store bd address */
                g_app_data.con_bd_addr = p_event_data->bd_addr;

                /* Store received UCID */
                g_app_data.st_ucid = p_event_data->cid;

                AppSetState(app_connected);
      

                if(g_app_data.bonded == TRUE && 
                    GattIsAddressResolvableRandom(&g_app_data.bonded_bd_addr) &&
                    (SMPrivacyMatchAddress(&g_app_data.con_bd_addr,
                                           g_app_data.central_device_irk.irk,
                                           MAX_NUMBER_IRK_STORED, 
                                           MAX_WORDS_IRK) < 0))
                {
                    /* Application was bonded to a remote device with
                     * resolvable random address and application has failed to
                     * resolve the remote device address to which we just
                     * connected. So disconnect and start advertising again
                     */
                    g_app_data.auth_failure = TRUE;
                    AppSetState(app_disconnecting);
                    ANCSC_LOG_DEBUG("application was bonded to a remote device, so disconnect and start advertising again.\r\n");
                }
                else
                {
                    ANCSC_LOG_DEBUG("others1.\r\n");
                    /* If we are bonded to this host, then it may be appropriate
                     * to indicate that the database is not now what it had
                     * previously.
                     */
                    if(g_app_data.bonded)
                    {
                        GattOnConnection(p_event_data->cid);
                        ANCSC_LOG_DEBUG("001.\r\n");
                    }

                    /* Initiate slave security request if the remote host 
                     * supports security feature. This is added for this device 
                     * to work against legacy hosts that don't support security
                     */

                    /* Security supported by the remote host */
                    if(!GattIsAddressResolvableRandom(&g_app_data.con_bd_addr))
                    {
                        /* Non-Apple Device.Initiate Security request */
                        SMRequestSecurityLevel(&g_app_data.con_bd_addr);
                        ANCSC_LOG_DEBUG("002.\r\n");
                    }
                    else /* APPLE Device */
                    {
                        ANCSC_LOG_DEBUG("003.\r\n");
                        /* Check if have the remote gatt handles cached */
                        if(!g_app_data.remote_gatt_handles_present)
                        {
                            /* Start Gatt Database discovery. */
                            DiscoverServices(); 
                            ANCSC_LOG_DEBUG("004.\r\n");  
                        }
                    }
                }
            }
            else
            {
                ANCSC_LOG_DEBUG("others2.\r\n");
                /* Else wait for user activity before we start advertising 
                 * again
                 */
                AppSetState(app_idle);
            }
        }
        break;
        default:
        {
            ANCSC_LOG_DEBUG("others3.\r\n");
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        }
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLsConnUpdateSignalCfm
 *
 *  DESCRIPTION
 *      This function handles the LS_CONNECTION_UPDATE_SIGNALLING_RSP event
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalLsConnUpdateSignalCfm(
                                 LS_CONNECTION_PARAM_UPDATE_CFM_T *p_event_data)
{
    /*Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_connected: /* FALLTHROUGH */
        case app_disconnecting:
        {
            /* Received in response to the L2CAP_CONNECTION_PARAMETER_UPDATE 
             * request sent from the slave after service discovery. If the
             * request has failed, the device should again send same request
             * only after Tgap(conn_param_timeout). Refer Bluetooth 4.0
             * spec Vol 3 Part C, Section 9.3.9 and profile spec.
             */
            if (((p_event_data->status) != ls_err_none)
                && (g_app_data.num_conn_update_req < 
                                MAX_NUM_CONN_PARAM_UPDATE_REQS))
            {
                /* Delete timer if running */
                TimerDelete(g_app_data.conn_param_update_tid);
                
                g_app_data.conn_param_update_tid= TimerCreate(
                                       GAP_CONN_PARAM_TIMEOUT,
                                       TRUE, 
                                       handleConnParamUpdateRequestTimerExpiry);
                g_app_data.cpu_timer_value = GAP_CONN_PARAM_TIMEOUT;
            }
        }
        break;
        default:
        {
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        }
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLmConnectionUpdate
 *
 *  DESCRIPTION
 *      This function handles the LM_EV_CONNECTION_UPDATE event
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void handleSignalLmConnectionUpdate(
        LM_EV_CONNECTION_UPDATE_T* p_event_data)
{
    switch(g_app_data.state)
    {
        case app_connected:
        case app_disconnecting:
        {
            /* Store the new connection parameters. */
            g_app_data.conn_interval = p_event_data->data.conn_interval;
            g_app_data.conn_latency = p_event_data->data.conn_latency;
            g_app_data.conn_timeout = p_event_data->data.supervision_timeout;
        }
        break;

        default:
            /* Connection parameter update indication received in unexpected
             * application state.
             */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLsConnParamUpdateInd
 *
 *  DESCRIPTION
 *      This function handles the LS_CONNECTION_PARAM_UPDATE_IND event
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalLsConnParamUpdateInd(
        LS_CONNECTION_PARAM_UPDATE_IND_T *p_event_data)
{
    /* The application had already received the new connection parameters while 
     * handling event LM_EV_CONNECTION_UPDATE.
     * Check if new parameters comply with application preferred parameters. 
     * If not, application shall trigger Connection parameter update procedure 
     */

    /* Delete timer if running */
    TimerDelete(g_app_data.conn_param_update_tid);
    g_app_data.conn_param_update_tid = TIMER_INVALID;
    g_app_data.cpu_timer_value = 0;

    if(g_app_data.conn_interval < APPLE_MIN_CON_INTERVAL ||
       g_app_data.conn_interval > APPLE_MAX_CON_INTERVAL
#if APPLE_SLAVE_LATENCY
       || g_app_data.conn_latency < APPLE_SLAVE_LATENCY
#endif
      )
    {
        /* Set the num of connection update attempts to zero */
        g_app_data.num_conn_update_req = 0;

        /* Start timer to trigger Connection Parameter Update procedure */
        g_app_data.conn_param_update_tid = TimerCreate(GAP_CONN_PARAM_TIMEOUT,
                                     TRUE, 
                                     handleConnParamUpdateRequestTimerExpiry);
        g_app_data.cpu_timer_value = GAP_CONN_PARAM_TIMEOUT;


    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattAccessInd
 *
 *  DESCRIPTION
 *      This function handles GATT_ACCESS_IND message for attributes 
 *      maintained by the application.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

static void handleSignalGattAccessInd(GATT_ACCESS_IND_T *p_event_data)
{

    /*Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_connected:
        {
            /* If connection parameter update timer with value TGAP_CPC_PERIOD 
             * is running, restart the timer.
             */
            if(g_app_data.cpu_timer_value == TGAP_CPC_PERIOD)
            {
                TimerDelete(g_app_data.conn_param_update_tid);
                g_app_data.conn_param_update_tid = TimerCreate(TGAP_CPC_PERIOD,
                                 TRUE, handleConnParamUpdateRequestTimerExpiry);
            }

            /* Received GATT ACCESS IND with write access */
            if(p_event_data->flags == 
                (ATT_ACCESS_WRITE | 
                 ATT_ACCESS_PERMISSION | 
                 ATT_ACCESS_WRITE_COMPLETE))
            {
                HandleAccessWrite(p_event_data);
            }
            /* Received GATT ACCESS IND with read access */
            else if(p_event_data->flags == 
                (ATT_ACCESS_READ | 
                ATT_ACCESS_PERMISSION))
            {
                HandleAccessRead(p_event_data);
            }
            else
            {
                GattAccessRsp(p_event_data->cid, p_event_data->handle, 
                              gatt_status_request_not_supported,
                              0, NULL);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}


/*---------------------------------------------------------------------------
 *
 *  NAME
 *      handleSignalSmPairingAuthInd
 *
 *  DESCRIPTION
 *      This function handles the SM_PAIRING_AUTH_IND event. This message will
 *      only be received when the peer device is initiating 'Just Works' 
 *      pairing.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 
*----------------------------------------------------------------------------*/
static void handleSignalSmPairingAuthInd(SM_PAIRING_AUTH_IND_T *p_event_data)
{
    bool status = FALSE;

    /* Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_connected: /* FALLTHROUGH */
        {
            /* Authorise the pairing request if the application is NOT bonded */
            if(!g_app_data.bonded)
            {
                g_app_data.pairing_in_progress = TRUE;
                status = TRUE;
            } /* else reject the pairing request */

            if(status == TRUE)
            {
                ANCSC_LOG_INFO("pairing request authorise.\r\n");
            }
            else
            {
                ANCSC_LOG_WARNING("pairing request reject.\r\n");
            }
            SMPairingAuthRsp(p_event_data->data, status);
        }
        break;

        default:
            ANCSC_LOG_DEBUG("sm-pairing auth error(code: 0x%02X).\r\n", g_app_data.state);
            ReportPanic(app_panic_invalid_state);
        break;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalSmSimplePairingCompleteInd
 *
 *  DESCRIPTION
 *      This function handles the SM_SIMPLE_PAIRING_COMPLETE_IND event
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalSmSimplePairingCompleteInd(
                                SM_SIMPLE_PAIRING_COMPLETE_IND_T *p_event_data)
{
    /* Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_connected: /* FALLTHROUGH */ 
        {
            if(p_event_data->status == sys_status_success)
            {
                /* Pairing succeeded. Application is bonded now */
                ANCSC_LOG_INFO("pairing complete\r\n");
                
                if(g_app_data.bonding_reattempt_tid != TIMER_INVALID)
                {                   
                    /* Delete the bonding chance timer */
                    TimerDelete(g_app_data.bonding_reattempt_tid);
                    g_app_data.bonding_reattempt_tid = TIMER_INVALID;
                }

                #if USE_WHITELIST //! add by mlw at 20200312 14:47
                g_app_data.bonded = TRUE;
                #if !USE_WHITELIST
                g_app_data.single_bonded = TRUE;
                #endif
                g_app_data.bonded_bd_addr = p_event_data->bd_addr;
                g_app_data.pairing_in_progress = FALSE;


                /* Write one word bonded flag */
                Nvm_Write((uint16*)&g_app_data.bonded, 
                          sizeof(g_app_data.bonded),
                          NVM_OFFSET_BONDED_FLAG);
                ANCSC_LOG_DEBUG("Write one word bonded flag(bond flag: %d).\r\n", g_app_data.bonded);

                /* Write typed Bluetooth address of bonded host */
                Nvm_Write((uint16*)&g_app_data.bonded_bd_addr, 
                         sizeof(TYPED_BD_ADDR_T), NVM_OFFSET_BONDED_ADDR);

                if(!GattIsAddressResolvableRandom(&g_app_data.bonded_bd_addr))
                {
                    /* White list is configured with the bonded host address */
                    if(LsAddWhiteListDevice(&g_app_data.bonded_bd_addr) != 
                                        ls_err_none)
                    {
                        ReportPanic(app_panic_add_whitelist);
                    }
                }

                /* Notify the Gatt service about the pairing */
                GattBondingNotify();     
                #else
                g_app_data.single_bonded = TRUE;
                /* Write one word single bonded flag */
                Nvm_Write((uint16*)&g_app_data.single_bonded, sizeof(g_app_data.single_bonded), NVM_OFFSET_SINGLE_BONDED_FLAG);
                ANCSC_LOG_WARNING("pairing complete, write single bonded flag to NVM.\r\n");
                #endif
                
                /* Notify the Battery service about the pairing */
                BatteryBondingNotify();

            }
            else
            {
                /* Pairing has failed.
                 * 1. If pairing has failed due to repeated attempts, the 
                 *    application should immediately disconnect the link.
                 * 2. The application was bonded and pairing has failed.
                 *    Since the application was using whitelist, so the remote 
                 *    device has same address as our bonded device address.
                 *    The remote connected device may be a genuine one but 
                 *    instead of using old keys, wanted to use new keys. We 
                 *    don't allow bonding again if we are already bonded but we
                 *    will give some time to the connected device to encrypt the
                 *    link using the old keys. if the remote device encrypts the
                 *    link in that time, it's good. Otherwise we will disconnect
                 *    the link.
                 */
                 if(p_event_data->status == sm_status_repeated_attempts)
                 {
                    AppSetState(app_disconnecting);
                    ANCSC_LOG_INFO("pairing has failed.\r\n");
                 }
                 else if(g_app_data.bonded)
                 {
                    g_app_data.bonding_reattempt_tid = TimerCreate(
                                           BONDING_CHANCE_TIMER,
                                           TRUE, 
                                           handleBondingChanceTimerExpiry);
                 }
                 else
                 {
                    /* If the application was not bonded and pairing has failed,
                     * the application will wait for PAIRING_WAIT_TIMER timer
                     * value for remote host to retry pairing.On the timer 
                     * expiry,the application will disconnect the link.
                     * Timer bonding_reattempt_tid has been reused in this case.
                     */
                    if(g_app_data.bonding_reattempt_tid == TIMER_INVALID)
                    {
                        g_app_data.bonding_reattempt_tid = TimerCreate(
                                PAIRING_WAIT_TIMER,
                                TRUE,
                                handleBondingChanceTimerExpiry);                   
                    }
                }
            }
        }
        break;
        default:
        {
            /* Firmware may send this signal after disconnection. So don't 
             * panic but ignore this signal.
             */
        }
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalSmDivApproveInd
 *
 *  DESCRIPTION
 *      This function handles the SM_DIV_APPROVE_IND event
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

static void handleSignalSmDivApproveInd(SM_DIV_APPROVE_IND_T *p_event_data)
{
    /* Handling signal as per current state */
    switch(g_app_data.state)
    {
        /* Request for approval from application comes only when pairing is not
         * in progress
         */
        case app_connected: /* FALLTHROUGH */
        {
            sm_div_verdict approve_div = SM_DIV_REVOKED;
            
            /* Check whether the application is still bonded Then check 
             * whether the diversifier is the same as the one stored by the 
             * application
             */
            #if USE_WHITELIST
            if(g_app_data.bonded)
            {
                if(g_app_data.diversifier == p_event_data->div)
                {
                    approve_div = SM_DIV_APPROVED;
                }
            }
            #else
            if(g_app_data.single_bonded)
            {
                if(g_app_data.diversifier == p_event_data->div)
                {
                    approve_div = SM_DIV_APPROVED;
                }
            }
            #endif

            if(approve_div == SM_DIV_APPROVED)
            {
                M_LOG_INFO("bonding approved.\r\n");
            }
            else
            {
                 M_LOG_WARNING("bonding disapproved.\r\n");   
            }
            SMDivApproval(p_event_data->cid, approve_div);
        }
        break;

        default:
        {
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        }
        break;

    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattDbCfm
 *
 *  DESCRIPTION
 *      This function handles the GATT_ADD_DB_CFM event
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalGattDbCfm(GATT_ADD_DB_CFM_T *p_event_data)
{
    /* Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_init:
        {
            if(p_event_data->result == sys_status_success)
            {
                 /* Database is set up. So start advertising */
                /**original advertise mode: fast adv 30s -> slow adv 60s -> idle*/
                AppSetState(app_fast_advertising);
                
                /** new advertise mode: slow adv mode with no fast adv or idle*/
                /*AppSetState(app_slow_advertising);*/
            }
            else
            {
                /* Don't expect this to happen */
                ReportPanic(app_panic_db_registration);
            }
        }
        break;

       default:
        {
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        }
       break;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalSmKeysInd
 *
 *  DESCRIPTION
 *      This function handles the SM_KEYS_IND event and copies IRK from it
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalSmKeysInd(SM_KEYS_IND_T *p_event_data)
{
    /*Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_connected: /* FALLTHROUGH */
        {
            /* If keys are present, save them */
            if((p_event_data->keys)->keys_present & (1 << SM_KEY_TYPE_DIV))
            {
                ANCSC_LOG_INFO("store the diversifier.\r\n");
                /* Store the diversifier which will be used for accepting/
                 * rejecting the encryption requests.
                 */
                g_app_data.diversifier = (p_event_data->keys)->div;

                /* Write the new diversifier to NVM */
                Nvm_Write(&g_app_data.diversifier,
                          sizeof(g_app_data.diversifier), 
                          NVM_OFFSET_SM_DIV);
            }

            /* Store the IRK, it is used afterwards to validate the identity of
             * connected host
             */
            if((p_event_data->keys)->keys_present & (1 << SM_KEY_TYPE_ID))
            {
                ANCSC_LOG_INFO("store the IRK.\r\n");
                /* If bonded device is resolvable random, store the IRK */
                MemCopy(g_app_data.central_device_irk.irk, 
                            (p_event_data->keys)->irk,
                            MAX_WORDS_IRK);

                /* If bonded device address is resolvable random
                 * then store IRK to NVM 
                 */
                Nvm_Write(g_app_data.central_device_irk.irk, 
                              MAX_WORDS_IRK, 
                              NVM_OFFSET_SM_IRK);
            }
        }
        break;

        default:
        {
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        }
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLMEncryptionChange
 *
 *  DESCRIPTION
 *      This function handles the LM_EV_ENCRYPTION_CHANGE event
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleSignalLMEncryptionChange(
        LM_EV_ENCRYPTION_CHANGE_T *p_event_data)
{
    /*Handling signal as per current state */
    switch(g_app_data.state)
    {
        case app_connected: 
        {
            if(p_event_data->data.status == sys_status_success)
            {
                ANCSC_LOG_DEBUG("app connected, delete the bonding chance timer\r\n");
                /* Delete the bonding chance timer */
                TimerDelete(g_app_data.bonding_reattempt_tid);
                g_app_data.bonding_reattempt_tid = TIMER_INVALID;

                /*Initiate configuring ANCS notification handle */
                if( GetAncsNotificationCCDHandle() != INVALID_ATT_HANDLE)
                {
                    if(!g_app_data.notif_configuring)
                    {
                       ANCSC_LOG_INFO("initiate configuring ANCS notification handle\r\n");
                       g_app_data.notif_configuring = TRUE;
                       appConfigureNotifications(g_app_data.st_ucid,FALSE);
                    }
                }
                else
                {
                    ANCSC_LOG_INFO("start gatt database discovery\r\n");
                   /* Start Gatt Database discovery */
                   DiscoverServices();   
                }
            }
        }
        break;

        default:
        {
            ANCSC_LOG_DEBUG("default: Control should never come here\r\n");
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        }
        break;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      handleGattReadCharValCfm
 *
 *  DESCRIPTION
 *      This function handles GATT_READ_CHAR_VAL_CFM messages
 *      received from the firmware.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/

static void handleGattReadCharValCfm(GATT_READ_CHAR_VAL_CFM_T *p_event_data)
{
    if(p_event_data->result == sys_status_success)
    {
        /* Nothing to do */
    }
    else if((p_event_data->result == GATT_RESULT_INSUFFICIENT_ENCRYPTION) ||
         (p_event_data->result == GATT_RESULT_INSUFFICIENT_AUTHENTICATION))
    {
        /* If we have received an insufficient encryption error code, we will 
         * start a slave security request
         */
        SMRequestSecurityLevel(&g_app_data.con_bd_addr);
    }
    else if(p_event_data->result != GATT_RESULT_TIMEOUT) 
    {
        /* handles all failures except time-out case as that gets handled 
           when we receive disconnect indication */

        /* Something went wrong. We can't recover, so disconnect */
        AppSetState(app_disconnecting);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleGattWriteCharValCfm
 *
 *  DESCRIPTION
 *      This function handles GATT_WRITE_CHAR_VAL_CFM messages
 *      received from the firmware.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
static void handleGattWriteCharValCfm(GATT_WRITE_CHAR_VAL_CFM_T *p_event_data)
{
    uint16 handle = GetTempReadWriteHandle();
    
    if(p_event_data->result == sys_status_success)
    {
        if(GetAncsNotificationCCDHandle() == handle)
        {
          /* Configure for data source */
          appConfigureNotifications(g_app_data.st_ucid,TRUE);
          ANCSC_LOG_INFO("configure for data source(CCCD).\r\n");
        }
        
        if(GetAncsDataSourceCCDHandle() == handle)
        {
          /* Configure for GATT Service changed indication */             
          ConfigureGattIndications();
          ANCSC_LOG_INFO("configure for GATT Service changed indication.\r\n");
        }
        
        /* If service changed notifications are configured, we are done
         * configuring notifications for ANCS and GATT.
         */
        if(GetGattNotificationCCDHandle() == handle)
        {
            /* Set the temporary handle to invalid as we are done */
            SetTempReadWriteHandle(INVALID_ATT_HANDLE);
            
            /* Reset the notif_configuring variable */
            g_app_data.notif_configuring = FALSE;
            
            ANCSC_LOG_INFO("configure notifications for ANCS and GATT.\r\n");
        }
    }
    else if((p_event_data->result == GATT_RESULT_INSUFFICIENT_ENCRYPTION) ||
         (p_event_data->result == GATT_RESULT_INSUFFICIENT_AUTHENTICATION))
    {
        ANCSC_LOG_INFO("start a slave security request.\r\n");
        /* If we have received an insufficient encryption error code, 
         * we will start a slave security request
         */
        g_app_data.notif_configuring = FALSE;
        
        /* Security supported by the remote host */
        if(!g_app_data.pairing_in_progress)
        {
           SMRequestSecurityLevel(&g_app_data.con_bd_addr);
           ANCSC_LOG_INFO("security supported by the remote host.\r\n");
        }
    }
    else if(p_event_data->result != GATT_RESULT_TIMEOUT) 
    {
        /* iOS device might have returned this error.Do not initiate disconnect.
         * Discard this error and continue further.
         */
        if((p_event_data->result == ANCS_ERROR_UNKNOWN_COMMAND) || 
           (p_event_data->result == ANCS_ERROR_INVALID_COMMAND) ||
           (p_event_data->result == ANCS_ERROR_INVALID_PARAMETER))
        {

            ANCSC_LOG_INFO("unable to retrieve Notification Data");
        }
        else
        {
            #if USE_WHITELIST_RESET
            /* Reset and clear the whitelist */
            ANCSC_LOG_WARNING("now reset the whitelist, please try to connect later.\r\n");
            LsResetWhiteList();
            HandlePairingRemoval();
            #else
            /* Something went wrong. We can't recover, so disconnect */
            ANCSC_LOG_WARNING("something went wrong. we can't recover, so disconnect.\r\n");
            AppSetState(app_disconnecting);
            #endif
    
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleNotificationData
 *
 *  DESCRIPTION
 *      This function handles all registered notifications from peer device 
 *
 * RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleNotificationData(GATT_CHAR_VAL_IND_T *p_event_data)
{
      /* Handle the event as per application state. */
    switch(g_app_data.state)
    {
        case app_connected:       /* FALLTHROUGH */
        case app_disconnecting:
        {
            /* Check which service the handle belongs to and call the 
             * corresponding handling function.
             */
            if(DoesHandleBelongsToAncsService(p_event_data->handle))
            {
               AncsHandlerNotifInd(p_event_data);
            }
            
            if(DoesHandleBelongToDiscoveredGattService(p_event_data->handle))
            {
               HandleGattServiceCharValInd(p_event_data);
            }
        }
        break;

        default:
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      appInitExit
 *
 *  DESCRIPTION
 *      This function is called upon exiting the app_init state.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

static void appInitExit(void)
{
    if(g_app_data.bonded == TRUE && 
        (!GattIsAddressResolvableRandom(&g_app_data.bonded_bd_addr)))
    {
        /* If the device is bonded, configure white list with the
         * bonded host address 
         */
        if(LsAddWhiteListDevice(&g_app_data.bonded_bd_addr) != 
                                        ls_err_none)
        {
            ReportPanic(app_panic_add_whitelist);
        }
    }
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      OtaTimerHandler
 *
 *  DESCRIPTION
 *      This function gets called on expiry of OTA wait timer.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void OtaTimerHandler(timer_id tid)
{
    if(tid == g_app_data.ota_wait_tid)
    {
        /* Reset the OTA wait timer */
        TimerDelete(g_app_data.ota_wait_tid);
        g_app_data.ota_wait_tid = TIMER_INVALID;

        /* The remote device does not support anything interesting.
         * Disconnect and wait for some one else to connect.
         */
        AppSetState(app_disconnecting);

    }
}

#ifdef NVM_TYPE_FLASH
/*----------------------------------------------------------------------------*
 *  NAME
 *      WriteApplicationAndServiceDataToNVM
 *
 *  DESCRIPTION
 *      This function writes the application data to NVM. This function should 
 *      be called on getting nvm_status_needs_erase
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern void WriteApplicationAndServiceDataToNVM(void)
{
    uint16 nvm_sanity = 0xffff;
    nvm_sanity = NVM_SANITY_MAGIC;
    
    ANCSC_LOG_DEBUG("writes the application data to NVM\r\n");

    /* Write NVM sanity word to the NVM */
    Nvm_Write(&nvm_sanity, sizeof(nvm_sanity), NVM_OFFSET_SANITY_WORD);

    /* Write Bonded flag to NVM. */
    Nvm_Write((uint16*)&g_app_data.bonded, 
               sizeof(g_app_data.bonded),
               NVM_OFFSET_BONDED_FLAG);

    #if !USE_WHITELIST
    /* Write one word single bonded flag */
    Nvm_Write((uint16*)&g_app_data.single_bonded, sizeof(g_app_data.single_bonded), NVM_OFFSET_SINGLE_BONDED_FLAG);
    #endif
            
    /* Write Bonded address to NVM. */
    Nvm_Write((uint16*)&g_app_data.bonded_bd_addr,
              sizeof(TYPED_BD_ADDR_T),
              NVM_OFFSET_BONDED_ADDR);

    /* Write the diversifier to NVM */
    Nvm_Write(&g_app_data.diversifier,
                sizeof(g_app_data.diversifier),
                NVM_OFFSET_SM_DIV);

    /* Store the IRK to NVM */
    Nvm_Write(g_app_data.central_device_irk.irk,
                MAX_WORDS_IRK,
                NVM_OFFSET_SM_IRK);
    
    Nvm_Write((uint16 *)&g_app_data.remote_gatt_handles_present, 
                  sizeof(g_app_data.remote_gatt_handles_present), 
                  NVM_OFFSET_DISC_HANDLES_PRESENT);

    /* Write GAP service data into NVM. */
    WriteGapServiceDataInNVM();

    /* Write Gatt service data into NVM. */
    WriteGattServiceDataInNvm();

    /* Write Battery service data into NVM. */
    WriteBatteryServiceDataInNvm();
    
    /* Write ANCS service data into NVM */
    WriteDiscAncsServiceHandlesToNVM();
    
    /* Write GATT service data into NVM */
    WriteDiscGattServiceHandlesToNVM();
}
#endif /* NVM_TYPE_FLASH */

/*----------------------------------------------------------------------------*
 *  NAME
 *      ReportPanic
 *
 *  DESCRIPTION
 *      This function raises the panic
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void ReportPanic(app_panic_code panic_code)
{
    /* If we want any debug prints, we can put them here */
#ifdef ENABLE_DEBUG_PANIC
    Panic(panic_code);
#endif
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleShortButtonPress
 *
 *  DESCRIPTION
 *      Function that handles short button press.
 *
 *
 *  RETURNS
 *      nothing
 *
 *----------------------------------------------------------------------------*/
extern void HandleShortButtonPress(void)
{
    ANCSC_LOG_WARNING("(short button)force ble to fast advertising mode(0x%02X).\r\n", g_app_data.state);
    
    #if 0
    /* Handle signal as per current state */
    switch(g_app_data.state)
    {
        case app_idle:
        {
             ANCSC_LOG_INFO("(short button)app_idle state: set disconnect, reset and clear whitelist.\r\n");
             /* Start fast undirected advertisements. */
             AppSetState(app_fast_advertising);
        }
        break;
        default:
            ANCSC_LOG_INFO("(short button)default state(0x%02X): ignore states.\r\n", g_app_data.state);
            /* Ignore in remaining states */
        break;
    }
    #endif
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandlePairingRemoval
 *
 *  DESCRIPTION
 *      Handles the delete pairing request of the pairing removal service.
 *
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void HandlePairingRemoval(void)
{
        /* Remove bonding information*/
        /* The device will no more be bonded */
        g_app_data.bonded = FALSE;

        /* Write bonded status to NVM */
        Nvm_Write((uint16*)&g_app_data.bonded, 
                  sizeof(g_app_data.bonded), 
                  NVM_OFFSET_BONDED_FLAG);
            
        #if !USE_WHITELIST
        g_app_data.single_bonded = FALSE;
        /* Write one word single bonded flag */
        Nvm_Write((uint16*)&g_app_data.single_bonded, sizeof(g_app_data.single_bonded), NVM_OFFSET_SINGLE_BONDED_FLAG);
        #endif
            
        /* Reset the cached handles database */        
        ResetDiscoveredHandlesDatabase();
        
        /* Reset the boolean flag remote_gatt_handles_present to FALSE and store 
         * it in NVM
         */
        g_app_data.remote_gatt_handles_present = FALSE;        
        Nvm_Write((uint16 *)&g_app_data.remote_gatt_handles_present,
                  sizeof(g_app_data.remote_gatt_handles_present),
                  NVM_OFFSET_DISC_HANDLES_PRESENT);


        switch(g_app_data.state)
        {
            case app_connected: /* FALLTHROUGH */
            {
                ANCSC_LOG_INFO("(pairing removal)connected state: set disconnect and reset whitelist.\r\n");
                /* Disconnect with the connected host before triggering 
                 * advertisements again for any host to connect. Application
                 * and services data related to bonding status will get 
                 * updated while exiting disconnecting state
                 */
                AppSetState(app_disconnecting);

                /* Reset and clear the whitelist */
                LsResetWhiteList();
            }
            break;

            case app_fast_advertising: /* FALLTHROUGH */
            case app_slow_advertising:
            {
                ANCSC_LOG_INFO("(pairing removal)advertising state: restart advertisements and reset whitelist.\r\n");
                g_app_data.pairing_remove_button_pressed = TRUE;

                /* Delete the advertising timer */
                TimerDelete(g_app_data.app_tid);
                g_app_data.app_tid = TIMER_INVALID;

                /* Stop advertisements first as it may be making use of white 
                 * list. Once advertisements are stopped, reset the whitelist
                 * and trigger advertisements again for any host to connect
                 */
                GattStopAdverts();
            }
            break;

            case app_disconnecting:
            {
                ANCSC_LOG_INFO("(pairing removal)disconnecting state: reset whitelist.\r\n");
                /* Disconnect procedure on-going, so just reset the whitelist 
                 * and wait for procedure to get completed before triggering 
                 * advertisements again for any host to connect. Application
                 * and services data related to bonding status will get 
                 * updated while exiting disconnecting state
                 */
                LsResetWhiteList();
            }
            break;

            default: /* app_state_init / app_state_idle handling */
            {
                ANCSC_LOG_INFO("(pairing removal)default state(0x%02X): reset whitelist.\r\n", g_app_data.state);
                /* Initialise application data. */
                appDataInit();

                /* Reset and clear the whitelist. */
                LsResetWhiteList();

                /* Start fast undirected advertisements. */
                AppSetState(app_fast_advertising);
            }
            break;

        }
}




/*----------------------------------------------------------------------------*
 *  NAME
 *      AppSetState
 *
 *  DESCRIPTION
 *      This function is used to set the state of the application.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void AppSetState(app_state new_state)
{
    app_state old_state = g_app_data.state;

    /* Check if the new state to be set is not the same as the present state
     * of the application. 
     */
    if (old_state != new_state)
    {
        /* Handle exiting old state */
        switch (old_state)
        {
            case app_init:
            {
                appInitExit();
            }
            break;

            case app_disconnecting:
            {
                /* Common things to do whenever application exits
                 * app_disconnecting state.
                 */
                appDataInit();
            }
            break;

            case app_fast_advertising: /* FALLTHROUGH */ 
            case app_slow_advertising:
            {
                /* Common things to do whenever application exits
                 * APP_*_ADVERTISING state.
                 */
                /* Cancel advertisement timer */
                TimerDelete(g_app_data.app_tid);
                g_app_data.app_tid = TIMER_INVALID;
            }
            break;

            case app_idle:
                /* Nothing to do */
            break;

            case app_connected:
            break;

            default:
                /* Nothing to do */
            break;
        }

        /* Set new state */
        g_app_data.state = new_state;

        /* Handle entering new state */
        switch (new_state)
        {
            case app_fast_advertising:
            {
                /* Start advertising and indicate this to user */
                GattTriggerFastAdverts();
                ANCSC_LOG_INFO("BLE state: Fast advertising\r\n");
            }
            break;

            case app_slow_advertising:
            {
                GattStartAdverts(FALSE);
                ANCSC_LOG_INFO("BLE state: Slow advertising\r\n");
            }
            break;

            case app_idle:
            {
                /* Sound long beep to indicate non connectable mode*/
                ANCSC_LOG_INFO("BLE state: Idle\r\n");
            }
            break;

            case app_connected:
            {
                /* Common things to do whenever application enters
                 * app_state_connected state.
                 */

                /* If the battery level has changed since the last read, update 
                 * the connected Host about it.
                 */
                SendBatteryLevelNotification();
                ANCSC_LOG_INFO("BLE state: Connected\r\n"); 
                
                /* Discovery/Configuration is complete.
                 * Profile specs recommend to update connection parameters 
                 * after service discovery/configuration. Since this 
                 * application is implementing client role and has discovered  
                 * services, so we should update the parameters now.
                 */
                if(new_state == app_connected)
                {
                    /* Connection parameters should be updated only if the new 
                     * state is app_connected.
                     */

                    /* Check if the current parameter value comply with the 
                     * preferred connection intervals.
                     */
                    if(g_app_data.conn_interval < APPLE_MIN_CON_INTERVAL ||
                       g_app_data.conn_interval > APPLE_MAX_CON_INTERVAL
#if APPLE_SLAVE_LATENCY
                       || g_app_data.conn_latency < APPLE_SLAVE_LATENCY
#endif
                      )
                    {
                        /* Delete timer if running */
                        TimerDelete(g_app_data.conn_param_update_tid);

                        /* Set the num of connection update attempts to zero */
                        g_app_data.num_conn_update_req = 0;

                        /* Start timer to trigger Connection Parameter Update 
                         * procedure.
                         */
                        g_app_data.conn_param_update_tid = 
                               TimerCreate(TGAP_CPP_PERIOD,
                                            TRUE, 
                                            handleGapCppTimerExpiry);
                        g_app_data.cpu_timer_value = TGAP_CPP_PERIOD;
                    }
                }
            }
            break;

            case app_disconnecting:
            {
                if(g_app_data.auth_failure)
                {
                    /* Disconnect with an error - Authentication Failure */
                    GattDisconnectReasonReq(g_app_data.st_ucid, 
                          ls_err_authentication);
                }
                else
                {
                    /* Disconnect with the default error */
                    GattDisconnectReq(g_app_data.st_ucid);
                }
                ANCSC_LOG_INFO("BLE state: Disconnected\r\n");
            }
            break;

            default:
                /* Nothing to do */
            break;
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppIsDeviceBonded
 *
 *  DESCRIPTION
 *      This function returns the status whether the connected device is 
 *      bonded or not.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
bool AppIsDeviceBonded(void)
{
    return g_app_data.bonded;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetConnectionID
 *
 *  DESCRIPTION
 *      This function returns the connection identifier for the connection
 *
 *  RETURNS/MODIFIES
 *      Connection identifier.
 *
 
*----------------------------------------------------------------------------*/
extern uint16 GetConnectionID(void)
{
    return g_app_data.st_ucid;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppGetState
 *
 *  DESCRIPTION
 *      This function returns the current state of the device. 
 *
 *  RETURNS/MODIFIES
 *      app_state : current state.
 *
 *---------------------------------------------------------------------------*/
app_state AppGetState(void)
{
    return g_app_data.state;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppPowerOnReset
 *
 *  DESCRIPTION
 *      This function is called just after a power-on reset (including after
 *      a firmware panic).
 *
 *      NOTE: this function should only contain code to be executed after a
 *      power-on reset or panic. Code that should also be executed after an
 *      HCI_RESET should instead be placed in the reset() function.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void AppPowerOnReset(void)
{
    /* Configure the application constants */
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppInit
 *
 *  DESCRIPTION
 *      This function is called after a power-on reset (including after a
 *      firmware panic) or after an HCI Reset has been requested.
 *
 *      NOTE: In the case of a power-on reset, this function is called
 *      after app_power_on_reset.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void AppInit(sleep_state last_sleep_state)
{
    uint16 gatt_db_length = 0;
    uint16 *p_gatt_db = NULL;
    uint8 devName[20] = {0};

    /* Initialise the application timers */
    TimerInit(MAX_APP_TIMERS, (void*)app_timers);

#ifdef ENABLE_UART  
    /* Initialise the UART interface */
    m_uart_init();
    m_devname_init(devName);
    ANCSC_LOG_INFO("system started: %s\r\n", devName);
    m_printf_test();
#endif /* ENABLE_UART */
    
    m_timer_init();
    
    /* Initialise GATT entity */
    GattInit();

    /* Install GATT Client functionality to use Apple Notification 
       Center Service 
     */
    GattInstallClientRole();
    GattInstallServerWrite();

#ifdef NVM_TYPE_EEPROM
    /* Configure the NVM manager to use I2C EEPROM for NVM store */
    NvmConfigureI2cEeprom();
#elif NVM_TYPE_FLASH
    /* Configure the NVM Manager to use SPI flash for NVM store. */
    NvmConfigureSpiFlash();
#endif /* NVM_TYPE_EEPROM */

    Nvm_Disable();

    /*Initialise application hardware */
    InitAncsHardware();

    /* Battery Initialisation on Chip reset */
    BatteryInitChipReset();

    /* Initialise the gap data. Needs to be done before readPersistentStore */
    GapDataInit();

    /* Read persistent storage */
    readPersistentStore();

    /* Tell Security Manager module about the value it needs to initialise it's
     * diversifier to.
     */
    SMInit(g_app_data.diversifier);

    /* Initialise  application data structure */
    appDataInit();

    /* Initialise ANCS application State
       Always the application state should be initialised before the 
       GATT Database functions as GATT_ADD_DB_CFM gets invoked 
       prior to executing the code after GattAddDatabaseReq()
    */
    g_app_data.state = app_init;
    
    g_app_data.pairing_in_progress = FALSE;



    /* Tell GATT about our database. We will get a GATT_ADD_DB_CFM event when
     * this has completed.
     */
    p_gatt_db = GattGetDatabase(&gatt_db_length);

    GattAddDatabaseReq(gatt_db_length, p_gatt_db);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcessSystemEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a system event, such
 *      as a battery low notification, is received by the system.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void AppProcessSystemEvent(sys_event_id id, void *data)
{
    switch(id)
    {
        case sys_event_battery_low:
        {
            /* Battery low event received - notify the connected host. If 
             * not connected, the battery level will get notified when 
             * device gets connected again
             */
            if(g_app_data.state == app_connected)
            {
                BatteryUpdateLevel(g_app_data.st_ucid,TRUE);
            }
        }
        break;

        case sys_event_pio_changed:
        {
            /* Handle PIO events */
            HandlePIOChangedEvent(((pio_changed_data*)data)->pio_cause);
        }
        break;

        default:
        {
            break;
        }
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcessLmEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a LM-specific event is
 *      received by the system.
 *
 *  RETURNS
 *      bool.
 *
 *----------------------------------------------------------------------------*/
bool AppProcessLmEvent(lm_event_code event_code, LM_EVENT_T *event_data)
{
    switch (event_code)
    {
        case GATT_ADD_DB_CFM:
            ANCSC_LOG_INFO("GATT_ADD_DB_CFM - database registration completed.\r\n");
            handleSignalGattDbCfm((GATT_ADD_DB_CFM_T*)event_data);
        break;

        case LM_EV_CONNECTION_COMPLETE:
            ANCSC_LOG_DEBUG("LM_EV_CONNECTION_COMPLETE\r\n");
            handleSignalLmEvConnectionComplete(
                    (LM_EV_CONNECTION_COMPLETE_T*)event_data);
        break;

        case GATT_CONNECT_CFM:
            ANCSC_LOG_DEBUG("GATT_CONNECT_CFM\r\n");
            handleSignalGattConnectCfm((GATT_CONNECT_CFM_T *)event_data);
        break;


        /* Discovered primary service information for the requested
         * primary service UUID
         */
        case GATT_DISC_PRIM_SERV_BY_UUID_IND:
        {
            ANCSC_LOG_DEBUG("GATT_DISC_PRIM_SERV_BY_UUID_IND\r\n");
            HandleGenericDiscoverPrimaryServiceInd(
                (GATT_DISC_PRIM_SERV_BY_UUID_IND_T*)event_data);
        }
        break;

        /* Discover primary service by UUID confirmation */
        case GATT_DISC_PRIM_SERV_BY_UUID_CFM:
        {
            ANCSC_LOG_DEBUG("GATT_DISC_PRIM_SERV_BY_UUID_CFM\r\n");
            HandleGenericServiceDiscoverPrimaryServiceByUuidCfm(
            (GATT_DISC_PRIM_SERV_BY_UUID_CFM_T*)event_data, g_app_data.st_ucid);
        }
        break;

        case GATT_CHAR_DECL_INFO_IND:
            ANCSC_LOG_DEBUG("GATT_CHAR_DECL_INFO_IND\r\n");
               HandleGenericGattServiceCharacteristicDeclarationInfoInd(
                (GATT_CHAR_DECL_INFO_IND_T*)event_data);
        break;

        case GATT_DISC_SERVICE_CHAR_CFM:
            ANCSC_LOG_DEBUG("GATT_DISC_SERVICE_CHAR_CFM\r\n");
                    HandleGenericGattDiscoverServiceCharacteristicCfm(
                (GATT_DISC_SERVICE_CHAR_CFM_T*)event_data, g_app_data.st_ucid);
        break;

        case GATT_CHAR_DESC_INFO_IND:
            ANCSC_LOG_DEBUG("GATT_CHAR_DESC_INFO_IND\r\n");
            HandleGenericGattCharacteristicDescriptorInfoInd(
                                (GATT_CHAR_DESC_INFO_IND_T *)event_data);
        break;

        case GATT_DISC_ALL_CHAR_DESC_CFM:
            ANCSC_LOG_DEBUG("GATT_DISC_ALL_CHAR_DESC_CFM\r\n");
            HandleGenericGattCharacteristicDescriptorCfm(
                    (GATT_DISC_ALL_CHAR_DESC_CFM_T *) event_data, 
                    g_app_data.st_ucid);
        break;

        case GATT_READ_CHAR_VAL_CFM:
            ANCSC_LOG_DEBUG("GATT_READ_CHAR_VAL_CFM\r\n");
            handleGattReadCharValCfm((GATT_READ_CHAR_VAL_CFM_T *)event_data);
        break;

        case GATT_WRITE_CHAR_VAL_CFM:
            ANCSC_LOG_DEBUG("GATT_WRITE_CHAR_VAL_CFM\r\n");
            handleGattWriteCharValCfm((GATT_WRITE_CHAR_VAL_CFM_T *)event_data);
        break;

        case LM_EV_ENCRYPTION_CHANGE:
            ANCSC_LOG_INFO("LM_EV_ENCRYPTION_CHANGE - link encryption changed.\r\n");
            handleSignalLMEncryptionChange(
                    (LM_EV_ENCRYPTION_CHANGE_T *)event_data);
        break;

        case LS_CONNECTION_PARAM_UPDATE_CFM:
            ANCSC_LOG_DEBUG("LS_CONNECTION_PARAM_UPDATE_CFM\r\n");
            handleSignalLsConnUpdateSignalCfm(
                    (LS_CONNECTION_PARAM_UPDATE_CFM_T *)event_data);
        break;

        case LM_EV_CONNECTION_UPDATE:
            ANCSC_LOG_DEBUG("LM_EV_CONNECTION_UPDATE\r\n");
            /* This event is sent by the controller on connection parameter 
             * update. 
             */
            handleSignalLmConnectionUpdate(
                    (LM_EV_CONNECTION_UPDATE_T*)event_data);
        break;

        case LS_CONNECTION_PARAM_UPDATE_IND:
            ANCSC_LOG_DEBUG("LS_CONNECTION_PARAM_UPDATE_IND\r\n");
            handleSignalLsConnParamUpdateInd(
                    (LS_CONNECTION_PARAM_UPDATE_IND_T *)event_data);
        break;

        case SM_DIV_APPROVE_IND:
            ANCSC_LOG_INFO("SM_DIV_APPROVE_IND - ANCS device re-encrypts.\r\n");
            handleSignalSmDivApproveInd((SM_DIV_APPROVE_IND_T *)event_data);
        break;

        case SM_KEYS_IND:
            ANCSC_LOG_INFO("SM_KEYS_IND - bonding procedure completed.\r\n");
            handleSignalSmKeysInd((SM_KEYS_IND_T *)event_data);
        break;

        case SM_PAIRING_AUTH_IND:
            ANCSC_LOG_INFO("SM_PAIRING_AUTH_IND - ANCS device initiates pairing.\r\n");
            /* Authorize or Reject the pairing request */
            handleSignalSmPairingAuthInd((SM_PAIRING_AUTH_IND_T*)event_data);
        break;

        case SM_SIMPLE_PAIRING_COMPLETE_IND:
            ANCSC_LOG_INFO("SM_SIMPLE_PAIRING_COMPLETE_IND -  pairing has completed successfully.\r\n");
            handleSignalSmSimplePairingCompleteInd(
                    (SM_SIMPLE_PAIRING_COMPLETE_IND_T *)event_data);
        break;

        case GATT_DISCONNECT_IND:
            ANCSC_LOG_DEBUG("GATT_DISCONNECT_IND\r\n");
            /* Disconnect procedure triggered by remote host or due to 
             * link loss is considered complete on reception of 
             * LM_EV_DISCONNECT_COMPLETE event. So, it gets handled on 
             * reception of LM_EV_DISCONNECT_COMPLETE event.
             */
        break;

        case GATT_DISCONNECT_CFM:
            ANCSC_LOG_DEBUG("GATT_DISCONNECT_CFM\r\n");
            /* Confirmation for the completion of GattDisconnectReq()
             * procedure is ignored as the procedure is considered complete 
             * on reception of LM_EV_DISCONNECT_COMPLETE event. So, it gets 
             * handled on reception of LM_EV_DISCONNECT_COMPLETE event.
             */
        break;

        case LM_EV_DISCONNECT_COMPLETE:
        {
            ANCSC_LOG_DEBUG("LM_EV_DISCONNECT_COMPLETE\r\n");
            /* Disconnect procedures either triggered by application or remote
             * host or link loss case are considered completed on reception 
             * of LM_EV_DISCONNECT_COMPLETE event
             */
             handleSignalLmDisconnectComplete(
                     ((LM_EV_DISCONNECT_COMPLETE_T *)event_data));
        }
        break;

        case GATT_CANCEL_CONNECT_CFM:
            ANCSC_LOG_DEBUG("GATT_CANCEL_CONNECT_CFM\r\n");
            handleSignalGattCancelConnectCfm(
                    (GATT_CANCEL_CONNECT_CFM_T*)event_data);
        break;

        case GATT_NOT_CHAR_VAL_IND:
            ANCSC_LOG_DEBUG("GATT_NOT_CHAR_VAL_IN\r\nD");
            /* A notification has been received */
            /* Depending on the handle , it will get handled in corresponding
             * function.
           */
           handleNotificationData((GATT_CHAR_VAL_IND_T *)event_data);
        break;

        case LM_EV_NUMBER_COMPLETED_PACKETS: /* FALLTHROUGH */ 
        case GATT_CHAR_VAL_NOT_CFM: /* FALLTHROUGH */ 
            ANCSC_LOG_DEBUG("LM_EV_NUMBER_COMPLETED_PACKETS or GATT_CHAR_VAL_NOT_CFM(%d)\r\n", event_code);
        break;

        case GATT_ACCESS_IND: 
            ANCSC_LOG_DEBUG("GATT_ACCESS_IND - ANCS application tries to access an ATT characteristic managed.\r\n");
            /* Indicates that an attribute controlled directly by the
             * application (ATT_ATTR_IRQ attribute flag is set) is being 
             * read from or written to.
             */
            handleSignalGattAccessInd((GATT_ACCESS_IND_T*)event_data);
            break;

        default:
        {
            ANCSC_LOG_DEBUG("default(%d\r\n)", event_code);
            /* Control should never come here */
            break;
        }
    }
    return TRUE;
}



/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleConnectReq
 *
 *  DESCRIPTION
 *      This function is used to initiate a connection with the ANCS server.
 *
 *  RETURNS/MODIFIES
 *      none
 *
 *---------------------------------------------------------------------------*/
void HandleConnectReq(void)
{
    /* Start advertising */
    if(g_app_data.state == app_idle)
    {
        AppSetState(app_fast_advertising);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleDisconnectReq
 *
 *  DESCRIPTION
 *      This function is used to initiates a disconnect with the ANC server.
 *
 *  RETURNS/MODIFIES
 *      none
 *
 *---------------------------------------------------------------------------*/
void HandleDisconnectReq(void)
{
   if(app_connected == AppGetState())
    {
        /* Initiate a disconnect */
        AppSetState(app_disconnecting);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      SetTempReadWriteHandle
 *
 *  DESCRIPTION
 *      This function sets the temporary handle to the parameter value.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void SetTempReadWriteHandle(uint16 handle)
{
    g_app_data.temp_handle = handle;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetTempReadWriteHandle
 *
 *  DESCRIPTION
 *      This function returns the temporary handle which the application 
 *      maintains while performing reads and writes.
 *
 *  RETURNS
 *      Handle value
 *
 *----------------------------------------------------------------------------*/
extern uint16 GetTempReadWriteHandle(void)
{
    return(g_app_data.temp_handle);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      DiscoverServices
 *
 *  DESCRIPTION
 *      This function starts a timer, expiry of which triggers GATT database 
 *      discovery.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void DiscoverServices(void)
{
    /* Create a timer of value DISCOVERY_START_TIMER */
    TimerDelete(g_app_data.app_tid);
    g_app_data.app_tid =  TIMER_INVALID;
    
    g_app_data.app_tid = TimerCreate(DISCOVERY_START_TIMER, TRUE,
                                       handleDiscoveryTimerExpiry);
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      AppHandleProcedureCompletionEvent
 *
 *  DESCRIPTION
 *      Callback function for the application procedures.
 *      Whenever a procedure gets completed, it calls this function with an
 *      appropriate event code.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void AppHandleProcedureCompletionEvent(app_procedure_code app_proc_code,
                                              APP_PROC_EVENT_T *p_event_data)
{
    switch(app_proc_code)
    {
        /* Discovery procedure completion. */
        case APP_DISCOVERY_CFM:
        {
            /* Service discovery is complete. Set the boolean flag 
             * remote_gatt_handles_present to TRUE and store it in NVM
             */
            g_app_data.remote_gatt_handles_present = TRUE;
            Nvm_Write((uint16 *)&g_app_data.remote_gatt_handles_present, 
                            sizeof(g_app_data.remote_gatt_handles_present), 
                            NVM_OFFSET_DISC_HANDLES_PRESENT);
            
            
            /* Initiate configuring ANCS Notification Handle */
            if( GetAncsNotificationCCDHandle() != INVALID_ATT_HANDLE )
            {
                if(!g_app_data.notif_configuring)
                {
                     g_app_data.notif_configuring = TRUE;
                     appConfigureNotifications(g_app_data.st_ucid,FALSE);
                 }
            }
            else
            {
                /* Reset the OTA wait timer */
                TimerDelete(g_app_data.ota_wait_tid);
                g_app_data.ota_wait_tid = TIMER_INVALID;

                /* Start a timer to allow the device to update the application
                 * software on it over the air (OTA) if required.
                 * If no OTA update is there for OTA_WAIT_TIME,disconnect and 
                 * wait for some one else to connect.
                 */
                g_app_data.ota_wait_tid = TimerCreate(OTA_WAIT_TIME, TRUE,
                                                      OtaTimerHandler);
            }
        }
        break;

        default:
        break;
    }
}