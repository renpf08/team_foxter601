#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../driver.h"

typedef struct {
	pin_t pin;
	event_callback keym_cb;
	EVENT_E last_state;
}csr_keym_cfg_t;

static csr_keym_cfg_t csr_keym_cfg = {
	.pin = {
				.group = 0,
				.num = 0,
           },
	.keym_cb = NULL,
	.last_state = KEY_A_UNKNOWN,
};

s16 csr_keym_event_handler(u16 key_num, u16 key_status)
{
	EVENT_E now_state;
	if(key_num & (1UL << csr_keym_cfg.pin.num)) {
		if(key_status & (1 << csr_keym_cfg.pin.num)) {
			now_state = KEY_A_UP;
		} else {
			now_state = KEY_A_DOWN;
		}

		if(csr_keym_cfg.last_state == now_state) {
			return 0;
		} else {
			if(NULL != csr_keym_cfg.keym_cb) {
				csr_keym_cfg.keym_cb(now_state);
			}
			csr_keym_cfg.last_state = now_state;
		}
	}
	return 0;
}

static s16 csr_keym_init(cfg_t *args, event_callback cb)
{
	csr_keym_cfg.pin.group = args->keym_cfg.group;
	csr_keym_cfg.pin.num = args->keym_cfg.num;
	csr_keym_cfg.keym_cb = cb;
	
    /* Configure button to be controlled directly */
    PioSetMode(csr_keym_cfg.pin.num, pio_mode_user);
    
    /* Configure button to be input */
    PioSetDir(csr_keym_cfg.pin.num, PIO_DIR_INPUT);
    
    /* Set weak pull up on button PIO, in order not to draw too much current
     * while button is pressed
     */
    PioSetPullModes((1UL << csr_keym_cfg.pin.num), pio_mode_weak_pull_up);

    /* Set the button to generate sys_event_pio_changed when pressed as well
     * as released
     */
    PioSetEventMask((1UL << csr_keym_cfg.pin.num), pio_event_mode_both);
	return 0;
}

static s16 csr_keym_uninit(void)
{
	csr_keym_cfg.keym_cb = NULL;
	return 0;
}

key_t csr_keym = {
	.key_init = csr_keym_init,
	.key_uninit = csr_keym_uninit,
};


