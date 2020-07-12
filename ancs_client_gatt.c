/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      ancs_client_gatt.c
 *
 *  DESCRIPTION
 *      Implementation of the ANCS Client GATT-related routines
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <ls_app_if.h>
#include <gap_app_if.h>
#include <gatt.h>
#include <mem.h>
#include <timer.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "ancs_client.h"
#include "discovered_ancs_service.h"
#include "app_gatt_db.h"
#include "gap_uuids.h"
#include "battery_service.h"
#include "gap_service.h"
#include "dev_info_service.h"
#include "gatt_service.h"
#include "csr_ota_service.h"
#include "serial_service.h"
#include "user_config.h"
#if USE_ADV_DATA
#include "appearance.h"
#include "csr_ota_uuids.h"
#endif

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

 /* Length of Tx Power prefixed with Tx power AD type */
#define TX_POWER_VALUE_LENGTH                    (2)

/* Acceptable shortened device name length that can be sent in advertisement
 * data 
 */
#define SHORTENED_DEV_NAME_LEN                   (8)

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* This function sets the advertising paramters */
static void gattSetAdvertParams(gap_mode_connect connect_mode);

/* This function gets called on advertising timer expiry.*/
static void gattAdvertTimerHandler(timer_id tid);

/* This function adds device name into advertisement data */
static void addDeviceNameToAdvData(uint16 adv_data_len, uint16 scan_data_len);


/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      addDeviceNameToAdvData
 *
 *  DESCRIPTION
 *      This function is used to add device name to advertisement or scan 
 *      response data. It follows below steps:
 *      a. Try to add complete device name to the advertisement packet
 *      b. Try to add complete device name to the scan response packet
 *      c. Try to add shortened device name to the advertisement packet
 *      d. Try to add shortened (max possible) device name to the scan 
 *         response packet
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void addDeviceNameToAdvData(uint16 adv_data_len, uint16 scan_data_len)
{
    uint8 *p_device_name = NULL;
    uint16 device_name_adtype_len = 0;

    /* Read device name along with AD Type and its length */
    p_device_name = GapGetNameAndLength(&device_name_adtype_len);

    /* Add complete device name to Advertisement data */
    p_device_name[0] = AD_TYPE_LOCAL_NAME_COMPLETE;

    /* Increment device name length by one to account for length field
     * which will be added by the GAP layer. 
     *
     * Check if Complete Device Name can fit in remaining advertisement 
     * data space 
     */
    if((device_name_adtype_len + 1) <= (MAX_ADV_DATA_LEN - adv_data_len))
    {
        /* Add Complete Device Name to Advertisement Data */
        if (LsStoreAdvScanData(device_name_adtype_len , p_device_name, 
                      ad_src_advertise) != ls_err_none)
        {
            #if USE_PANIC_PRINT
            ReportPanic(__FILE__, __func__, __LINE__, app_panic_set_advert_data);
            #endif
        }
    }
    /* Check if Complete Device Name can fit in Scan response message */
    else if((device_name_adtype_len + 1) <= (MAX_ADV_DATA_LEN - scan_data_len)) 
    {
        /* Add Complete Device Name to Scan Response Data */
        if (LsStoreAdvScanData(device_name_adtype_len , p_device_name, 
                      ad_src_scan_rsp) != ls_err_none)
        {
            #if USE_PANIC_PRINT
            ReportPanic(__FILE__, __func__, __LINE__, app_panic_set_scan_rsp_data);
            #endif
        }
    }
    /* Check if Shortened Device Name can fit in remaining advertisement 
     * data space 
     */
    else if((MAX_ADV_DATA_LEN - adv_data_len) >=
            (SHORTENED_DEV_NAME_LEN + 2)) /* Added 2 for Length and AD type 
                                           * added by GAP layer
                                           */
    {
        /* Add shortened device name to Advertisement data */
        p_device_name[0] = AD_TYPE_LOCAL_NAME_SHORT;

        /* Add 1 octet to the length for AD type */
       if (LsStoreAdvScanData((SHORTENED_DEV_NAME_LEN + 1), p_device_name, 
                      ad_src_advertise) != ls_err_none)
        {
            #if USE_PANIC_PRINT
            ReportPanic(__FILE__, __func__, __LINE__, app_panic_set_advert_data);
            #endif
        }
    }
    else /* Add device name to remaining Scan response data space */
    {
        /* Add as much as can be stored in Scan Response data */
        p_device_name[0] = AD_TYPE_LOCAL_NAME_SHORT;

       if (LsStoreAdvScanData(MAX_ADV_DATA_LEN - scan_data_len, 
                                    p_device_name, 
                                    ad_src_scan_rsp) != ls_err_none)
        {
            #if USE_PANIC_PRINT
            ReportPanic(__FILE__, __func__, __LINE__, app_panic_set_scan_rsp_data);
            #endif
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      gattSetAdvertParams
 *
 *  DESCRIPTION
 *      This function is used to set advertisement parameters
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void gattSetAdvertParams(bool fast_connection)
{
    #if USE_ADV_DATA
    /* Add 128-bit UUID for supported main service  */
    uint8 advert_data[MAX_ADV_DATA_LEN] = {
                                            AD_TYPE_SERVICE_UUID_128BIT_LIST,
                                            UUID_SERIAL_SERVICE_16,
                                            UUID_SERIAL_SERVICE_15,
                                            UUID_SERIAL_SERVICE_14,
                                            UUID_SERIAL_SERVICE_13,
                                            UUID_SERIAL_SERVICE_12,
                                            UUID_SERIAL_SERVICE_11,
                                            UUID_SERIAL_SERVICE_10,
                                            UUID_SERIAL_SERVICE_9,
                                            UUID_SERIAL_SERVICE_8,
                                            UUID_SERIAL_SERVICE_7,
                                            UUID_SERIAL_SERVICE_6,
                                            UUID_SERIAL_SERVICE_5,
                                            UUID_SERIAL_SERVICE_4,
                                            UUID_SERIAL_SERVICE_3,
                                            UUID_SERIAL_SERVICE_2,
                                            UUID_SERIAL_SERVICE_1};
    uint16 length = 17;
    #endif
    uint32 adv_interval_min = RP_ADVERTISING_INTERVAL_MIN;
    uint32 adv_interval_max = RP_ADVERTISING_INTERVAL_MAX;

    int8 tx_power_level = 0xff; /* Signed value */

    /* Tx power level value prefixed with 'Tx Power' AD Type */
    uint8 device_tx_power[TX_POWER_VALUE_LENGTH] = {
                AD_TYPE_TX_POWER
                };

    #if USE_ADV_DATA
    uint8 device_appearance[ATTR_LEN_DEVICE_APPEARANCE + 1] = {
                AD_TYPE_APPEARANCE,
                LE8_L(APPEARANCE_RSC_SENSOR_VALUE),
                LE8_H(APPEARANCE_RSC_SENSOR_VALUE)
                };   /*jim add*/
    #endif
    
    /* A variable to keep track of the data added to AdvData. The limit is 
     * MAX_ADV_DATA_LEN. GAP layer will add AD Flags to AdvData which 
     * is 3 bytes. Refer BT Spec 4.0, Vol 3, Part C, Sec 11.1.3.
     */
    uint16 length_added_to_adv = 3;

    if(fast_connection)
    {
        adv_interval_min = FC_ADVERTISING_INTERVAL_MIN;
        adv_interval_max = FC_ADVERTISING_INTERVAL_MAX;
    }

    /* In a Public Environment like Gym, there is a requirement for HR Sensor 
     * to communicate with fitness machines even without having a bond. So, 
     * for HR Sensor application we will not enforce bonding by calling SM 
     * Slave Security Request. Though, we will set HR Sensor's security 
     * capabilities to 'gap_mode_bond_yes' so that we accept bonding if the 
     * collector (like Watch or Cell Phone) would like to bond.
     */

    if((GapSetMode(gap_role_peripheral, gap_mode_discover_general,
                        gap_mode_connect_undirected, 
                        gap_mode_bond_yes,
                        gap_mode_security_unauthenticate) != ls_err_none) ||
       (GapSetAdvInterval(adv_interval_min, adv_interval_max) 
                        != ls_err_none))
    {
        #if USE_PANIC_PRINT
        ReportPanic(__FILE__, __func__, __LINE__, app_panic_set_advert_params);
        #endif
    }


    /* Reset existing advertising data */
    if(LsStoreAdvScanData(0, NULL, ad_src_advertise) != ls_err_none)
    {
        #if USE_PANIC_PRINT
        ReportPanic(__FILE__, __func__, __LINE__, app_panic_set_advert_data);
        #endif
    }

    #if USE_ADV_DATA
    /* One added for Length field, which will be added to Adv Data by GAP 
     * layer 
     */
    length_added_to_adv += (length + 1);

    if (LsStoreAdvScanData(length, advert_data, 
                        ad_src_advertise) != ls_err_none)
    {
        #if USE_PANIC_PRINT
        ReportPanic(__FILE__, __func__, __LINE__, app_panic_set_advert_data);
        #endif
    }

    /* One added for Length field, which will be added to Adv Data by GAP 
     * layer 
     */
    length_added_to_adv += (sizeof(device_appearance) + 1);

    /* Add device appearance to the advertisements */
    if (LsStoreAdvScanData(ATTR_LEN_DEVICE_APPEARANCE + 1, 
        device_appearance, ad_src_advertise) != ls_err_none)
    {
        #if USE_PANIC_PRINT
        ReportPanic(__FILE__, __func__, __LINE__, app_panic_set_advert_data);
        #endif
    }
    #else
    /* Reset existing scan response data */
    if(LsStoreAdvScanData(0, NULL, ad_src_scan_rsp) != ls_err_none)
    {
        #if USE_PANIC_PRINT
        ReportPanic(__FILE__, __func__, __LINE__, app_panic_set_scan_rsp_data);
        #endif
    }
    #endif

    /* Setup ADVERTISEMENT DATA */
   
    /* Read tx power of the chip */
    if(LsReadTransmitPowerLevel(&tx_power_level) != ls_err_none)
    {
        #if USE_PANIC_PRINT
        /* Reading tx power failed */
        ReportPanic(__FILE__, __func__, __LINE__, app_panic_read_tx_pwr_level);
        #endif
    }

    /* Add the read tx power level to device_tx_power 
      * Tx power level value is of 1 byte 
      */
    device_tx_power[TX_POWER_VALUE_LENGTH - 1] = (uint8 )tx_power_level;

    /* One added for Length field, which will be added to Adv Data by GAP 
     * layer 
     */
    length_added_to_adv += (TX_POWER_VALUE_LENGTH + 1);

    /* Add tx power value of device to the advertising data */
    if (LsStoreAdvScanData(TX_POWER_VALUE_LENGTH, device_tx_power, 
                          ad_src_advertise) != ls_err_none)
    {
        #if USE_PANIC_PRINT
        ReportPanic(__FILE__, __func__, __LINE__, app_panic_set_advert_data);
        #endif
    }

    /* Add device name into advertisement data */
    addDeviceNameToAdvData(length_added_to_adv, 0);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      gattAdvertTimerHandler
 *
 *  DESCRIPTION
 *      This function is used to stop on-going advertisements at the expiry of 
 *      DISCOVERABLE or RECONNECTION timer.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void gattAdvertTimerHandler(timer_id tid)
{
    /* Based upon the timer id, stop on-going advertisements */
    if(g_app_data.app_tid == tid)
    {
        if(g_app_data.state == app_fast_advertising)
        {
            /* Advertisement timer for reduced power connections */
            g_app_data.advert_timer_value = 
                                SLOW_CONNECTION_ADVERT_TIMEOUT_VALUE;
        }

            /* Stop on-going advertisements */
            GattStopAdverts();

    } /* Else ignore timer expiry, could be because of 
       * some race condition 
       */
    g_app_data.app_tid = TIMER_INVALID;
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles read operation on attributes (as received in 
 *      GATT_ACCESS_IND message) maintained by the application and respond 
 *      with the GATT_ACCESS_RSP message.
 *
 *  RETURNS
 *      Nothing
 *
 *---------------------------------------------------------------------------*/
void HandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    /* For the received attribute handle, check all the services that support 
     * attribute 'Read' operation handled by application.
     */

    if(GapCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to GAP service */
        GapHandleAccessRead(p_ind);
    }
    else if(GattCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to Gatt service */
        GattHandleAccessRead(p_ind);
    }
    else if(OtaCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to OTA service */
        if(GetAncsNotificationCCDHandle()== INVALID_ATT_HANDLE)
        {
            /* Reset the OTA wait timer */
            TimerDelete(g_app_data.ota_wait_tid);
            g_app_data.ota_wait_tid = TIMER_INVALID;

            /* Restart the OTA wait timer to allow the device to update the
             * application software on it over the air (OTA) if required.
             * If no OTA update is there for OTA_WAIT_TIME,disconnect and wait
             * for some one else to connect.
             */
            g_app_data.ota_wait_tid = TimerCreate(OTA_WAIT_TIME, TRUE,
                                              OtaTimerHandler);
        }
        OtaHandleAccessRead(p_ind);
    }
    else if(BatteryCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to Battery service */
        BatteryHandleAccessRead(p_ind);
    }
    else if(DeviceInfoCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to the Device Information Service */
        DeviceInfoHandleAccessRead(p_ind);
    }
    else
    {
        /* Application doesn't support 'Read' operation on received 
         * attribute handle, hence return 'gatt_status_read_not_permitted'
         * status
         */
        GattAccessRsp(p_ind->cid, p_ind->handle, 
                      gatt_status_read_not_permitted,
                      0, NULL);
    }

}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleAccessWrite
 *
 *  DESCRIPTION
 *      This function handles Write operation on attributes (as received in 
 *      GATT_ACCESS_IND message) maintained by the application.
 *
 *  RETURNS
 *      Nothing
 *
 *---------------------------------------------------------------------------*/
void HandleAccessWrite(GATT_ACCESS_IND_T *p_ind)
{
    /* For the received attribute handle, check all the services that support 
     * attribute 'Write' operation handled by application.
     */

    if(GapCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to GAP service */
        GapHandleAccessWrite(p_ind);
    }
    else if(GattCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to Gatt service */
        GattHandleAccessWrite(p_ind);
    }
    else if(OtaCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to OTA service */
       if( GetAncsNotificationCCDHandle()== INVALID_ATT_HANDLE )
       {
            /* Reset the OTA wait timer */
            TimerDelete(g_app_data.ota_wait_tid);
            g_app_data.ota_wait_tid = TIMER_INVALID;

            /* Restart the OTA wait timer to allow the device to update the
             * application software on it over the air (OTA) if required.
             * If no OTA update is there for OTA_WAIT_TIME,disconnect and wait
             * for some one else to connect.
             */
            g_app_data.ota_wait_tid = TimerCreate(OTA_WAIT_TIME, TRUE,
                                              OtaTimerHandler);
        }
        OtaHandleAccessWrite(p_ind);
    }
    else if(BatteryCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to Battery service */
        BatteryHandleAccessWrite(p_ind);
    }
    else if(SerialCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to Serial service */
        SerialHandleAccessWrite(p_ind);
    }
    else
    {
        /* Application doesn't support 'Write' operation on received 
         * attribute handle, hence return 'gatt_status_write_not_permitted'
         * status
         */
        GattAccessRsp(p_ind->cid, p_ind->handle, 
                      gatt_status_write_not_permitted,
                      0, NULL);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattStartAdverts
 *
 *  DESCRIPTION
 *      This function is used to start undirected advertisements and moves to
 *      ADVERTISING state.
 *
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void GattStartAdverts(bool fast_connection)
{
    uint16 connect_flags = L2CAP_CONNECTION_SLAVE_UNDIRECTED | 
                          L2CAP_OWN_ADDR_TYPE_PUBLIC | 
                          L2CAP_PEER_ADDR_TYPE_PUBLIC;

    /* Set UCID to INVALID_UCID */
    g_app_data.st_ucid = GATT_INVALID_UCID;

    /* Set advertisement parameters */
    gattSetAdvertParams(fast_connection);

    /* If white list is enabled, set the controller's advertising filter policy 
     * to "process scan and connection requests only from devices in the
     * White List"
     */
    #if USE_WHITELIST_ADV
    if(g_app_data.bonded == TRUE && 
        (!GattIsAddressResolvableRandom(&g_app_data.bonded_bd_addr)))
    {
        connect_flags = L2CAP_CONNECTION_SLAVE_WHITELIST |
                       L2CAP_OWN_ADDR_TYPE_PUBLIC | 
                       L2CAP_PEER_ADDR_TYPE_PUBLIC;
    }
    #endif

    /* Start GATT connection in Slave role */
    GattConnectReq(NULL, connect_flags);

    /* Start advertisement timer */
    if(g_app_data.advert_timer_value)
    {
        TimerDelete(g_app_data.app_tid);

        /* Start advertisement timer  */
        g_app_data.app_tid = TimerCreate(g_app_data.advert_timer_value, TRUE, 
                                        gattAdvertTimerHandler);
    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattStopAdverts
 *
 *  DESCRIPTION
 *      This function is used to stop on-going advertisements.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void GattStopAdverts(void)
{
    GattCancelConnectReq();
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattIsAddressResolvableRandom
 *
 *  DESCRIPTION
 *      This function checks if the address is resolvable random or not.
 *
 *  RETURNS
 *      Boolean - True if address is Resolvable random address
 *                False otherwise
 *
 *----------------------------------------------------------------------------*/
bool GattIsAddressResolvableRandom(TYPED_BD_ADDR_T *addr)
{
    if ((addr->type != L2CA_RANDOM_ADDR_TYPE) || 
        (addr->addr.nap & BD_ADDR_NAP_RANDOM_TYPE_MASK)
                                      != BD_ADDR_NAP_RANDOM_TYPE_RESOLVABLE)
    {
        /* This isn't a resolvable private address... */
        return FALSE;
    }
    return TRUE;
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      GattTriggerFastAdverts
 *
 *  DESCRIPTION
 *      This function is used to start advertisements for fast connection 
 *      parameters
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
void GattTriggerFastAdverts(void)
{
    /* Start the fast advertising timer */
    g_app_data.advert_timer_value = FAST_CONNECTION_ADVERT_TIMEOUT_VALUE;

    /* Trigger fast connections */
    GattStartAdverts(TRUE);
}
