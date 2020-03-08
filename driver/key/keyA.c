#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../driver.h"


typedef struct {
	pin_t pin;
	event_callback keya_cb;
}csr_keya_cfg_t;

static csr_keya_cfg_t csr_keya_cfg = {
	.pin = {
				.group = 0,
				.num = 0,
           },
	.keya_cb = NULL,
};

s16 csr_keya_event_handler(void)
{


	if(NULL != csr_keya_cfg.keya_cb) {
		csr_keya_cfg.keya_cb(KEY_A_UP);
	}
	return 0;
}

static s16 csr_keya_init(cfg_t *args, event_callback cb)
{
	csr_keya_cfg.pin.group = args->keya_cfg.group;
	csr_keya_cfg.pin.num = args->keya_cfg.num;
	csr_keya_cfg.keya_cb = cb;
	
    /* Configure button to be controlled directly */
    PioSetMode(csr_keya_cfg.pin.num, pio_mode_user);
    
    /* Configure button to be input */
    PioSetDir(csr_keya_cfg.pin.num, PIO_DIR_INPUT);
    
    /* Set weak pull up on button PIO, in order not to draw too much current
     * while button is pressed
     */
    PioSetPullModes((1UL << csr_keya_cfg.pin.num), pio_mode_weak_pull_up);

    /* Set the button to generate sys_event_pio_changed when pressed as well
     * as released
     */
    PioSetEventMask((1UL << csr_keya_cfg.pin.num), pio_event_mode_both);	
	return 0;
}

static s16 csr_keya_uninit(void)
{
	csr_keya_cfg.keya_cb = NULL;
	return 0;
}

key_t csr_keyA = {
	.key_init = csr_keya_init,
	.key_uninit = csr_keya_uninit,
};
