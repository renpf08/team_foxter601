#include"../common/common.h"
#include"driver.h"

extern timer_t csr_timer;
extern uart_t csr_uart;

static driver_t csr_driver = {
	.timer = &csr_timer,
	.uart = &csr_uart,
};

driver_t *get_driver(void)
{
	return &csr_driver;
}
