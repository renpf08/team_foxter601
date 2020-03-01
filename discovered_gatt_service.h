/******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      discovered_gatt_service.h
 *
 *  DESCRIPTION
 *      Header file for the service information
 *
 *
 ******************************************************************************/
#ifndef __DISCOVERED_GATT_SERVICE_H__
#define __DISCOVERED_GATT_SERVICE_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <gatt.h>
#include <bt_event_types.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "discovery.h"

/*============================================================================*
 *  Public Definitions
 *============================================================================*/
/* Macros for NVM access */
#define NVM_DISC_GATT_SERVICE_START_HANDLE_OFFSET                       (0)
#define NVM_DISC_GATT_SERVICE_END_HANDLE_OFFSET                         (1)
#define NVM_DISC_SERVICE_CHANGED_HANDLE_OFFSET                          (2)
#define NVM_DISC_SERVICE_CHANGED_CCD_HANDLE_OFFSET                      (3)
    
#define NVM_DISC_GATT_SERVICE_TOTAL_WORDS                               (4)
    
/*============================================================================*
 *  Public Data Declaration
 *============================================================================*/
extern SERVICE_DATA_T g_disc_gatt_service;

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/
/* This function stores the discovered service handles in NVM */
extern void WriteDiscGattServiceHandlesToNVM(void);

/* This function reads the GATT service handles from NVM */
extern void ReadDiscoveredGattServiceHandlesFromNVM(uint16 *p_offset,
                                                    bool handles_present);

/* This function returns the start handle for the Discovered GATT 
 * service.
 */
extern uint16 GetRemoteDiscGattServiceStartHandle(void);

/* This function returns the end handle for the Discovered GATT 
 * service.
 */
extern uint16 GetRemoteDiscGattServiceEndHandle(void);


/* This function returns true if the provided handle belongs to ANCS Service */
extern bool DoesHandleBelongToDiscoveredGattService(uint16 handle);

/* This function returns ANCS Service notification handle */
extern uint16 GetGattNotificationHandle(void);

/* This function returns ANCS Service notification CCD handle */
extern uint16 GetGattNotificationCCDHandle(void);

/* This function handles the service changed characteristic indication
 *  and starts discovery procedure.
 */
extern void HandleGattServiceCharValInd(GATT_CHAR_VAL_IND_T *ind);
#endif /* __DISCOVERED_GATT_SERVICE_H__ */
