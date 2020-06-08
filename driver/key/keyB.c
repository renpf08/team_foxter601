#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <panic.h>          /* Support for applications to panic */
#include <timer.h>          /* Chip timer functions */
#include "../driver.h"

static csr_key_cfg_t csr_keyb_cfg = {
	.pin = {
				.group = 0,
				.num = 0,
           },
	.key_cb = NULL,
	.last_state = KEY_B_UP,
};

static void csr_keyb_event_recheck(u16 id)
{
	if((true == VAL_GET(csr_keyb_cfg.pin.num)) &&
		(KEY_B_UP == csr_keyb_cfg.now_state)) {
		csr_keyb_cfg.now_state = KEY_B_UP;
	}else if((false == VAL_GET(csr_keyb_cfg.pin.num)) &&
		(KEY_B_DOWN == csr_keyb_cfg.now_state)) {
		csr_keyb_cfg.now_state = KEY_B_DOWN;
	}else {
		return;
	}

	if(csr_keyb_cfg.last_state == csr_keyb_cfg.now_state) {
		return;
	} else {
		if(NULL != csr_keyb_cfg.key_cb) {
			csr_keyb_cfg.key_cb(csr_keyb_cfg.now_state);
		}
		csr_keyb_cfg.last_state = csr_keyb_cfg.now_state;
	}
}

s16 csr_keyb_event_handler(u32 key_num, u32 key_status)
{
	if(key_num & (0x01UL << csr_keyb_cfg.pin.num)) {
		if(key_status & (0x01UL << csr_keyb_cfg.pin.num)) {
			csr_keyb_cfg.now_state = KEY_B_UP;
			timer_create(10*MILLISECOND, csr_keyb_event_recheck);
		} else {
			csr_keyb_cfg.now_state = KEY_B_DOWN;
			timer_create(10*MILLISECOND, csr_keyb_event_recheck);
		}
	}
	return 0;
}

static s16 csr_keyb_init(cfg_t *args, event_callback cb)
{
	csr_keyb_cfg.pin.group = args->keyb_cfg.group;
	csr_keyb_cfg.pin.num = args->keyb_cfg.num;
	csr_keyb_cfg.key_cb = cb;
	
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
	csr_keyb_cfg.key_cb = NULL;
	return 0;
}

key_t csr_keyb = {
	.key_init = csr_keyb_init,
	.key_uninit = csr_keyb_uninit,
};
