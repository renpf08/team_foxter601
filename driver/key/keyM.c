#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <panic.h>          /* Support for applications to panic */
#include <timer.h>          /* Chip timer functions */
#include "../driver.h"

static csr_key_cfg_t csr_keym_cfg = {
	.pin = {
				.group = 0,
				.num = 0,
           },
    .flag = 0,
	.key_cb = NULL,
	.last_state = KEY_M_UP,
};

static void csr_keym_event_recheck(u16 id)
{
	csr_keym_cfg.flag = 0;

	if((true == VAL_GET(csr_keym_cfg.pin.num)) &&
		(KEY_M_UP == csr_keym_cfg.now_state)) {
		csr_keym_cfg.now_state = KEY_M_UP;
	}else if((false == VAL_GET(csr_keym_cfg.pin.num)) &&
		(KEY_M_DOWN == csr_keym_cfg.now_state)) {
		csr_keym_cfg.now_state = KEY_M_DOWN;
	}else {
		return;
	}

	if(csr_keym_cfg.last_state == csr_keym_cfg.now_state) {
		return;
	} else {
		if(NULL != csr_keym_cfg.key_cb) {
			csr_keym_cfg.key_cb(csr_keym_cfg.now_state);
		}
		csr_keym_cfg.last_state = csr_keym_cfg.now_state;
	}
}

s16 csr_keym_event_handler(u32 key_num, u32 key_status)
{
	if(key_num & (0x01UL << csr_keym_cfg.pin.num)) {
		if(key_status & (0x01UL << csr_keym_cfg.pin.num)) {
			if(0 == csr_keym_cfg.flag) {
				csr_keym_cfg.flag = 1;			
				csr_keym_cfg.now_state = KEY_M_UP;
				timer_create(10*MILLISECOND, csr_keym_event_recheck, 0x14);
			}
		} else {
			if(0 == csr_keym_cfg.flag) {
				csr_keym_cfg.flag = 1;			
				csr_keym_cfg.now_state = KEY_M_DOWN;
				timer_create(10*MILLISECOND, csr_keym_event_recheck, 0x15);
			}
		}
	}
	return 0;
}

static s16 csr_keym_init(cfg_t *args, event_callback cb)
{
	csr_keym_cfg.pin.group = args->keym_cfg.group;
	csr_keym_cfg.pin.num = args->keym_cfg.num;
	csr_keym_cfg.key_cb = cb;
	
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

key_t csr_keym = {
	.key_init = csr_keym_init,
};
