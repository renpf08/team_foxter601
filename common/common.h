#ifndef COMMON_H
#include COMMON_H

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


#endif
