#include <types.h>
#include <pio.h>
#include <pio_ctrlr.h>
#include "../driver.h"

typedef 

static s16 csr_gsensor_read(void *args)
{
	
}

static s16 csr_gsensor_init(cfg_t *args, event_callback cb)
{

}

static s16 csr_gsensor_uninit(void)
{

}

gsensor_t csr_gsensor = {
	.gsensor_init = csr_gsensor_init,
	.gsensor_read = csr_gsensor_read,
	.gsensor_uninit = csr_gsensor_uninit,
};
