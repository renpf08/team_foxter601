#include"../common/common.h"
#include"driver.h"

extern timer_t csr_timer;
extern uart_t csr_uart;
extern battery_t csr_battery;
extern key_t csr_keyA;

static driver_t csr_driver = {
	.timer = &csr_timer,
	.uart = &csr_uart,
	.battery = &csr_battery,
	.keya = &csr_keyA,
};

driver_t *get_driver(void)
{
	return &csr_driver;
}
