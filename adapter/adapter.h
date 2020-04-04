#ifndef ADAPTER_H
#define ADAPTER_H

#include "../common/common.h"
#include "../driver/driver.h"

s16 adapter_init(adapter_callback cb);
s16 adapter_uninit(void);


clock_t *clock_get(void);
s16 clock_set(clock_t *ck);

#endif
