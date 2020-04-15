#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <time.h>
#include "../adapter.h"

enum {
	FIRST_HALF,
	RECOVER,
	SECOND_HALF,
	STOP,
};

typedef struct {
	driver_t *drv;
	u8 hour_cur_pos;
	u8 minute_cur_pos;
	u8 date_cur_pos;
	u8 notify_cur_pos;
	u8 battery_week_cur_pos;
	u8 activity_cur_pos;
}motor_manager_t;

static motor_manager_t motor_manager = {
	.drv = NULL,
	.hour_cur_pos = 0,
	.minute_cur_pos = 0,
	.date_cur_pos = 0,
	.notify_cur_pos = 0,
	.battery_week_cur_pos = 0,
	.activity_cur_pos = 0,
};

static s16 motor_run_clockwise_one_step(void)
{

	switch() {
		case FIRST_HALF:
			break;
		case RECOVER:
			break;
		case SECOND_HALF:
			break;
		case STOP:
			break;
		default:
			break;
	}

	motor_manager.drv->timer->timer_start(2000, csr_timer_cb);

	u8 i = 0, j = 0;
	for(i = 0; i < 10; i++) {
		motor_manager.drv->motor_hour->motor_positive_first_half(NULL);
		TimeDelayUSec(5 * MILLISECOND);
		motor_manager.drv->motor_hour->motor_stop(NULL);
		motor_manager.drv->motor_hour->motor_positive_second_half(NULL);
		TimeDelayUSec(5 * MILLISECOND);
		motor_manager.drv->motor_hour->motor_stop(NULL);
		
		for(j = 0; j < 20; j++) {
			TimeDelayUSec(50 * MILLISECOND);
		}
	}
	return 0;
}

static s16 motor_run_anti_clockwise_one_step(void)
{
	for(i = 0; i < 10; i++) {
		motor_manager.drv->motor_hour->motor_negtive_first_half(NULL);
		TimeDelayUSec(5 * MILLISECOND);
		motor_manager.drv->motor_hour->motor_stop(NULL);
		motor_manager.drv->motor_hour->motor_negtive_second_half(NULL);
		TimeDelayUSec(5 * MILLISECOND);
		motor_manager.drv->motor_hour->motor_stop(NULL);
		
		for(j = 0; j < 20; j++) {
			TimeDelayUSec(50 * MILLISECOND);
		}
	}

	return 0;
}

s16 motor_manager_init(void)
{
	motor_manager.drv = get_driver();
	return 0;
}
