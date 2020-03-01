/******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      discovered_gatt_service.c
 *
 *  DESCRIPTION
 *      This file keeps information related to the discovered GATT
 *      service 
 *
 *
 ******************************************************************************/
/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <types.h>
#include <gatt.h>
#include <bt_event_types.h>
#include <buf_utils.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/

#include "app_gatt.h"
#include "discovered_gatt_service.h"
#include "gatt_uuids.h"
#include "nvm_access.h"
#include "ancs_client.h"
/*============================================================================*
 *  Public Data
 *============================================================================*/
/* GATT service changed characteristic */
CHAR_DATA_T g_disc_gatt_char = 
{
    INVALID_ATT_HANDLE,                 /* start_handle */
    INVALID_ATT_HANDLE,                 /* end_handle */
    GATT_UUID16,
    {
        UUID_SERVICE_CHANGED        /* UUID for the Service Change
                                         * characteristic 
                                         */
    },
    TRUE,                              /* Has_ccd */
    INVALID_ATT_HANDLE,                 /* ccd_handle*/
    0 /*value*/
};



/* GATT Service */
SERVICE_DATA_T g_disc_gatt_service = 
{
    INVALID_ATT_HANDLE,         /* start handle */
    INVALID_ATT_HANDLE,         /* end_handle */
    GATT_UUID16,
    {
        UUID_GATT_SERVICE               /* GATT Service UUID */
    },
    1,                          /* Number of characteristics  */
    0,                          /* characteristic index */
    &g_disc_gatt_char,
    0,                          /* NVM_OFFSET*/
    ReadDiscoveredGattServiceHandlesFromNVM,
    WriteDiscGattServiceHandlesToNVM,
};


/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/


/*----------------------------------------------------------------------------*
 *  NAME
 *      WriteDiscGattServiceHandlesToNVM
 *
 *  DESCRIPTION
 *      This function stores the discovered service handles in NVM
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void WriteDiscGattServiceHandlesToNVM(void)
{

    uint16 offset= g_disc_gatt_service.nvm_offset;

    /* Store all the handles in NVM */

    /* Write the service start handle */
    Nvm_Write((uint16*)&g_disc_gatt_service.start_handle,
                sizeof(g_disc_gatt_service.start_handle),
                offset + NVM_DISC_GATT_SERVICE_START_HANDLE_OFFSET);

    /* Write the service end handle  */
    Nvm_Write((uint16*)&g_disc_gatt_service.end_handle,
                sizeof(g_disc_gatt_service.end_handle),
                offset + NVM_DISC_GATT_SERVICE_END_HANDLE_OFFSET);

    /* Write the service changed characteristic handle */
    Nvm_Write((uint16*)&g_disc_gatt_char.start_handle,
                sizeof(g_disc_gatt_char.start_handle),
                offset + NVM_DISC_SERVICE_CHANGED_HANDLE_OFFSET);

    /* Write the service changed Client Characteristic Configuration descriptor
     * handle.
     */
    Nvm_Write((uint16*)&g_disc_gatt_char.ccd_handle,
                sizeof(g_disc_gatt_char.ccd_handle),
                offset + NVM_DISC_SERVICE_CHANGED_CCD_HANDLE_OFFSET);

}


/*----------------------------------------------------------------------------*
 *  NAME
 *      ReadDiscoveredGattServiceHandlesFromNVM
 *
 *  DESCRIPTION
 *      This function reads the GATT service handles from NVM
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void ReadDiscoveredGattServiceHandlesFromNVM(uint16 *p_offset,
                                                    bool handles_present)
{
    /* The application stores the discoverd GATT service 
     * handles in NVM 
     */
    g_disc_gatt_service.nvm_offset = *p_offset;

    if(handles_present)
    {
        /* Read the service start handle from NVM */
        Nvm_Read((uint16*)&g_disc_gatt_service.start_handle,
                sizeof(g_disc_gatt_service.start_handle),
                *p_offset + NVM_DISC_GATT_SERVICE_START_HANDLE_OFFSET);

        /* Read the service end handle from NVM */
        Nvm_Read((uint16*)&g_disc_gatt_service.end_handle,
                sizeof(g_disc_gatt_service.end_handle),
                *p_offset + NVM_DISC_GATT_SERVICE_END_HANDLE_OFFSET);

        /* Read the service changed characteristic handle from NVM */
        Nvm_Read((uint16*)&g_disc_gatt_char.start_handle,
                sizeof(g_disc_gatt_char.start_handle),
                *p_offset + NVM_DISC_SERVICE_CHANGED_HANDLE_OFFSET);

        /* Read the service changed Client Characteristic Configuration 
         * descriptor handle from NVM 
         */
        Nvm_Read((uint16*)&g_disc_gatt_char.ccd_handle,
                sizeof(g_disc_gatt_char.ccd_handle),
                *p_offset + NVM_DISC_SERVICE_CHANGED_CCD_HANDLE_OFFSET);
    }
    /* Increment the offset by the number of words of NVM memory required 
     * by the discovered service.
     */
    *p_offset += NVM_DISC_GATT_SERVICE_TOTAL_WORDS;

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetRemoteDiscGattServiceStartHandle
 *
 *  DESCRIPTION
 *      This function returns the start handle for the Discovered GATT 
 *      service.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern uint16 GetRemoteDiscGattServiceStartHandle(void)
{
    return g_disc_gatt_char.start_handle;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetRemoteDiscGattServiceEndHandle
 *
 *  DESCRIPTION
 *      This function returns the end handle for the Discovered GATT 
 *      service.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern uint16 GetRemoteDiscGattServiceEndHandle(void)
{
    return g_disc_gatt_char.end_handle;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      DoesHandleBelongToDiscoveredGattService
 *
 *  DESCRIPTION
 *      This function checks if the handle belongs to discovered GATT 
 *      service
 *
 *  RETURNS
 *      Boolean - Indicating whether handle falls in range or not.
 *
 *---------------------------------------------------------------------------*/

extern bool DoesHandleBelongToDiscoveredGattService(uint16 handle)
{
    return ((handle >= g_disc_gatt_char.start_handle) &&
            (handle <= g_disc_gatt_char.end_handle))
            ? TRUE : FALSE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetGattNotificationHandle
 *
 *  DESCRIPTION
 *      This function gives the GATT Notification Characteristic Handle.
 *
 *  RETURNS
 *      GATT Notification Characteristic Handle
 *
 *---------------------------------------------------------------------------*/

extern uint16 GetGattNotificationHandle(void)
{
    return g_disc_gatt_char.start_handle;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetGattNotificationCCDHandle
 *
 *  DESCRIPTION
 *      This function gives the GATT Notification Characteristic CCD Handle.
 *
 *  RETURNS
 *      This function gives the GATT Notification Characteristic CCD Handle.
 *
 *---------------------------------------------------------------------------*/

extern uint16 GetGattNotificationCCDHandle(void)
{
    return g_disc_gatt_char.ccd_handle;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleGattServiceCharValInd
 *
 *  DESCRIPTION
 *      This function handles the service changed characteristic indication
 *      and starts discovery procedure .
 *
 * RETURNS
        Nothing
 *----------------------------------------------------------------------------*/

extern void HandleGattServiceCharValInd(GATT_CHAR_VAL_IND_T *ind)
{
    if(ind->handle == g_disc_gatt_char.ccd_handle)
    {
        /* Remote Server wants client to rediscover all the handles.
         * start discovery procedure
         */
        DiscoverServices();
    }
}