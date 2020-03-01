/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      dev_info_service.c
 *
 *  DESCRIPTION
 *      This file defines routines for using the Device Information Service.
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header includes 
 *============================================================================*/

#include <gatt.h>
#include <bluetooth.h>
#include <config_store.h>

/*============================================================================*
 *  Local Header files
 *============================================================================*/

#include "app_gatt_db.h"    
#include "dev_info_service.h"
#include "m_printf.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Device Information Service System ID characteristic */
/* Bytes have been reversed */
#define SYSTEM_ID_FIXED_CONSTANT    (0xFFFE)
#define SYSTEM_ID_LENGTH            (8)

/*============================================================================*
 *  Private Datatypes
 *============================================================================*/

/* System ID : System ID has two fields;
 * Manufacturer Identifier           : The Company Identifier is concatenated
 *                                     with 0xFFFE
 * Organizationally Unique Identifier: Company Assigned Identifier of the
 *                                     Bluetooth Address
 *
 *
 * See following web link for definition of system ID
 * http://developer.bluetooth.org/gatt/characteristics/Pages/
 * CharacteristicViewer.aspx?u=org.bluetooth.characteristic.system_id.xml
 */

typedef struct _SYSTEM_ID_T
{
    /* System ID size is 8 octets */
    uint8 byte[SYSTEM_ID_LENGTH];
} SYSTEM_ID_T;

typedef struct _DEV_INFO_DATA_T
{
    /* System ID of Device Information Service */
    SYSTEM_ID_T system_id;
} DEV_INFO_DATA_T;

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* DIS data structure */
static DEV_INFO_DATA_T          g_dev_info_data;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Calculate the System ID based on the Bluetooth address of the device */
static bool getSystemId(SYSTEM_ID_T *sys_id);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      getSystemId
 *
 *  DESCRIPTION
 *      This function returns the Device Information Service System ID.
 *
 *
 *  RETURNS
 *      TRUE on success,
 *      FALSE if unable to access the device's Bluetooth address
 *----------------------------------------------------------------------------*/
static bool getSystemId(SYSTEM_ID_T * sys_id)
{
    BD_ADDR_T bdaddr;           /* Device's Bluetooth address */

    if(CSReadBdaddr(&bdaddr))
    {
        /* Manufacturer-defined identifier */
        sys_id->byte[0] = (uint8)(bdaddr.lap);
        sys_id->byte[1] = (uint8)(bdaddr.lap >> 8);
        sys_id->byte[2] = (uint8)(bdaddr.lap >> 16);
        sys_id->byte[3] = (uint8)(SYSTEM_ID_FIXED_CONSTANT);
        sys_id->byte[4] = (uint8)(SYSTEM_ID_FIXED_CONSTANT >> 8);

        /* Organizationally Unique Identifier */
        sys_id->byte[5] = (uint8)(bdaddr.uap);
        sys_id->byte[6] = (uint8)(bdaddr.nap);
        sys_id->byte[7] = (uint8)(bdaddr.nap >> 8);
        
        m_printf("addr: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
                 sys_id->byte[0],sys_id->byte[1],sys_id->byte[2],sys_id->byte[3],sys_id->byte[4],sys_id->byte[5],sys_id->byte[6],sys_id->byte[7]);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      DeviceInfoHandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles read operations on Device Information Service
 *      attributes maintained by the application and responds with the
 *      GATT_ACCESS_RSP message.
 *
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void DeviceInfoHandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    uint16 length = 0;                  /* Length of attribute data, octets */
    uint8 *p_value = NULL;              /* Pointer to attribute value */
    sys_status rc = gatt_status_irq_proceed;  /* Function status */

    switch(p_ind->handle)
    {
        case HANDLE_DEVICE_INFO_SYSTEM_ID:
        {
            /* System ID read has been requested */
            length = SYSTEM_ID_LENGTH;
            if(getSystemId(&g_dev_info_data.system_id))
            {
                p_value = (uint8 *)(&g_dev_info_data.system_id);
                rc = sys_status_success;
            }
        }
        break;
    }

    /* Send response indication */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, length, p_value);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      DeviceInfoCheckHandleRange
 *
 *  DESCRIPTION
 *      This function is used to check if the handle belongs to the Device
 *      Information Service.
 *
 *
 *  RETURNS
 *      TRUE if handle belongs to the Device Information Service, FALSE
 *      otherwise
 *----------------------------------------------------------------------------*/
extern bool DeviceInfoCheckHandleRange(uint16 handle)
{
    return ((handle >= HANDLE_DEVICE_INFO_SERVICE) &&
            (handle <= HANDLE_DEVICE_INFO_SERVICE_END))
            ? TRUE : FALSE;
}
