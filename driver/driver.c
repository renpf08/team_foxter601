#include"../common/common.h"
#include"driver.h"

extern timer_t csr_timer;

static driver_t csr_driver = {
	.timer = &csr_timer,
};

driver_t *get_driver(void)
{
	return &csr_driver;
}
