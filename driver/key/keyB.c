#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../driver.h"

typedef struct {
	pin_t pin;
	event_callback keyb_cb;
	EVENT_E last_state;
}csr_keyb_cfg_t;

static csr_keyb_cfg_t csr_keyb_cfg = {
	.pin = {
				.group = 0,
				.num = 0,
           },
	.keyb_cb = NULL,
	.last_state = KEY_B_UP,
};

s16 csr_keyb_event_handler(u32 key_num, u32 key_status)
{
	EVENT_E now_state;
	if(key_num & (0x01UL << csr_keyb_cfg.pin.num)) {
		if(key_status & (0x01UL << csr_keyb_cfg.pin.num)) {
			now_state = KEY_B_UP;
		} else {
			now_state = KEY_B_DOWN;
		}

		if(csr_keyb_cfg.last_state == now_state) {
			return 0;
		} else {
			if(NULL != csr_keyb_cfg.keyb_cb) {
				csr_keyb_cfg.keyb_cb(now_state);
			}
			csr_keyb_cfg.last_state = now_state;
		}
	}
	return 0;
}

static s16 csr_keyb_init(cfg_t *args, event_callback cb)
{
	csr_keyb_cfg.pin.group = args->keyb_cfg.group;
	csr_keyb_cfg.pin.num = args->keyb_cfg.num;
	csr_keyb_cfg.keyb_cb = cb;
	
    /* Configure button to be controlled directly */
    PioSetMode(csr_keyb_cfg.pin.num, pio_mode_user);
    
    /* Configure button to be input */
    PioSetDir(csr_keyb_cfg.pin.num, PIO_DIR_INPUT);
    
    /* Set weak pull up on button PIO, in order not to draw too much current
     * while button is pressed
     */
    PioSetPullModes((1UL << csr_keyb_cfg.pin.num), pio_mode_weak_pull_up);

    /* Set the button to generate sys_event_pio_changed when pressed as well
     * as released
     */
    PioSetEventMask((1UL << csr_keyb_cfg.pin.num), pio_event_mode_both);
	return 0;
}

static s16 csr_keyb_uninit(void)
{
	csr_keyb_cfg.keyb_cb = NULL;
	return 0;
}

key_t csr_keyb = {
	.key_init = csr_keyb_init,
	.key_uninit = csr_keyb_uninit,
};
