#include <gatt.h>
#include <gatt_prim.h>
#include <battery.h>
#include "../driver.h"

static s16 csr_battery_voltage_read(void *args)
{
	return BatteryReadVoltage();
}

static s16 csr_battery_init(cfg_t *args, event_callback cb)
{
	return 0;
}

static s16 csr_battery_uninit(void)
{
	return 0;
}

battery_t csr_battery = {
	.battery_init = csr_battery_init,
	.battery_voltage_read = csr_battery_voltage_read,
	.battery_uninit = csr_battery_uninit,
};
