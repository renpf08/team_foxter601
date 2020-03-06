#ifndef COMMON_H
#define COMMON_H

#include "typedef.h"

typedef enum {
	KEY_A_UP,
	KEY_A_DOWN,
	KEY_B_UP,
	KEY_B_DOWN,
	KEY_M_UP,
	KEY_DOWN
}EVENT_E;

typedef struct {
	u8 group;
	u8 num;
}pin_t;

typedef struct {
	pin_t tx;
	pin_t rx;
}uart_cfg_t;

typedef struct {
	uart_cfg_t uart_cfg;

}cfg_t;

typedef int (*event_callback)(EVENT_E ev);

typedef int (*init)(void *args);
typedef int (*run)(void);
typedef int (*uninit)(void);
typedef int (*read)(void *args);
typedef int (*write)(void *args);

typedef void(*timer_cb)(u16 id);
typedef int (*timer_start_func)(unsigned ms, timer_cb cb);

#endif
