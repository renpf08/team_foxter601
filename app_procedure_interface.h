/******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      app_procedure_interface.h
 *
 *  DESCRIPTION
 *      Header file for the application-procedure interface
 *
 *
 ******************************************************************************/

#ifndef __APP_PROCEDURE_INTERFACE_H__
#define __APP_PROCEDURE_INTERFACE_H__


/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <main.h>
#include <bluetooth.h>
#include <bt_event_types.h>


/*============================================================================*
 *  Public Data Types
 *============================================================================*/

/* Enum defining codes for the application procedures */
typedef enum
{
    /* Confirmation signal for the Discovery procedure completion.
     */
    APP_DISCOVERY_CFM,
}app_procedure_code;


/* Application procedure events */
typedef struct 
{
    /* Identifies the type of event and hence the corresponding member of
     * APP_PROC_EVENT_T union 
     */
    app_procedure_code                          proc_code;

} APP_PROC_EVENT_COMMON_T;


/* Structure for the Discovery procedure complete event */
typedef struct 
{
    APP_PROC_EVENT_COMMON_T                     proc_event;
    bool                                        procedure_result;

} APP_DISCOVERY_COMPLETE_T;


/**** Union of all application procedure complete events *********************/

/* This union is the common datatype for all events received by
 * AppHandleProcedureCompletionEvent()
 */
typedef union
{
    APP_PROC_EVENT_COMMON_T                     proc_event;

    APP_DISCOVERY_COMPLETE_T                    proc_discovery_complete;

} APP_PROC_EVENT_T;


/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Callback function for the application procedures. */
extern void AppHandleProcedureCompletionEvent(app_procedure_code app_proc_code,
                                              APP_PROC_EVENT_T *p_event_data);

#endif /* __APP_PROCEDURE_INTERFACE_H__ */

