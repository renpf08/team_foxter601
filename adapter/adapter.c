#include "config.h"
#include "../driver/driver.h"
#include "adapter.h"

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

	return 0;
}

s16 adapter_init(adapter_callback cb)
{
	u8 test[20] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA};

	adapter.drv = get_driver();
	adapter.cb = cb;
	//timer init
	adapter.drv->timer->timer_init(NULL, NULL);
	adapter.drv->battery->battery_init(NULL, NULL);
	adapter.drv->keya->key_init(&args, csr_event_callback);
	adapter.drv->flash->flash_init(NULL, NULL);

	//uart init and test
	adapter.drv->uart->uart_init(&args, NULL);
	adapter.drv->uart->uart_write(test, 12);

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

s16 adapter_uninit()
{
	//timer init
	adapter.drv->timer->timer_uninit();
	adapter.drv->battery->battery_uninit();
	adapter.drv->keya->key_uninit();
	adapter.drv->flash->flash_uninit();

	//uart init and test
	adapter.drv->uart->uart_uninit();

	//vibrator init	
	adapter.drv->vibrator->vibrator_off(NULL);
	adapter.drv->vibrator->vibrator_uninit();

	//gsensor init
	adapter.drv->gsensor->gsensor_uninit();

	//magnetometer init
	adapter.drv->magnetometer->magnetometer_uninit();

	//motor init and off
	adapter.drv->motor_hour->motor_uninit();
	adapter.drv->motor_minute->motor_uninit();
	adapter.drv->motor_activity->motor_uninit();
	adapter.drv->motor_date->motor_uninit();
	adapter.drv->motor_battery_week->motor_uninit();
	adapter.drv->motor_notify->motor_uninit();

	adapter.drv = NULL;
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
