/******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      discovered_ancs_service.c
 *
 *  DESCRIPTION
 *      This file keeps information related to the discovered ANCS
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

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "app_gatt.h"
#include "discovered_ancs_service.h"
#include "nvm_access.h"
#include "ancs_uuids.h"
#include "ancs_client.h"

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* Discovered ANCS notification characteristic */
CHAR_DATA_T g_disc_ancs_char[] = 
{
    {
        INVALID_ATT_HANDLE,     /* start_handle */
        INVALID_ATT_HANDLE,     /* end_handle */
        GATT_UUID128,           /* UUID for ANCS notification characteristic */
        {
            UUID_ANCS_NOTIFICATION_SOURCE_0,
            UUID_ANCS_NOTIFICATION_SOURCE_1,
            UUID_ANCS_NOTIFICATION_SOURCE_2,
            UUID_ANCS_NOTIFICATION_SOURCE_3,
            UUID_ANCS_NOTIFICATION_SOURCE_4,
            UUID_ANCS_NOTIFICATION_SOURCE_5,
            UUID_ANCS_NOTIFICATION_SOURCE_6,
            UUID_ANCS_NOTIFICATION_SOURCE_7
        },

        TRUE,                       /* Has_ccd */
        INVALID_ATT_HANDLE,         /* ccd_handle*/
        0 /*value*/
    },

    {
        INVALID_ATT_HANDLE,     /* start_handle */
        INVALID_ATT_HANDLE,     /* end_handle */
        GATT_UUID128,           /* UUID for ANCS control point characteristic */
        {
            UUID_ANCS_CONTROL_POINT_0,
            UUID_ANCS_CONTROL_POINT_1,
            UUID_ANCS_CONTROL_POINT_2,
            UUID_ANCS_CONTROL_POINT_3,
            UUID_ANCS_CONTROL_POINT_4,
            UUID_ANCS_CONTROL_POINT_5,
            UUID_ANCS_CONTROL_POINT_6,
            UUID_ANCS_CONTROL_POINT_7
        },

        FALSE,                       /* Has_ccd */
        INVALID_ATT_HANDLE,         /* ccd_handle*/
        0 /*value*/
    },

    {
          INVALID_ATT_HANDLE,     /* start_handle */
          INVALID_ATT_HANDLE,     /* end_handle */
    
          GATT_UUID128,           /* UUID for ANCS data source characteristic */
          {
            UUID_ANCS_DATA_SOURCE_0,
            UUID_ANCS_DATA_SOURCE_1,
            UUID_ANCS_DATA_SOURCE_2,
            UUID_ANCS_DATA_SOURCE_3,
            UUID_ANCS_DATA_SOURCE_4,
            UUID_ANCS_DATA_SOURCE_5,
            UUID_ANCS_DATA_SOURCE_6,
            UUID_ANCS_DATA_SOURCE_7
          },
    
          TRUE,                       /* Has_ccd */
          INVALID_ATT_HANDLE,         /* ccd_handle*/
          0 /*value*/
    },
};

/* Discovered ANCS Service  */
SERVICE_DATA_T g_disc_ancs_service = 
{
    INVALID_ATT_HANDLE,         /* start handle */
    INVALID_ATT_HANDLE,         /* end_handle */
    GATT_UUID128,
    {
        UUID_ANCS_SERVICE_0,
        UUID_ANCS_SERVICE_1,
        UUID_ANCS_SERVICE_2,
        UUID_ANCS_SERVICE_3,
        UUID_ANCS_SERVICE_4,
        UUID_ANCS_SERVICE_5,
        UUID_ANCS_SERVICE_6,
        UUID_ANCS_SERVICE_7
    },
    3,                          /* Number of characteristics  */
    0,                          /* characteristic index */
    g_disc_ancs_char,
    0,                          /* NVM_OFFSET*/
    ReadDiscoveredAncsServiceHandlesFromNVM,
    WriteDiscAncsServiceHandlesToNVM,
};


/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      WriteDiscAncsServiceHandlesToNVM
 *
 *  DESCRIPTION
 *      This function stores the discovered service handles in NVM
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void WriteDiscAncsServiceHandlesToNVM(void)
{
    uint16 offset= g_disc_ancs_service.nvm_offset;
    uint16 index;

    /* Store the values in NVM */
    /* Write the service start handle */
    Nvm_Write((uint16*)&g_disc_ancs_service.start_handle,
                sizeof(g_disc_ancs_service.start_handle),
                offset + NVM_DISC_ANCS_SERVICE_START_HANDLE_OFFSET);

    /* Write the service end handle*/
    Nvm_Write((uint16*)&g_disc_ancs_service.end_handle,
                sizeof(g_disc_ancs_service.end_handle),
                offset + NVM_DISC_ANCS_SERVICE_END_HANDLE_OFFSET);

    /* Write the characteristic handles in NVM */
    for(index = 0 ; index < g_disc_ancs_service.num_chars ; index++ )
    {
        /* Write the characteristic handle into NVM */
        Nvm_Write((uint16*)&g_disc_ancs_char[index].start_handle,
                sizeof(g_disc_ancs_char[index].start_handle),
                offset + NVM_DISC_ANCS_NOTIFICATION_HANDLE_OFFSET + (index));
    }
    
    LogReport(__FILE__, __func__, __LINE__, Discovered_Ancs_write_ancs_service_handles_ok);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      ReadDiscoveredAncsServiceHandlesFromNVM
 *
 *  DESCRIPTION
 *      This function reads the ANCS Service handles from NVM
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void ReadDiscoveredAncsServiceHandlesFromNVM(uint16 *p_offset, 
                                                       bool handles_present)
{
    uint16 index;
    /* The application stores the discoverd ANCS service 
     * handles in NVM 
     */
    g_disc_ancs_service.nvm_offset = *p_offset;

    if(handles_present)
    {
        /* Read the service start handle from NVM */
        Nvm_Read((uint16*)&g_disc_ancs_service.start_handle,
                sizeof(g_disc_ancs_service.start_handle),
                *p_offset + NVM_DISC_ANCS_SERVICE_START_HANDLE_OFFSET);

        /* Read the service end handle from NVM */
        Nvm_Read((uint16*)&g_disc_ancs_service.end_handle,
                sizeof(g_disc_ancs_service.end_handle),
                *p_offset + NVM_DISC_ANCS_SERVICE_END_HANDLE_OFFSET);

        /* Read the characteristic handles from NVM */
        for(index = 0 ; index < g_disc_ancs_service.num_chars ; index++ )
        {
            Nvm_Read((uint16*)&g_disc_ancs_char[index].start_handle,
                    sizeof(g_disc_ancs_char[index].start_handle),
                    *p_offset + NVM_DISC_ANCS_NOTIFICATION_HANDLE_OFFSET + 
                    (index));
        }
        LogReport(__FILE__, __func__, __LINE__, Discovered_Ancs_read_ancs_service_handles_ok);
    }
    
    else //! add by mlw at 20200316 14:04
    {
        LogReport(__FILE__, __func__, __LINE__, Discovered_Ancs_read_ancs_service_handles_faild);
    }
    /* Increment the offset by the number of words of NVM memory required 
     * by the discovered Device Information Service.
     */
    *p_offset += NVM_DISC_ANCS_SERVICE_TOTAL_WORDS;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetRemoteDiscAncsServiceStartHandle
 *
 *  DESCRIPTION
 *      This function returns the start handle for the Discovered ANCS 
 *      service.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern uint16 GetRemoteDiscAncsServiceStartHandle(void)
{
    return g_disc_ancs_service.start_handle;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetRemoteDiscAncsServiceEndHandle
 *
 *  DESCRIPTION
 *      This function returns the end handle for the Discovered ANCS 
 *      service.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern uint16 GetRemoteDiscAncsServiceEndHandle(void)
{
    return g_disc_ancs_service.end_handle;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      DoesHandleBelongToDiscoveredAncsService
 *
 *  DESCRIPTION
 *      This function checks if the handle belongs to discovered ANCS 
 *      service
 *
 *  RETURNS
 *      Boolean - Indicating whether handle falls in range or not.
 *
 *---------------------------------------------------------------------------*/

extern bool DoesHandleBelongToDiscoveredAncsService(uint16 handle)
{
    return ((handle >= g_disc_ancs_service.start_handle) &&
            (handle <= g_disc_ancs_service.end_handle))
            ? TRUE : FALSE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetAncsNotificationHandle
 *
 *  DESCRIPTION
 *      This function gives the ANCS Notification Characteristic Handle.
 *
 *  RETURNS
 *      ANCS Notification Characteristic Handle
 *
 *---------------------------------------------------------------------------*/

extern uint16 GetAncsNotificationHandle(void)
{
    return g_disc_ancs_char[0].start_handle;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetAncsNotificationCCDHandle
 *
 *  DESCRIPTION
 *      This function gives the ANCS Notification Characteristic CCD Handle.
 *
 *  RETURNS
 *      This function gives the ANCS Notification Characteristic CCD Handle.
 *
 *---------------------------------------------------------------------------*/

extern uint16 GetAncsNotificationCCDHandle(void)
{
    return g_disc_ancs_char[0].ccd_handle;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetAncsControlPointHandle
 *
 *  DESCRIPTION
 *      This function gives the ANCS Control Point Characteristic Handle.
 *
 *  RETURNS
 *      This function gives the ANCS Control Point Characteristic Handle.
 *
 *---------------------------------------------------------------------------*/

extern uint16 GetAncsControlPointHandle(void)
{
    return g_disc_ancs_char[1].start_handle;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetAncsControlPointHandle
 *
 *  DESCRIPTION
 *      This function gives the ANCS Data Source Characteristic Handle.
 *
 *  RETURNS
 *      This function gives the ANCS Data Source Characteristic Handle.
 *
 *---------------------------------------------------------------------------*/

extern uint16 GetAncsDataSourceHandle(void)
{
    return g_disc_ancs_char[2].start_handle;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetAncsDataSourceCCDHandle
 *
 *  DESCRIPTION
 *      This function gives the ANCS Data Source Characteristic CCD Handle.
 *
 *  RETURNS
 *      ANCS Data Source Characteristic Configuration handle
 *
 *---------------------------------------------------------------------------*/

extern uint16 GetAncsDataSourceCCDHandle(void)
{
    return g_disc_ancs_char[2].ccd_handle;
}
