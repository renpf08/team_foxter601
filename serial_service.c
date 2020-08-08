/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 * FILE
 *     serial_service.c
 *
 * DESCRIPTION
 *     This file defines routines for using serial service.
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *===========================================================================*/

#include <timer.h>
#include <buf_utils.h>
/*============================================================================*
 *  Local Header Files
 *===========================================================================*/

#include "serial_service.h"
#include "nvm_access.h"
#include "app_gatt_db.h"
#include "app_gatt.h"
#include "ancs_client.h"
#include "adapter/adapter.h"

/*============================================================================*
 *  Private Definitions
 *===========================================================================*/
/* Number of words of NVM memory used by serial service */
#define SERIAL_SERVICE_NVM_MEMORY_WORDS              (1)

/* The offset of data being stored in NVM for serial service. This offset is 
 * added to serial service offset to NVM region to get the absolute offset at
 * which this data is stored in NVM
 */
#define SERIAL_NVM_LEVEL_CLIENT_CONFIG_OFFSET        (0)

/*============================================================================*
 *  Private Data types
 *===========================================================================*/

/* Serial Service data type */
typedef struct _SERIAL_DATA_T
{
    /* Client configuration descriptor for Serial Level characteristic */
    gatt_client_config serial_client_config;

    /* NVM Offset at which Serial data is stored */
    uint16 nvm_offset;

} SERIAL_DATA_T;

/*============================================================================*
 *  Private Data
 *===========================================================================*/

/* Battery Service data instance */
static SERIAL_DATA_T g_serial_data;

void cmd_parse(u8* content, u8 length);

/*============================================================================*
 *  Public Function Implementations
 *===========================================================================*/

#if 0
/*----------------------------------------------------------------------------*
 *  NAME
 *      SerialInitChipReset
 *
 *  DESCRIPTION
 *      This function is used to initialise serial service data structure
 *      at chip reset
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/

extern void SerialInitChipReset(void)
{
    /* Though the Serial Service Hardware Iniitalization is done in the
     * hw_access.c, but still the UART interface has been initialised here, so
     * that the serial interface is always initialised from this module, in case
     * it is changed going forward
     */
    InitUart();

    /* Don't wakeup on UART RX line */
    SleepWakeOnUartRX(TRUE);

    /* Determine the current status of Baud Rate PIO and configure UART */
    if(IsBaudRatePIOHigh())
    {
        /* Configure UART for high baud rate */
        SerialConfigureUart(TRUE);
    }
    else
    {
        /* Configure UART for low baud rate */
        SerialConfigureUart(FALSE);
    }
}
#endif

/*----------------------------------------------------------------------------*
 *  NAME
 *      SerialHandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles read operation on serial service attributes
 *      maintained by the application.and responds with the GATT_ACCESS_RSP
 *      message.
 *
 *  RETURNS
 *      Nothing
 *
 *---------------------------------------------------------------------------*/

extern void SerialHandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    /* Initialise to 2 octets for Client Configuration */
    uint16 length = 2;
    uint8  value[2];
    uint8  *p_val = NULL;
    sys_status rc = sys_status_success;

    switch(p_ind->handle)
    {
        case HANDLE_SERIAL_DATA_C_CFG:
        #if USE_BLE_LOG
        case HANDLE_SERIAL_LOG_C_CFG:
        #endif
        {
             p_val = value;
             
             /* copy the client configuration value in response buffer */
            BufWriteUint16(&p_val, g_serial_data.serial_client_config);
        }
        break;
        
        default:
        {
            rc = gatt_status_irq_proceed;
        }
        break;
    }

    /* Send GATT Response for the received request */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc,
                          length, value);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      SerialHandleAccessWrite
 *
 *  DESCRIPTION
 *      This function handles write operation on serial service attributes
 *      maintained by the application.and responds with the GATT_ACCESS_RSP
 *      message.
 *
 *  RETURNS
 *      Nothing
 *
 *---------------------------------------------------------------------------*/

extern void SerialHandleAccessWrite(GATT_ACCESS_IND_T *p_ind)
{
    uint16 client_config;
    uint8 *p_value = p_ind->value;
    sys_status rc = sys_status_success;

    switch(p_ind->handle)
    {
        case HANDLE_SERIAL_SEND_DATA:
        {
        }
        break;
        
        case HANDLE_SERIAL_RECV_DATA:
        {
            cmd_parse((u8*)p_ind->value, (uint8)p_ind->size_value);
        }
        break;
        
        case HANDLE_SERIAL_DATA_C_CFG:
        #if USE_BLE_LOG
        case HANDLE_SERIAL_LOG_C_CFG:
        #endif
        {
            client_config = BufReadUint16(&p_value);
            
           /* Client Configuration is bit field value so ideally bitwise 
            * comparison should be used but since the application supports only 
            * notifications, direct comparison is being used.
            */
            if((client_config == gatt_client_config_notification) ||
               (client_config == gatt_client_config_none))
            {
                g_serial_data.serial_client_config = client_config;
            }
            
            if(AppIsDeviceBonded())
            {
                Nvm_Write((uint16 *)&client_config,
                          sizeof(client_config),
                             g_serial_data.nvm_offset +
                              SERIAL_NVM_LEVEL_CLIENT_CONFIG_OFFSET);
            }
        }
        break;
        
        default:
        {
            rc = gatt_status_write_not_permitted;
        }
        break;
    }

    /* Send ACCESS RESPONSE */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, 0, NULL);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      SerialReadDataFromNVM
 *
 *  DESCRIPTION
 *      This function is used to read serial service specific data stored in
 *      NVM
 *
 *  PARAMETERS
 *      p_offset [in]           Offset to Serial Service data in NVM
 *               [out]          Offset to next entry in NVM
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/

extern void SerialReadDataFromNVM(bool bonded,uint16 *p_offset)
{
    g_serial_data.nvm_offset = *p_offset;

    /* Read NVM only if devices are bonded */
    if(AppIsDeviceBonded())
    {
        /* Read serial level client configuration */
        Nvm_Read((uint16 *)&g_serial_data.serial_client_config,
                   sizeof(g_serial_data.serial_client_config),
                   *p_offset + 
                   SERIAL_NVM_LEVEL_CLIENT_CONFIG_OFFSET);
    }

    /* Increment the offset by the number of words of NVM memory required 
     * by Battery service 
     */
    *p_offset += SERIAL_SERVICE_NVM_MEMORY_WORDS;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      SerialCheckHandleRange
 *
 *  DESCRIPTION
 *      This function is used to check if the handle belongs to the serial
 *      service
 *
 *  PARAMETERS
 *      handle [in]             Handle to check
 *
 *  RETURNS
 *      TRUE if handle belongs to the Battery Service, FALSE otherwise
 *----------------------------------------------------------------------------*/

extern bool SerialCheckHandleRange(uint16 handle)
{
    return ((handle >= HANDLE_SERIAL_SERVICE) &&
            (handle <= HANDLE_SERIAL_SERVICE_END))
            ? TRUE : FALSE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      SerialSendNotification
 *
 *  DESCRIPTION
 *      Sends the serial service notification.
 *
 *  PARAMETERS
 *      data [in]                data to be send
 *      size [in]                size of the data to be send
 *
 *  RETURNS
 *      TRUE, if notification is initiated, else FALSE
 *----------------------------------------------------------------------------*/

extern bool SerialSendNotification(uint8 *data, uint16 size, uint16 handle)
{
    if(size <= SERIAL_RX_DATA_LENGTH)
    {
      if(AppGetState() == app_connected)
      {
        if(g_serial_data.serial_client_config == gatt_client_config_notification)
        {
            GattCharValueNotification(GetConnectionID(),
                                  handle,
                                  size,
                                  data);

            return TRUE;
        }
      }
    }

    return FALSE;
}

extern bool ble_send_data(uint8 *data, uint16 size)
{
    if(g_app_data.state != app_connected) return false;
    size = (size>20)?20:size;
    return SerialSendNotification(data, size, HANDLE_SERIAL_SEND_DATA);
}
#if USE_BLE_LOG
/**
*	info message
*	5F 00 xx yy // pair code, xx:yy
*	5F 01 aa bb cc dd // state machine result
*         aa:init state
*         bb:event
*         cc:next state
*         dd:1-executed,0-not executed
*	5F 02 xx // zero adjust jump empty
*         xx=1:press lock; xx=2:run lock
*	5F 03 aa bb cc dd // notify msg
*         aa:sta, fixed to: 0:added, 1:modified, 2:removed
*         bb:level, 0~255, look appMsgList[] of MESSAGE_POSITION_xxx for details
*         cc:type, look appMsgList[] of APP_ID_STRING_xxx's index for details
*         dd:cnt, msg count
*	5F 04 aa bb cc dd // motor queue
*         aa:queue useer
*         bb:motor name
*         cc:tail
*         dd:head
*/
extern bool ble_send_log(uint8 *data, uint16 size)
{
    #if USE_CMD_TEST_LOG_TYPE_EN
    typedef struct{u8 head; u8 type;}log_t;
    log_t* log = (log_t*)data;

    if(log->head != CMD_TEST_SEND) {
        return false;
    }
    if(adapter_ctrl.log_type_en[log->type] == 0) {
        return false;
    }    
    #endif
    
    if(g_app_data.state != app_connected) {
        return false;
    }
    size = (size>20)?20:size;
    return SerialSendNotification(data, size, HANDLE_SERIAL_SEND_LOG);
}
#endif

#if 0
/*----------------------------------------------------------------------------*
 *  NAME
 *      SerialConfigureUart
 *
 *  DESCRIPTION
 *      Configure the UART port.
 *
 *  PARAMETERS
 *      bool   TRUE for High baud rate.
 *             FALSE for low baud rate.
 *  RETURNS
 *      nothing
 *----------------------------------------------------------------------------*/
extern void SerialConfigureUart(bool bHigh)
{
    ConfigureUart(bHigh);
}
#endif

/*----------------------------------------------------------------------------*
 *  NAME
 *      SerialBondingNotify
 *
 *  DESCRIPTION
 *      This function is used by application to notify bonding status to 
 *      Serial service
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void SerialBondingNotify()
{
     /* Write data to NVM if bond is established */
    if(AppIsDeviceBonded())
    {
        /* Write to NVM the client configuration value of battery level
         * that was configured prior to bonding
         */
        Nvm_Write((uint16*)&g_serial_data.serial_client_config,
                  sizeof(g_serial_data.serial_client_config),
                  g_serial_data.nvm_offset +
                  SERIAL_NVM_LEVEL_CLIENT_CONFIG_OFFSET);
    }
}

#ifdef NVM_TYPE_FLASH
/*----------------------------------------------------------------------------*
 *  NAME
 *      WriteSerialServiceDataInNvm
 *
 *  DESCRIPTION
 *      This function writes Serial service data in NVM
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern void WriteSerialServiceDataInNvm(void)
{
    /* Write to NVM the client configuration value. */
    Nvm_Write((uint16*)&g_serial_data.serial_client_config, 
              sizeof(g_serial_data.serial_client_config),
              g_serial_data.nvm_offset + 
              SERIAL_NVM_LEVEL_CLIENT_CONFIG_OFFSET);
}
#endif /* NVM_TYPE_FLASH */
