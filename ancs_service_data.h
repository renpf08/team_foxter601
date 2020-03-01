/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      ancs_service_data.h
 *
 *  DESCRIPTION
 *      Header file for Apple Notification Control Service (ANCS)
 *
 ******************************************************************************/
#ifndef __ANCS_SERVICE_DATA_H__
#define __ANCS_SERVICE_DATA_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <types.h>
#include <bt_event_types.h>
#include <gatt.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "ancs_uuids.h"
#include "ancs_client_gatt.h"
#include "app_gatt.h"

/*============================================================================*
 *  Public Definitions
 *============================================================================*/
/* Notification Source Event Flags */
#define ANCS_NS_EVENTFLAG_SILENT                 (0x01) 
#define ANCS_NS_EVENTFLAG_IMPORTANT              (0x02) 
#define ANCS_NS_EVENTFLAG_RESERVED               ((1<<2)-(1<<7))

/* This macro defines the size of the notification UUID */
#define ANCS_NS_NOTIF_UUID_SIZE                  (4)

/* ANCS error codes */
#define ANCS_ERROR_UNKNOWN_COMMAND               (0xAA0)
#define ANCS_ERROR_INVALID_COMMAND               (0xAA1)
#define ANCS_ERROR_INVALID_PARAMETER             (0xAA2)

/*============================================================================*
 *  Public Data Types
 *============================================================================*/
/* enum for ANC characteristic */
typedef enum 
{
    ancs_notification_source = 0,
    ancs_control_point,
    ancs_data_source,
    ancs_type_invalid
}ancs_type;

/* enum for event id */
typedef enum 
{
    ancs_event_id_notif_added     = 0x0,
    ancs_event_id_notif_modified,
    ancs_event_id_notif_removed,
    ancs_event_id_notif_reserved
}ancs_event_id;

/* enum for category id */
typedef enum
{
    ancs_cat_id_other             = 0x0,   /* Other : updates */
    ancs_cat_id_incoming_call,             /*  Call: Incoming call */
    ancs_cat_id_missed_call,               /* Missed call: Missed Call */
    ancs_cat_id_vmail,                     /* Voice mail: Voice mail*/
    ancs_cat_id_social,                    /* Social message indications */
    ancs_cat_id_schedule,                  /* Schedule: Calendar, planner */
    ancs_cat_id_email,                     /* Email: mail message arrives */
    ancs_cat_id_news,                      /* News: RSS/Atom feeds etc */
    ancs_cat_id_hnf,                       /* Health and Fitness: updates  */
    ancs_cat_id_bnf,                       /* Business and Finance: updates */
    ancs_cat_id_location,                  /* Location: updates */
    ancs_cat_id_entertainment,             /* Entertainment: updates */
    ancs_cat_id_reserved                   /* Reserved */
} ancs_category_id;

/* enum for command id */
typedef enum
{
   ancs_cmd_get_notification_att = 0x0,
   ancs_cmd_get_app_att
}ancs_command_id;

/* enum for notification attribute id */
typedef enum
{
    ancs_notif_att_id_app_id = 0x0,
    ancs_notif_att_id_title,
    ancs_notif_att_id_subtitle,
    ancs_notif_att_id_message,
    ancs_notif_att_id_message_size,
    ancs_notif_att_id_date
}ancs_notif_att_id;


/* enum for app attribute id */
typedef enum
{
    ancs_app_att_display_name = 0x0,
    ancs_app_att_reserved
}ancs_app_att_id;

/* enum for decoding the attribute data */
typedef enum
{
    ds_decoder_hdr = 0x0,
    ds_decoder_attrid,
    ds_decoder_attrlen,
    ds_decoder_attrdata,
    ds_decoder_unknown
}ds_decoder_state;

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/
/* Init function for ANCS Service */
extern void AncsServiceDataInit(void);

/* This function handles the received indications and notifications */
extern bool AncsHandlerNotifInd(GATT_CHAR_VAL_IND_T *p_ind);

/* This function initiates a write request on control point handle */
extern bool AncsWriteRequest(uint16 type,
                             uint8 *p_write_data, uint16 size,uint16 cid);

/* This function formats and sends the Get Notification Attribute request 
 * to the remote peer device. 
 */
extern void AncsGetNotificationAttributeCmd(uint16 cid);

/* This function configures indications on the ANCS Notification 
 * characteristic of ANCS Server.
 */
extern sys_status ConfigureAncsNotifications(uint16 ucid);

/* This function configures indications on the ANCS Data Source Notification 
 * characteristic of ANCS Server.
 */
extern sys_status ConfigureAncsDataSourceNotification(uint16 ucid);

/* This function checks if the passed handle belongs to ANCS Service. */
extern bool DoesHandleBelongsToAncsService(uint16 handle);

/* Get the current connection id */
extern uint16 GetCid(void);
#endif /* __ANCS_SERVICE_DATA_H__ */
