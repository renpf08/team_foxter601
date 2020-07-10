/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      nvm_access.c
 *
 *  DESCRIPTION
 *      This file defines routines which the application uses to access
 *      NVM.
 *
 ******************************************************************************/
/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <pio.h>
#include <nvm.h>
#include <i2c.h>
#include <gatt.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "nvm_access.h"
#include "ancs_client_gatt.h"
#include "app_gatt.h"
#include "battery_service.h"
#include "user_config.h"

/*============================================================================*
 * Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      Nvm_Disable
 *
 *  DESCRIPTION
 *      This function is used to perform things necessary to save power on NVM
 *      once the read/write operations are done.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

void Nvm_Disable(void)
{
    NvmDisable();
    PioSetI2CPullMode(pio_i2c_pull_mode_strong_pull_down);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      Nvm_Read
 *
 *  DESCRIPTION
 *      Read words from the NVM Store after preparing the NVM to be readable.
 *      After the read operation, perform things necessary in application
 *      to save power on NVM.
 *
 *      Read words starting at the word offset, and store them in the supplied
 *      buffer.
 *
 *      \param buffer  The buffer to read words into.
 *      \param length  The number of words to read.
 *      \param offset  The word offset within the NVM Store to read from.
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/

extern void Nvm_Read(uint16* buffer, uint16 length, uint16 offset)
{
    sys_status result;
            
    if(CheckLowBatteryVoltage())
    {
        /* As the current voltage is below the threshold voltage,do not proceed 
         * with the NVM Read.
         * Notify the low battery event to the host if connected and return. 
         */        
        return;
    }

    /* NvmRead automatically enables the NVM before reading */
    result = NvmRead(buffer, length, offset);

    /* Disable NVM after reading/writing */
    Nvm_Disable();

    #if USE_PANIC_PRINT
    /* If NvmRead fails, report panic */
    if(sys_status_success != result)
    {
        ReportPanic(__FILE__, __func__, __LINE__, app_panic_nvm_read);
    }
    #endif
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      Nvm_Write
 *
 *  DESCRIPTION
 *      Write words to the NVM store after preparing the NVM to be writeable. 
 *      After the write operation, perform things necessary in application
 *      to save power on NVM.
 *
 *      Write words from the supplied buffer into the NVM Store, starting at
 *      the given word offset.
 *
 *      \param buffer  The buffer to write.
 *      \param length  The number of words to write.
 *      \param offset  The word offset within the NVM Store to write to.
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/

extern void Nvm_Write(uint16* buffer, uint16 length, uint16 offset)
{
    sys_status result;
    
    if(CheckLowBatteryVoltage())
    {
        /* As the current voltage is below the threshold voltage,do not proceed 
         * with the NVM Write.
         * Notify the low battery event to the host if connected and return. 
         */      
        return;
    }
    
    /* Write to NVM. Firmware re-enables the NVM if it is disabled */
    result = NvmWrite(buffer, length, offset);

    /* Disable NVM after reading/writing */
    Nvm_Disable();

    /* If NvmWrite was a success, return */
    if(sys_status_success == result)
    {
        /* Write was successful. */
        return;
    }
#ifdef NVM_TYPE_FLASH
    else if(nvm_status_needs_erase == result)
    {
        /* The application already has a copy of NVM data in its variables,
         * so we can erase the NVM 
         */
        Nvm_Erase();

        /* Write back the NVM data. 
         * Please note that the following function writes data into NVM and 
         * should not fail. 
         */
         WriteApplicationAndServiceDataToNVM();
    }
#endif /* NVM_TYPE_FLASH */
    else
    {
        /* Irrecoverable error. Reset the chip. */
        #if USE_PANIC_PRINT
        ReportPanic(__FILE__, __func__, __LINE__, app_panic_nvm_write);
        #endif
    }
}


#ifdef NVM_TYPE_FLASH
/*----------------------------------------------------------------------------*
 *  NAME
 *      Nvm_Write
 *
 *  DESCRIPTION
 *      Erases the NVM memory.
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
extern void Nvm_Erase(void)
{
    sys_status result;

    /* NvmErase automatically enables the NVM before erasing */
    result = NvmErase(TRUE);

    /* Disable NVM after erasing */
    Nvm_Disable();

    #if USE_PANIC_PRINT
    /* If NvmErase fails, report panic */
    if(sys_status_success != result)
    {
        ReportPanic(__FILE__, __func__, __LINE__, app_panic_nvm_erase);
    }
    #endif
}
#endif /* NVM_TYPE_FLASH */