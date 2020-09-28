#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../adapter.h"
#include "../../common/common.h"

#include "../../log.h"

s16 vib_init(adapter_callback cb);

typedef struct {
	driver_t *drv;
	adapter_callback cb;
	u8 run_flag;
	u8 status;
	u8 step_count;
	s16 tid;
}vib_cfg_t;

static vib_cfg_t vib_cfg = {
	.drv = NULL,
	.cb = NULL,
	.run_flag = 0,
	.status = idle,
	.step_count = 0,
	.tid = TIMER_INVALID,
};

static void vib_cb_handler(u16 id)
{
    vib_cfg.tid = TIMER_INVALID;
    
	if(0 == vib_cfg.run_flag) {
		return;
	}

	if(run == vib_cfg.status) {
		vib_cfg.drv->vibrator->vibrator_on(NULL);
		vib_cfg.status = idle;
	    vib_cfg.tid = vib_cfg.drv->timer->timer_start(200, vib_cb_handler);
	}else {		
		vib_cfg.drv->vibrator->vibrator_off(NULL);
        _vib_log_.cur_step--;
		//if run complete, then exit without have timer start again
		if(0 == --vib_cfg.step_count) {
			vib_cfg.run_flag = 0;
			vib_cfg.status = idle;
            _vib_log_.run_flag = 0;
			return;
		}else {
			vib_cfg.status = run;
	        vib_cfg.tid = vib_cfg.drv->timer->timer_start(800, vib_cb_handler);
		}
        log_send((u8*)&_vib_log_);
	}
}

s16 vib_stop(void)
{
    vib_cfg.drv->timer->timer_del(vib_cfg.tid);
    vib_cfg.tid = TIMER_INVALID;
	vib_cfg.status = idle;
	vib_cfg.run_flag = 0;
	vib_cfg.step_count = 0;
	vib_cfg.drv->vibrator->vibrator_off(NULL);
	return 0;
}

s16 vib_run(u8 step_count, u8 caller)
{
    _vib_log_.caller = caller;
    _vib_log_.steps = step_count;
    _vib_log_.cur_step = step_count;
    _vib_log_.run_flag = 1;
    
	vib_cfg.status = run;
	vib_cfg.run_flag = 1;
	vib_cfg.step_count = step_count;	
	vib_cfg.drv->vibrator->vibrator_on(NULL);
	//return 0;
	if(vib_cfg.step_count > 0) {
		vib_cfg.tid = vib_cfg.drv->timer->timer_start(10, vib_cb_handler);
	}
	
	return 0;
}

u8 vib_state(void)
{
    return vib_cfg.run_flag;
}

s16 vib_init(adapter_callback cb)
{
	vib_cfg.cb = cb;
	vib_cfg.drv = get_driver();	
	vib_cfg.drv->vibrator->vibrator_off(NULL);
	return 0;
}
