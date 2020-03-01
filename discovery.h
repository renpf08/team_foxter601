/******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      discovery.h
 *
 *  DESCRIPTION
 *      Header file for GATT database discovery routine
 *
 *
 ******************************************************************************/

#ifndef __DISCOVERY_H__
#define __DISCOVERY_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <types.h>
#include <gatt.h>
#include <bt_event_types.h>


/*============================================================================*
 *  Public Data Types
 *============================================================================*/
/* Structure for a characteristic */
typedef struct
{
    uint16          start_handle;   /* Start handle for a characteristic */
    uint16          end_handle;     /* End handle for a characteristic */
    GATT_UUID_T     uuid_type;      /* UUID type 16-bit or 128-bit*/
    uint16          uuid[8];        /* UUID array */
    bool            has_ccd;        /* Does the characteristic has client 
                                     * configuration 
                                     */
    uint16          ccd_handle;     /* Handle for the client configuration 
                                     * attribute
                                     */
    uint16          value;          /* Client configuration */
}CHAR_DATA_T;


/* Structure for the discovered service */
typedef struct
{
    uint16          start_handle;   /* Start handle for a service */
    uint16          end_handle;     /* End handle for a service */
    GATT_UUID_T     uuid_type;      /* UUID type 16-bit or 128-bit */
    uint16          uuid[8];        /* UUID array */
    uint16          num_chars;      /* Number of characteristics in the service 
                                     */
    uint16          char_index;     /* Handy characteristic index being used 
                                     * while discovering characteristic 
                                     * descriptors.
                                     */
    CHAR_DATA_T     *char_data;     /* Pointer to characteristic data */
    uint16          nvm_offset;     /* NVM offset for the service data */

    /* Function for reading discovered service attribute handles stored in NVM
     */
    void    (*ReadDiscServiceSpecificHandlesFromNVM)(uint16 *p_offset, 
                                                     bool handles_present);

    /* Function for writing handles in NVM space.*/
    void    (*WriteDiscServiceSpecificHandlesToNVM)(void);

   
}SERVICE_DATA_T;

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/
/* This function starts the GATT database discovery */
extern void StartGattDatabaseDiscovery(uint16 ucid);

/*  This function stops the GATT database discovery */
extern void StopGattDatabaseDiscovery(void);

/* This function discovers a primary service */
extern void DiscoverAPrimaryService(uint16 ucid);

/* This function handles the signal GATT_DISC_PRIM_SERV_BY_UUID_IND */
extern void HandleGenericDiscoverPrimaryServiceInd(
                                     GATT_DISC_PRIM_SERV_BY_UUID_IND_T *p_prim);

/* This function handles the signal GATT_DISC_PRIM_SERV_BY_UUID_CFM */
extern void HandleGenericServiceDiscoverPrimaryServiceByUuidCfm(
                                     GATT_DISC_PRIM_SERV_BY_UUID_CFM_T *p_prim,
                                     uint16 ucid);

/* This function handles the signal GATT_CHAR_DECL_INFO_IND */
extern void HandleGenericGattServiceCharacteristicDeclarationInfoInd(
                                     GATT_CHAR_DECL_INFO_IND_T *ind);

/* This function handles GATT_DISC_SERVICE_CHAR_CFM messages. */
extern void HandleGenericGattDiscoverServiceCharacteristicCfm(
                                          GATT_DISC_SERVICE_CHAR_CFM_T *pInd,
                                          uint16 ucid);

/* This function handles GATT_CHAR_DESC_INFO_IND messages. */
extern void HandleGenericGattCharacteristicDescriptorInfoInd(
                                        GATT_CHAR_DESC_INFO_IND_T *p_prim);

/* This function handles GATT_DISC_ALL_CHAR_DESC_CFM messages. */
extern void HandleGenericGattCharacteristicDescriptorCfm(
                                        GATT_DISC_ALL_CHAR_DESC_CFM_T *p_prim,
                                        uint16 ucid);

/* This function reads the already discoverd attribute handles from NVM */
extern void ReadDiscoveredGattDatabaseFromNVM(uint16 *p_offset, 
                                              bool handles_present);

/* This function should be called on NVM intialisation phase and it stores
 * INVALID_ATT_HANDLE for all the GATT database attributes
 */
extern void WriteDiscoveredGattDatabaseToNVM(void);

/* This function resets the discovered handles database */
extern void ResetDiscoveredHandlesDatabase(void);

#endif /* __DISCOVERY_H__ */
