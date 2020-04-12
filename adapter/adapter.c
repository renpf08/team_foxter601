#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "config.h"
#include "adapter.h"

//callback handler
extern s16 button_A_down_cb_handler(void *args);
extern s16 button_A_up_cb_handler(void *args);
extern s16 button_M_down_cb_handler(void *args);
extern s16 button_M_up_cb_handler(void *args);
extern s16 button_B_down_cb_handler(void *args);
extern s16 button_B_up_cb_handler(void *args);
extern s16 mag_cb_handler(void *args);
extern s16 button_cb_handler(void *args);

//module init
extern s16 clock_init(void);

s16 csr_event_callback(EVENT_E ev);
void driver_uninit(void);

#if 0
static driver_callback_handler driver_cb[] = {
	[KEY_A_UP] =  button_A_up_cb_handler,
	[KEY_A_DOWN] = button_A_down_cb_handler,
	[KEY_B_UP] = button_B_up_cb_handler,
	[KEY_B_DOWN] = button_B_down_cb_handler,
	[KEY_M_UP] = button_M_up_cb_handler,
	[KEY_M_DOWN] = button_M_down_cb_handler,
	[MAGNETOMETER_READY] = mag_cb_handler,
};
#endif

typedef struct {
	driver_t *drv;
	adapter_callback cb;
}adapter_t;

static adapter_t adapter = {
	.drv = NULL,
	.cb = NULL,
};

s16 csr_event_callback(EVENT_E ev)
{
	if(ev >= EVENT_MAX) {
		return -1;
	}else {
		button_cb_handler((void*)ev);
	}
	
	return 0;
}

static s16 driver_init(void)
{
	//u8 test[25] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};

	adapter.drv = get_driver();
	//timer init
	adapter.drv->timer->timer_init(NULL, NULL);
	adapter.drv->battery->battery_init(NULL, NULL);
	adapter.drv->keya->key_init(&args, csr_event_callback);
	adapter.drv->keym->key_init(&args, csr_event_callback);
	adapter.drv->keyb->key_init(&args, csr_event_callback);	
	adapter.drv->flash->flash_init(NULL, NULL);

	//uart init and test
	adapter.drv->uart->uart_init(&args, NULL);
	//adapter.drv->uart->uart_write(test, 23);

	//vibrator init
	adapter.drv->vibrator->vibrator_init(&args, NULL);
	adapter.drv->vibrator->vibrator_off(NULL);

	//gsensor init
	adapter.drv->gsensor->gsensor_init(&args, NULL);

	//magnetometer init
	adapter.drv->magnetometer->magnetometer_init(&args, csr_event_callback);

	//motor init and off
	adapter.drv->motor_hour->motor_init(&args, NULL);
	adapter.drv->motor_hour->motor_stop(NULL);
	adapter.drv->motor_minute->motor_init(&args, NULL);
	adapter.drv->motor_minute->motor_stop(NULL);
	adapter.drv->motor_activity->motor_init(&args, NULL);
	adapter.drv->motor_activity->motor_stop(NULL);
	adapter.drv->motor_date->motor_init(&args, NULL);
	adapter.drv->motor_date->motor_stop(NULL);
	adapter.drv->motor_battery_week->motor_init(&args, NULL);
	adapter.drv->motor_battery_week->motor_stop(NULL);
	adapter.drv->motor_notify->motor_init(&args, NULL);
	adapter.drv->motor_notify->motor_stop(NULL);

	return 0;
}

void driver_uninit(void)
{
	//timer uninit
	adapter.drv->timer->timer_uninit();
	adapter.drv->battery->battery_uninit();
	adapter.drv->keya->key_uninit();
	adapter.drv->flash->flash_uninit();

	//uart uninit
	adapter.drv->uart->uart_uninit();

	//vibrator uninit	
	adapter.drv->vibrator->vibrator_off(NULL);
	adapter.drv->vibrator->vibrator_uninit();

	//gsensor uninit
	adapter.drv->gsensor->gsensor_uninit();

	//magnetometer uninit
	adapter.drv->magnetometer->magnetometer_uninit();

	//motor uninit
	adapter.drv->motor_hour->motor_uninit();
	adapter.drv->motor_minute->motor_uninit();
	adapter.drv->motor_activity->motor_uninit();
	adapter.drv->motor_date->motor_uninit();
	adapter.drv->motor_battery_week->motor_uninit();
	adapter.drv->motor_notify->motor_uninit();
	
	adapter.drv = NULL;
}

s16 adapter_init(adapter_callback cb)
{
	//driver init
	driver_init();
	adapter.cb = cb;

	//module init
	clock_init();

	return 0;
}

s16 adapter_uninit()
{
	driver_uninit();
	adapter.cb = NULL;
	return 0;
}

#if 0
u8 i = 0, j = 0;
for(i = 0; i < 10; i++) {
	driver->motor_hour->motor_positive_first_half(NULL);
	TimeDelayUSec(5 * MILLISECOND);
	driver->motor_hour->motor_stop(NULL);
	driver->motor_hour->motor_positive_second_half(NULL);
	TimeDelayUSec(5 * MILLISECOND);
	driver->motor_hour->motor_stop(NULL);
	
	for(j = 0; j < 20; j++) {
		TimeDelayUSec(50 * MILLISECOND);
	}
}

for(i = 0; i < 10; i++) {
	driver->motor_hour->motor_negtive_first_half(NULL);
	TimeDelayUSec(5 * MILLISECOND);
	driver->motor_hour->motor_stop(NULL);
	driver->motor_hour->motor_negtive_second_half(NULL);
	TimeDelayUSec(5 * MILLISECOND);
	driver->motor_hour->motor_stop(NULL);
	
	for(j = 0; j < 20; j++) {
		TimeDelayUSec(50 * MILLISECOND);
	}
}

driver->timer->timer_start(2000, csr_timer_cb);
m_printf("battery voltage[%d]\r\n",driver->battery->battery_voltage_read(NULL));
#endif
