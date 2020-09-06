#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../adapter.h"

s16 charge_status_init(adapter_callback cb);

typedef struct {
	driver_t *drv;
	adapter_callback cb;
	s16 tid;
	u8 last_status;
	u8 now_status;
}charge_status_cfg_t;

static charge_status_cfg_t charge_status_cfg = {
	.drv = NULL,
	.cb = NULL,
	.tid = TIMER_INVALID,
	.last_status = not_incharge,
	.now_status = not_incharge,
};

s16 charge_status_get(void)
{
	if((incharge == charge_status_cfg.last_status) &&
	   (incharge == charge_status_cfg.now_status)) {
		return incharge;
	}
	
	return not_incharge;
}

static void charge_status_cb_handler(u16 id)
{
	charge_status_cfg.tid = TIMER_INVALID;	
	charge_status_cfg.last_status = charge_status_cfg.now_status;
	charge_status_cfg.now_status = charge_status_cfg.drv->charge->charge_status_read(NULL);
	charge_status_cfg.tid = charge_status_cfg.drv->timer->timer_start(1000, charge_status_cb_handler);
}

s16 charge_status_init(adapter_callback cb)
{
	charge_status_cfg.drv = get_driver();
	charge_status_cfg.cb = cb;
	charge_status_cfg.tid = charge_status_cfg.drv->timer->timer_start(1, charge_status_cb_handler);
	return 0;
}
