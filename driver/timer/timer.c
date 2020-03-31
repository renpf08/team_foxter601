#include <main.h>           /* Functions relating to powering up the device */
#include <ls_app_if.h>      /* Link Supervisor application interface */
#include <debug.h>          /* Simple host interface to the UART driver */
#include <timer.h>          /* Chip timer functions */
#include <panic.h>          /* Support for applications to panic */
#include "../driver.h"

/* Number of timers used in this application */
#define MAX_TIMERS 30

static uint16 app_timers[SIZEOF_APP_TIMER * MAX_TIMERS];

#if 0
typedef int (*timer_cb)(void *args);

typedef struct {
	unsigned ms;
	timer_cb cb;
}timer_cfg_t;

typedef struct {
	timer_cfg_t cfg[MAX_TIMERS];
}csr_timer_cfg_t;

static csr_timer_cfg_t csr_timer_cfg;
#endif

static timer_id csr_timer_create(uint32 timeout, timer_callback_arg handler)
{
    const timer_id tId = TimerCreate(timeout, TRUE, handler);
    
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID)
    {
        DebugWriteString("\r\nFailed to start timer");
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
	return tId;
}

static s16 csr_timer_start(u16 ms, timer_cb cb)
{
	csr_timer_create(ms*MILLISECOND, cb);
    return 0;
}

static s16 csr_timer_init(cfg_t *args, event_callback cb)
{
	TimerInit(MAX_TIMERS, (void *)app_timers);
	return 0;
}

static s16 csr_timer_unint(void)
{
	return 0;
}

timer_t csr_timer = {
	.timer_init = csr_timer_init,
	.timer_start = csr_timer_start,
	.timer_uninit = csr_timer_unint,
};
