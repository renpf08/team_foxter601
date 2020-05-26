#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

s16 state_time_adjust(REPORT_E cb, void *args)
{
	print((u8 *)&"time_adjust", 11);


	return 0;
}
