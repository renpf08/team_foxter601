#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <panic.h>          /* Support for applications to panic */
#include <timer.h>          /* Chip timer functions */
#include "../driver.h"

static void csr_keyb_timer_create(uint32 timeout, timer_callback_arg handler);

#define KEYB_VAL_GET(num)        PioGet(num)

typedef struct {
	pin_t pin;
	event_callback keyb_cb;
	EVENT_E last_state;	
	EVENT_E now_state;
}csr_keyb_cfg_t;

static csr_keyb_cfg_t csr_keyb_cfg = {
	.pin = {
				.group = 0,
				.num = 0,
           },
	.keyb_cb = NULL,
	.last_state = KEY_B_UP,
};

static void csr_keyb_timer_create(uint32 timeout, timer_callback_arg handler)
{
    const timer_id tId = TimerCreate(timeout, TRUE, handler);
    
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID)
    {
        DebugWriteString("\r\nFailed to start timer");
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
}

static void csr_keyb_event_recheck(u16 id)
{
	if((true == KEYB_VAL_GET(csr_keyb_cfg.pin.num)) &&
		(KEY_B_UP == csr_keyb_cfg.now_state)) {
		csr_keyb_cfg.now_state = KEY_B_UP;
	}else if((false == KEYB_VAL_GET(csr_keyb_cfg.pin.num)) &&
		(KEY_B_DOWN == csr_keyb_cfg.now_state)) {
		csr_keyb_cfg.now_state = KEY_B_DOWN;
	}else {
		return;
	}

	if(csr_keyb_cfg.last_state == csr_keyb_cfg.now_state) {
		return;
	} else {
		if(NULL != csr_keyb_cfg.keyb_cb) {
			csr_keyb_cfg.keyb_cb(csr_keyb_cfg.now_state);
		}
		csr_keyb_cfg.last_state = csr_keyb_cfg.now_state;
	}
}

s16 csr_keyb_event_handler(u32 key_num, u32 key_status)
{
	if(key_num & (0x01UL << csr_keyb_cfg.pin.num)) {
		if(key_status & (0x01UL << csr_keyb_cfg.pin.num)) {
			csr_keyb_cfg.now_state = KEY_B_UP;
			csr_keyb_timer_create(10*MILLISECOND, csr_keyb_event_recheck);
		} else {
			csr_keyb_cfg.now_state = KEY_B_DOWN;
			csr_keyb_timer_create(10*MILLISECOND, csr_keyb_event_recheck);
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
