#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../adapter.h"

typedef struct {
	clock_t ck;
	
};

staic clock_t clock = {
	.week = ,
	.day = 0,
	.hour = 0,
	.minute = 0,
	.second = 0,
};

s16 clock_init()
{

}



