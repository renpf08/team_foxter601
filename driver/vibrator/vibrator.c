#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../driver.h"

#define VIBRATOR_HIGH(num) PioSets(1UL << num, 1UL)
#define VIBRATOR_LOW(num) PioSets(1UL << num, 0UL)

typedef struct {
	pin_t vib_pin;
	
}csr_vibrator_cfg_t;

static csr_vibrator_cfg_t csr_vibrator_cfg = {
	.vib_pin = {
		.group = 0,
		.num = 0,
	},
};

static s16 csr_vibrator_on(void *args)
{
	VIBRATOR_HIGH(csr_vibrator_cfg.vib_pin.num);
	return 0;
}

static s16 csr_vibrator_off(void *args)
{
	VIBRATOR_LOW(csr_vibrator_cfg.vib_pin.num);
	return 0;
}

static s16 csr_vibrator_init(cfg_t *args, event_callback cb)
{
	csr_vibrator_cfg.vib_pin.group = args->vibrator_cfg.group;
	csr_vibrator_cfg.vib_pin.num = args->vibrator_cfg.num;
	
	PioSetModes(1UL << csr_vibrator_cfg.vib_pin.num, pio_mode_user);
	PioSetDir(csr_vibrator_cfg.vib_pin.num, PIO_DIR_OUTPUT);
	PioSetPullModes(1UL << csr_vibrator_cfg.vib_pin.num, pio_mode_strong_pull_up);
	
	return 0;
}

static s16 csr_vibrator_uninit(void)
{
	csr_vibrator_cfg.vib_pin.group = 0;
	csr_vibrator_cfg.vib_pin.num = 0;
	return 0;
}

vibrator_t csr_vibrator = {
	.vibrator_init = csr_vibrator_init,
	.vibrator_on = csr_vibrator_on,
	.vibrator_off = csr_vibrator_off,
	.vibrator_uninit = csr_vibrator_uninit,
};
