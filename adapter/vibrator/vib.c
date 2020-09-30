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

LOG_SEND_VIB_STATE_VARIABLE_DEF(log_send, log_send_vib_info_t, LOG_CMD_SEND, LOG_SEND_VIB_STATE);

static void vib_cb_handler(u16 id)
{
    LOG_SEND_VIB_STATE_VALUE_SET(log_send.vib_en, get_vib_en());
    vib_cfg.tid = TIMER_INVALID;
    
	if(0 == vib_cfg.run_flag) {
		return;
	}

	if(run == vib_cfg.status) {
        if(LOG_SEND_VIB_STATE_VALUE_COMPARE(log_send.vib_en, 1)) {
		    vib_cfg.drv->vibrator->vibrator_on(NULL);
        }
		vib_cfg.status = idle;
	    vib_cfg.tid = vib_cfg.drv->timer->timer_start(200, vib_cb_handler);
	}else {		
        if(LOG_SEND_VIB_STATE_VALUE_COMPARE(log_send.vib_en, 1)) {
		    vib_cfg.drv->vibrator->vibrator_off(NULL);
        }
        LOG_SEND_VIB_STATE_VALUE_SUB(log_send.cur_step);
		//if run complete, then exit without have timer start again
		if(0 == --vib_cfg.step_count) {
			vib_cfg.run_flag = 0;
			vib_cfg.status = idle;
            LOG_SEND_VIB_STATE_VALUE_SET(log_send.run_flag, 0);
			return;
		}else {
			vib_cfg.status = run;
	        vib_cfg.tid = vib_cfg.drv->timer->timer_start(800, vib_cb_handler);
		}
        LOG_SEND_VIB_STATE_VALUE_SEND(log_send.head);
	}
}

s16 vib_stop(void)
{
    vib_cfg.drv->timer->timer_del(vib_cfg.tid);
    vib_cfg.tid = TIMER_INVALID;
	vib_cfg.status = idle;
	vib_cfg.run_flag = 0;
	vib_cfg.step_count = 0;
    if(LOG_SEND_VIB_STATE_VALUE_COMPARE(log_send.vib_en, 1)) {	
	    vib_cfg.drv->vibrator->vibrator_off(NULL);
    }
	return 0;
}

s16 vib_run(u8 step_count, u8 caller)
{
    LOG_SEND_VIB_STATE_VALUE_SET(log_send.caller, caller);
    LOG_SEND_VIB_STATE_VALUE_SET(log_send.steps, step_count);
    LOG_SEND_VIB_STATE_VALUE_SET(log_send.cur_step, step_count);
    LOG_SEND_VIB_STATE_VALUE_SET(log_send.run_flag, 1);
    LOG_SEND_VIB_STATE_VALUE_SET(log_send.vib_en, get_vib_en());
    
	vib_cfg.status = run;
	vib_cfg.run_flag = 1;
	vib_cfg.step_count = step_count;
    if(LOG_SEND_VIB_STATE_VALUE_COMPARE(log_send.vib_en, 1)) {	
	    vib_cfg.drv->vibrator->vibrator_on(NULL);
    }
    LOG_SEND_VIB_STATE_VALUE_SEND(log_send.head);
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
