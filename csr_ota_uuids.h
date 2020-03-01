/******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      csr_ota_uuids.h
 *
 *  DESCRIPTION
 *      UUID MACROS for CSR OTA Update Application Service.
 *
 *****************************************************************************/

#ifndef _CSR_OTA_UUIDS_H
#define _CSR_OTA_UUIDS_H

/*=============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "user_config.h"

/*=============================================================================*
 *  Public Definitions
 *============================================================================*/

/* *****************************************************************************
 * This is the UUID for the CSR Over-the-Air Update Application Service 
 */
#define CSR_OTA_UPDATE_SERV_UUID_WORD1      0x0000
#define CSR_OTA_UPDATE_SERV_UUID_WORD2      0x1016
#define CSR_OTA_UPDATE_SERV_UUID_WORD3      0xd102
#define CSR_OTA_UPDATE_SERV_UUID_WORD4      0x11e1
#define CSR_OTA_UPDATE_SERV_UUID_WORD5      0x9b23
#define CSR_OTA_UPDATE_SERV_UUID_WORD6      0x0002
#define CSR_OTA_UPDATE_SERV_UUID_WORD7      0x5b00
#define CSR_OTA_UPDATE_SERV_UUID_WORD8      0xa5a5

#define CSR_OTA_UPDATE_SERVICE_UUID         0x00001016d10211e19b2300025b00a5a5

/* *****************************************************************************
 * This is the UUID for the Current Application Characteristic
 */
#define CSR_OTA_CURRENT_APP_UUID_WORD1      0x0000
#define CSR_OTA_CURRENT_APP_UUID_WORD2      0x1013
#define CSR_OTA_CURRENT_APP_UUID_WORD3      0xd102
#define CSR_OTA_CURRENT_APP_UUID_WORD4      0x11e1
#define CSR_OTA_CURRENT_APP_UUID_WORD5      0x9b23
#define CSR_OTA_CURRENT_APP_UUID_WORD6      0x0002
#define CSR_OTA_CURRENT_APP_UUID_WORD7      0x5b00
#define CSR_OTA_CURRENT_APP_UUID_WORD8      0xa5a5

#define CSR_OTA_CURRENT_APP_UUID            0x00001013d10211e19b2300025b00a5a5

/* *****************************************************************************
 * This is the UUID for the Read CS Block Characteristic 
 */
#define CSR_OTA_READ_CS_BLOCK_UUID_WORD1    0x0000
#define CSR_OTA_READ_CS_BLOCK_UUID_WORD2    0x1018
#define CSR_OTA_READ_CS_BLOCK_UUID_WORD3    0xd102
#define CSR_OTA_READ_CS_BLOCK_UUID_WORD4    0x11e1
#define CSR_OTA_READ_CS_BLOCK_UUID_WORD5    0x9b23
#define CSR_OTA_READ_CS_BLOCK_UUID_WORD6    0x0002
#define CSR_OTA_READ_CS_BLOCK_UUID_WORD7    0x5b00
#define CSR_OTA_READ_CS_BLOCK_UUID_WORD8    0xa5a5

#define CSR_OTA_READ_CS_BLOCK_UUID          0x00001018d10211e19b2300025b00a5a5

/* *****************************************************************************
 * This is the UUID for the Data Transfer Characteristic
 */
#define CSR_OTA_DATA_TRANSFER_UUID_WORD1    0x0000
#define CSR_OTA_DATA_TRANSFER_UUID_WORD2    0x1014
#define CSR_OTA_DATA_TRANSFER_UUID_WORD3    0xd102
#define CSR_OTA_DATA_TRANSFER_UUID_WORD4    0x11e1
#define CSR_OTA_DATA_TRANSFER_UUID_WORD5    0x9b23
#define CSR_OTA_DATA_TRANSFER_UUID_WORD6    0x0002
#define CSR_OTA_DATA_TRANSFER_UUID_WORD7    0x5b00
#define CSR_OTA_DATA_TRANSFER_UUID_WORD8    0xa5a5

#define CSR_OTA_DATA_TRANSFER_UUID          0x00001014d10211e19b2300025b00a5a5

/* *****************************************************************************
 * This is the UUID for the Version Characteristic 
 */
#define CSR_OTA_VERSION_UUID_WORD1          0x0000
#define CSR_OTA_VERSION_UUID_WORD2          0x1011
#define CSR_OTA_VERSION_UUID_WORD3          0xd102
#define CSR_OTA_VERSION_UUID_WORD4          0x11e1
#define CSR_OTA_VERSION_UUID_WORD5          0x9b23
#define CSR_OTA_VERSION_UUID_WORD6          0x0002
#define CSR_OTA_VERSION_UUID_WORD7          0x5b00
#define CSR_OTA_VERSION_UUID_WORD8          0xa5a5

#define CSR_OTA_VERSION_UUID            	0x00001011d10211e19b2300025b00a5a5

/* OTA Update Application Service protocol version */
#define OTA_VERSION                         6

//------------------------------------------------------------------------------
// UUIDs for serial service
#define UUID_SERIAL_SERVICE            0x00005500D10211E19B2300025B00A5A5
#define UUID_SERIAL_DATA_TRANSFER      0x00005501D10211E19B2300025B00A5A5
#define UUID_SERIAL_DATA_ATDR          0x00005502D10211E19B2300025B00A5A5

/*Split the 128bit UUID into 8-bit*/
#define UUID_SERIAL_SERVICE_1          0x00
#define UUID_SERIAL_SERVICE_2          0x00
#define UUID_SERIAL_SERVICE_3          0x55
#define UUID_SERIAL_SERVICE_4          0x00
#define UUID_SERIAL_SERVICE_5          0xd1
#define UUID_SERIAL_SERVICE_6          0x02
#define UUID_SERIAL_SERVICE_7          0x11
#define UUID_SERIAL_SERVICE_8          0xe1
#define UUID_SERIAL_SERVICE_9          0x9b
#define UUID_SERIAL_SERVICE_10         0x23
#define UUID_SERIAL_SERVICE_11         0x00
#define UUID_SERIAL_SERVICE_12         0x02
#define UUID_SERIAL_SERVICE_13         0x5b
#define UUID_SERIAL_SERVICE_14         0x00
#define UUID_SERIAL_SERVICE_15         0xa5
#define UUID_SERIAL_SERVICE_16         0xa5

#endif /* _CSR_OTA_UUIDS_H */
