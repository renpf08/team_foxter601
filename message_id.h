/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *
 *  FILE
 *      message_id.h
 *
 *  DESCRIPTION
 *      Header file for the UART messages and strings
 *
 ******************************************************************************/

#ifndef __MESSAGE_ID_H__
#define __MESSAGE_ID_H__

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

#define ANCS_FIRST_REQUEST_ID           (0x1)

/* enum for host requests */
typedef enum
{
    /* Message index for initiating a connect */
    ANCS_CONNECT_MSG_REQ           = ANCS_FIRST_REQUEST_ID,

    /* Message index for initiating a disconnect. */
    ANCS_DISCONNECT_MSG_REQ,

    /* Message index for sending a Get Notification Attributes Request
    */
    ANCS_GET_NOTIF_ATTR_REQ,

    /* Message index for sending a Get App Attribute Request
    */
    ANCS_GET_APP_ATTR_REQ,

   /* Add all MSG ID's above */
   ANCS_UNKNOWN_MSG_REQ 
}ANCS_REQUEST_MSG;


/* Text to be displayed when a connection is being initiated. */
#define STR_CONNECTING          "\r\n+CONNECTING"

/* Text to be displayed when a connection is established with a remote device.*/
#define STR_CONNECTED           "\r\n+CONNECTED\r\n"

/* Text to be displayed when a disconnection is being initiated. */
#define STR_DISCONNECTING       "\r\n+DISCONNECTING"

/* Text to be displayed when disconnected from a remote device.*/
#define STR_DISCONNECTED        "\r\n+DISCONNECTED\r\n"

/* Text to be displayed when a Get Notification Attributes req is initiated. */
#define STR_GNA_COMMAND         "\r\n+GET NOTIFICATION ATTRIBUTES REQ\r\n"

/* Text to be displayed when a Get App Attributes request is being initiated. */
#define STR_GAA_COMMAND         "\r\n+GET APP ATTRIBUTES REQ\r\n"

/* Appended text when a command is requested without a response.*/
#define STR_RETURN              "\r\n"

/* The string that will be displayed when the requested command is 
 * not allowed (Error code 1).
 */
#define STR_ERROR_NOTALLOWED    "+ERROR: NOT ALLOWED\r\n"

/* The string to be displayed when the requested command cannot be processed 
 * as the device is not ready i.e. maybe not connected (Error code 2). 
 */
#define STR_ERROR_NOTREADY      "+ERROR: NOT READY\r\n"

/* The string to be displayed when the requested command cannot be processed 
 * as another command is already in progress (Error code 3). 
 */
#define STR_ERROR_CMDINPROGRESS "+ERROR: BUSY\r\n"

#endif /* __MESSAGE_ID_H__*/
