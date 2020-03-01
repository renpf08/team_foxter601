/******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      discovery.c
 *
 *  DESCRIPTION
 *      This file defines routines for GATT database discovery
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
#include "discovery.h"
#include "app_procedure_interface.h"
#include "app_gatt.h"
#include "discovered_ancs_service.h"
#include "discovered_gatt_service.h"

/*============================================================================*
 *  Private Data Types
 *============================================================================*/


typedef struct
{
    /* Boolean flag which tells if the discovery procedure is ongoing or not */
    bool        is_discovering;

    /* variable which tells the index of the service structure currently being
     * discovered in the g_service_list
     */
    uint16      service_index;

    /* Boolean flag which tells if the discovery procedure was successful */
    bool        discovery_result;
}DISCOVERY_PROC_T;


/*============================================================================*
 *  Private Data
 *============================================================================*/
/* Discovery procedure data structure.*/
static DISCOVERY_PROC_T                     g_discovery_data;

/* Pointer to the currently used service structure */
static SERVICE_DATA_T                       *g_service_data;

/* List of pointers to supported service structure. Terminated
 * by a null pointer */
static SERVICE_DATA_T                       *g_service_list[] =
                                            {
                                                &g_disc_ancs_service,
                                                &g_disc_gatt_service,
                                                NULL
                                            };

/*============================================================================*
 *  Public Function Definitions
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      StartGattDatabaseDiscovery
 *
 *  DESCRIPTION
 *      This function starts the GATT database discovery
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/

extern void StartGattDatabaseDiscovery(uint16 ucid)
{
    /* This discovery procedure discovers all the primary services listed
     * in the array discovery_array. Start with the service at index 0.
     */
    g_discovery_data.service_index = 0;
    g_discovery_data.is_discovering = TRUE;

    /* Initialise the discovery_result to SUCCESS, reset it to 
     * false upon first failure of discovery
     */
    g_discovery_data.discovery_result = TRUE;
    DiscoverAPrimaryService(ucid);
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      StopGattDatabaseDiscovery
 *
 *  DESCRIPTION
 *      This function stops the GATT database discovery
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/

extern void StopGattDatabaseDiscovery(void)
{

    APP_DISCOVERY_COMPLETE_T        g_discovery_complete;

    /* Write the discovered database handles into NVM */
    WriteDiscoveredGattDatabaseToNVM();

    /* Service discovery is complete. Set the is_discovering flag to FALSE */
    g_discovery_data.is_discovering = FALSE;

    /* Tell the main application about the service discover completion. */
    g_discovery_complete.procedure_result = g_discovery_data.discovery_result;
    AppHandleProcedureCompletionEvent(APP_DISCOVERY_CFM, 
                                     (APP_PROC_EVENT_T *)&g_discovery_complete);
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      DiscoverAPrimaryService
 *
 *  DESCRIPTION
 *      This function discovers a primary service.
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
extern void DiscoverAPrimaryService(uint16 ucid)
{
    /* Initialise the service data */
    g_service_data = g_service_list[g_discovery_data.service_index];

    /* Start discovery */
    GattDiscoverPrimaryServiceByUuid(ucid,
                                 g_service_data->uuid_type,
                                 g_service_data->uuid);
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleGenericDiscoverPrimaryServiceInd
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_DISC_PRIM_SERV_BY_UUID_IND
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/

extern void HandleGenericDiscoverPrimaryServiceInd(
                                     GATT_DISC_PRIM_SERV_BY_UUID_IND_T *p_prim)
{
    /* Store start and end handles of the service */
    g_service_data->start_handle = p_prim->strt_handle;
    g_service_data->end_handle   = p_prim->end_handle;
    g_service_data->char_index = 0;
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleGenericServiceDiscoverPrimaryServiceByUuidCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_DISC_PRIM_SERV_BY_UUID_CFM
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/

extern void HandleGenericServiceDiscoverPrimaryServiceByUuidCfm(
                                     GATT_DISC_PRIM_SERV_BY_UUID_CFM_T *p_prim,
                                     uint16 ucid)
{
    /* If primary service discovery was a success and valid handles have been 
     * discovered, go ahead with the characteristic discovery.
     */
    if((p_prim->result == sys_status_success) &&
       (g_service_data->start_handle != INVALID_ATT_HANDLE) && 
       (g_service_data->end_handle != INVALID_ATT_HANDLE))
    {
        /* Initialise the char index to invalid index */
        g_service_data->char_index = 0xFFFF;

        /* Discover service characteristics */
        GattDiscoverServiceChar(ucid,
                                g_service_data->start_handle,
                                g_service_data->end_handle,
                                GATT_UUID_NONE,
                                NULL);
    }
    else
    {
        /* Discovery has failed set the discovery_result to FALSE 
         * but continue if there are more services to be discovered
         */
        g_discovery_data.discovery_result = FALSE;

        /* Increment the service index to the next service in the service list 
         */
        g_discovery_data.service_index++;

        /* If there are more services to be discoverd, go ahead */
        if(g_service_list[g_discovery_data.service_index] != NULL )
        {
            DiscoverAPrimaryService(ucid);
        }
        else
        {
            /* If not, stop the discovery */
            StopGattDatabaseDiscovery();
        }
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleGenericGattServiceCharacteristicDeclarationInfoInd
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_CHAR_DECL_INFO_IND
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/

extern void HandleGenericGattServiceCharacteristicDeclarationInfoInd(
                                     GATT_CHAR_DECL_INFO_IND_T *ind)
{
    uint16 *uuid = ind->uuid;
    uint16 index;

    /* Iterate through the whole list of characteristics */
    for(index = 0; index < g_service_data->num_chars; index++ )
    {
        /* Check the UUID to see whether it is one of interest. */
        if(
            /* If it's a 16-bit UUID */
            (g_service_data->char_data[index].uuid_type == GATT_UUID16 && 
            uuid[0] == g_service_data->char_data[index].uuid[0]) ||

            /* If it's a 128 bit UUID */
            (g_service_data->char_data[index].uuid_type == GATT_UUID128 &&
            uuid[0] == g_service_data->char_data[index].uuid[0] &&
            uuid[1] == g_service_data->char_data[index].uuid[1] &&
            uuid[2] == g_service_data->char_data[index].uuid[2] &&
            uuid[3] == g_service_data->char_data[index].uuid[3] &&
            uuid[4] == g_service_data->char_data[index].uuid[4] &&
            uuid[5] == g_service_data->char_data[index].uuid[5] &&
            uuid[6] == g_service_data->char_data[index].uuid[6] &&
            uuid[7] == g_service_data->char_data[index].uuid[7] )
          )
        {
            /* Store the discovered characteristic handle */
            g_service_data->char_data[index].start_handle = ind->val_handle;

            /* It could be the last characteristic of the service so
             * assign service end handle to it. If required, the end handle 
             * will be updated with the appropriate value on the next 
             * characteristic indication
             */
            g_service_data->char_data[index].end_handle = 
                                               g_service_data->end_handle;

            if( g_service_data->char_index != 0xFFFF )
            {
                /* More than 1 characteristics are discovered so assign the
                 * the (value_handle - 2) to the end_handle of previous 
                 * characteristic
                 */
                g_service_data->char_data[g_service_data->char_index].end_handle
                                                        = ind->val_handle - 2;
            }

            /* Update the char_index to the new identified characteristic
             * index 
             */
            g_service_data->char_index = index;

            break;
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleGenericGattDiscoverServiceCharacteristicCfm
 *
 *  DESCRIPTION
 *      This function handles GATT_DISC_SERVICE_CHAR_CFM messages.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void HandleGenericGattDiscoverServiceCharacteristicCfm(
                                          GATT_DISC_SERVICE_CHAR_CFM_T *pInd,
                                          uint16 ucid)
{
    uint16 index;
    bool isDescDiscoveryRequired = FALSE;

    /* Characteristic discovery is complete, discover the characteristic 
     * descriptors if required
     */
    for( index = 0; index < g_service_data->num_chars; index++)
    {
        if( g_service_data->char_data[index].has_ccd )
        {
            isDescDiscoveryRequired = TRUE;
            g_service_data->char_index = index;
            break;
        }
    }

    /* If characteristic discovery was successful and there are descriptors to 
     * be discovered, go ahead with the descriptor discovery.
     */
    if(pInd->result == sys_status_success && 
      (g_service_data->char_data[index].start_handle != INVALID_ATT_HANDLE) &&
       isDescDiscoveryRequired )
    {
        /* Characteristic discovery is successful and at least one 
         * characteristic has descriptor to be discovered
         */
        GattDiscoverAllCharDescriptors(ucid,
                            g_service_data->char_data[index].start_handle,
                            g_service_data->char_data[index].end_handle);
    }
    else
    {
        /* Discovery has failed set the discovery_result to FALSE 
         * but continue if there are more services to be discovered
         */
        if( pInd->result != sys_status_success )
        {
            g_discovery_data.discovery_result = FALSE;
        }

        /* Move to the next location in the service list */
        g_discovery_data.service_index++;

        /* If there are more services to be discovered, discover it. */
        if(g_service_list[g_discovery_data.service_index] != NULL)
        {
            DiscoverAPrimaryService(ucid);
        }
        else
        {
            /* If not, stop the discovery */
            StopGattDatabaseDiscovery();
        }
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleGenericGattCharacteristicDescriptorInfoInd
 *
 *  DESCRIPTION
 *      This function handles GATT_CHAR_DESC_INFO_IND messages.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void HandleGenericGattCharacteristicDescriptorInfoInd(
                                        GATT_CHAR_DESC_INFO_IND_T *p_prim)
{
    uint16 index = g_service_data->char_index;

    /* If client configuration descriptor has been discovered, store its handle 
     */
    if( p_prim->uuid_type == GATT_UUID16 &&
        p_prim->uuid[0] == UUID_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC )
    {
        g_service_data->char_data[index].ccd_handle = p_prim->desc_handle;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleGenericGattCharacteristicDescriptorCfm
 *
 *  DESCRIPTION
 *      This function handles GATT_DISC_ALL_CHAR_DESC_CFM messages.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void HandleGenericGattCharacteristicDescriptorCfm(
                                        GATT_DISC_ALL_CHAR_DESC_CFM_T *p_prim,
                                        uint16 ucid)
{
    bool isDescDiscoveryComplete = TRUE;
    uint16 index;

    /* If descriptor discovery was successful, go ahead with discovering 
     * descriptors and services.
     */
    if(p_prim->result == sys_status_success)
    {
        index = g_service_data->char_index;

        /* Continue to discover descriptors if any more
         * characteristics of the service have descriptors
         */
        while( ++index < g_service_data->num_chars)
        {
            if( g_service_data->char_data[index].has_ccd )
            {
                isDescDiscoveryComplete = FALSE;
                g_service_data->char_index = index;
                break;
            }
        }

        if( isDescDiscoveryComplete )
        {
            /* Move to the next location in the service list */
            g_discovery_data.service_index++;

            /* If there are more services to be discovered, go ahead with 
             * discovering the next service.
             */
            if(g_service_list[g_discovery_data.service_index] != NULL )
            {
                DiscoverAPrimaryService(ucid);
            }
            else /* No more service to be discovered, stop the discovery 
                  * procedure. 
                  */
            {
                StopGattDatabaseDiscovery();
            }
        }
        else
        {
            /* More descriptors to discover */
            GattDiscoverAllCharDescriptors(ucid,
                    g_service_data->char_data[index].start_handle,
                    g_service_data->char_data[index].end_handle);
        }
    }
    else
    {
        /* Service discovery has failed so stop */
        g_discovery_data.discovery_result = FALSE;
        StopGattDatabaseDiscovery();
    }

}


/*----------------------------------------------------------------------------*
 *  NAME
 *      ReadDiscoveredGattDatabaseFromNVM
 *
 *  DESCRIPTION
 *      This function reads the already discoverd attribute handles from NVM
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void ReadDiscoveredGattDatabaseFromNVM(uint16 *p_offset, 
                                              bool handles_present)
{
    uint8 index  = 0;

    while( g_service_list[index] != NULL )
    {
        /* Read service attributes for all services in the list. */
        g_service_list[index]->ReadDiscServiceSpecificHandlesFromNVM(p_offset, 
                                                               handles_present);
        index++;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      WriteDiscoveredGattDatabaseToNVM
 *
 *  DESCRIPTION
 *      This function should be called on NVM intialisation phase and it stores
 *      INVALID_ATT_HANDLE for all the GATT database attributes
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void WriteDiscoveredGattDatabaseToNVM(void)
{
    uint8 index = 0;
    while( g_service_list[index] != NULL )
    {
        /* Read service attributes for all services in the list. */
        g_service_list[index]->WriteDiscServiceSpecificHandlesToNVM();
        index++;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      ResetDiscoveredHandlesDatabase
 *
 *  DESCRIPTION
 *      This function resets the discovered handles database.
 *
 *  RETURNS
 *      Nothing.
 *----------------------------------------------------------------------------*/
extern void ResetDiscoveredHandlesDatabase(void)
{
     uint8 srv_index = 0; 
     uint8 char_index;

     while( g_service_list[srv_index] != NULL)
     {  
         /* Reset service start handle and end handle*/
         g_service_data = g_service_list[srv_index];         
         g_service_data->start_handle = INVALID_ATT_HANDLE;
         g_service_data->end_handle = INVALID_ATT_HANDLE;
         
         /* Reset characteristic start handle ,end handle and client 
          * configuration descriptor handle
          */
         for(char_index = 0; char_index < g_service_data->num_chars;
             char_index++)
         {
             g_service_data->char_data[char_index].start_handle = 
                     INVALID_ATT_HANDLE;
             g_service_data->char_data[char_index].end_handle = 
                     INVALID_ATT_HANDLE;
             g_service_data->char_data[char_index].ccd_handle = 
                     INVALID_ATT_HANDLE;
         }
         
         /* Update the handle values in NVM */
         g_service_data->WriteDiscServiceSpecificHandlesToNVM();
         srv_index++;
     }     
}


