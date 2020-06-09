#include <mem.h>			/* Services of the memory library. */
#include <debug.h>          /* Simple host interface to the UART driver */
#include <nvm.h>
#include "../../nvm_access.h"
#include "../driver.h"

static s16 csr_flash_read(u16 *buffer, u16 length, u16 offset)
{
	Nvm_Read(buffer, length, offset);
	return 0;
}

static s16 csr_flash_write(u16 *buffer, u16 length, u16 offset)
{
	Nvm_Write(buffer, length, offset);
	return 0;
}

static s16 csr_flash_init(cfg_t *args, event_callback cb)
{
#ifdef NVM_TYPE_EEPROM
		/* Configure the NVM manager to use I2C EEPROM for NVM store */
		NvmConfigureI2cEeprom();
#elif NVM_TYPE_FLASH
		/* Configure the NVM Manager to use SPI flash for NVM store. */
		NvmConfigureSpiFlash();
#endif /* NVM_TYPE_EEPROM */

    Nvm_Disable();
	return 0;
}

flash_t csr_flash = {
	.flash_init = csr_flash_init,
	.flash_read = csr_flash_read,
	.flash_write = csr_flash_write,
};
