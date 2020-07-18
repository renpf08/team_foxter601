#include <main.h>           /* Functions relating to powering up the device */
#include <ls_app_if.h>      /* Link Supervisor application interface */
#include <debug.h>          /* Simple host interface to the UART driver */
#include <timer.h>          /* Chip timer functions */
#include <panic.h>          /* Support for applications to panic */
#include "../driver.h"

/* Number of timers used in this application */
#define MAX_TIMERS 30

static uint16 app_timers[SIZEOF_APP_TIMER * MAX_TIMERS];

static s16 csr_timer_start(u16 ms, timer_cb cb)
{
	timer_create(ms*MILLISECOND, cb);
    return 0;
}

static s16 csr_timer_init(cfg_t *args, event_callback cb)
{
	TimerInit(MAX_TIMERS, (void *)app_timers);
	return 0;
}

timer_t csr_timer = {
	.timer_init = csr_timer_init,
	.timer_start = csr_timer_start,
};
