/******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      discovered_ancs_service.h
 *
 *  DESCRIPTION
 *      Header file for the service information
 *
 *
 ******************************************************************************/
#ifndef __DISCOVERED_ANCS_SERVICE_H__
#define __DISCOVERED_ANCS_SERVICE_H__

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
#define NVM_DISC_ANCS_SERVICE_START_HANDLE_OFFSET                      (0)
#define NVM_DISC_ANCS_SERVICE_END_HANDLE_OFFSET                        (1)
#define NVM_DISC_ANCS_NOTIFICATION_HANDLE_OFFSET                       (2)
#define NVM_DISC_ANCS_NOTIFICATION_CCD_HANDLE_OFFSET                   (3)
#define NVM_DISC_ANCS_CONTROL_POINT_HANDLE_OFFSET                      (4)
#define NVM_DISC_ANCS_DATA_SOURCE_HANDLE_OFFSET                        (5)
#define NVM_DISC_ANCS_DATA_SOURCE_CCD_HANDLE_OFFSET                    (6)

#define NVM_DISC_ANCS_SERVICE_TOTAL_WORDS                              (7)

/*============================================================================*
 *  Public Data Declaration
 *============================================================================*/

extern SERVICE_DATA_T g_disc_ancs_service;

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function stores the discovered service handles in NVM */
extern void WriteDiscAncsServiceHandlesToNVM(void);

/* This function reads the Device Information service handles from NVM */
extern void ReadDiscoveredAncsServiceHandlesFromNVM(uint16 *p_offset,
                                                          bool handles_present);

/* This function returns the start handle for the Discovered ANCS 
 * service.
 */
extern uint16 GetRemoteDiscAncsServiceStartHandle(void);

/* This function returns the end handle for the Discovered ANCS 
 * service.
 */
extern uint16 GetRemoteDiscAncsServiceEndHandle(void);


/* This function returns true if the provided handle belongs to ANCS Service */
extern bool DoesHandleBelongToDiscoveredAncsService(uint16 handle);

/* This function returns ANCS Service notification handle */
extern uint16 GetAncsNotificationHandle(void);

/* This function returns ANCS Service notification CCD handle */
extern uint16 GetAncsNotificationCCDHandle(void);

/* This function returns ANCS Service control point handle */
extern uint16 GetAncsControlPointHandle(void);

/* This function returns ANCS Service data source handle */
extern uint16 GetAncsDataSourceHandle(void);

/* This function returns ANCS Service data source notification CCD handle */
extern uint16 GetAncsDataSourceCCDHandle(void);

#endif /* __DISCOVERED_ANCS_SERVICE_H__ */
