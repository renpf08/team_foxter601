#include <gatt.h>
#include <gatt_prim.h>
#include <battery.h>
#include "../driver.h"

typedef struct {
	pin_t pin;
	event_callback cb;
}csr_charge_t;

static csr_charge_t csr_charge_cfg = {
	.pin = {
			.group = 0,
			.num = 0,
    	},
	.cb = NULL,
};

enum {
	low = 0,
	high = 1,
};

static s16 csr_charge_status_read(void *args)
{
	bool status = not_incharge;
	
    /* Set pio_mode_no_pulls on charge PIO, in order not to draw too much current*/
    PioSetPullModes((1UL << csr_charge_cfg.pin.num), pio_mode_strong_pull_up);
	if(low == PioGet(csr_charge_cfg.pin.num)) {
		status = incharge;
	}

    PioSetPullModes((1UL << csr_charge_cfg.pin.num), pio_mode_no_pulls);
	return status;
}

static s16 csr_charge_init(cfg_t *args, event_callback cb)
{
	csr_charge_cfg.pin.group = args->charge_cfg.group;
	csr_charge_cfg.pin.num = args->charge_cfg.num;
	csr_charge_cfg.cb = cb;

    /* Configure button to be controlled directly */
    PioSetMode(csr_charge_cfg.pin.num, pio_mode_user);
    
    /* Configure button to be input */
    PioSetDir(csr_charge_cfg.pin.num, PIO_DIR_INPUT);
    
    /* Set pio_mode_no_pulls on charge PIO, in order not to draw too much current*/
    PioSetPullModes((1UL << csr_charge_cfg.pin.num), pio_mode_no_pulls);
	return 0;
}

charge_t csr_charge = {
	.charge_init = csr_charge_init,
	.charge_status_read = csr_charge_status_read,
};
