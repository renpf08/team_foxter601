#ifndef BUSINESS_H
#define BUSINESS_H

#include "../adapter/adapter.h"
#include "../common/common.h"

s16 business_init(void);
void pair_code_generate(void);
pair_code_t *pair_code_get(void);

#endif
