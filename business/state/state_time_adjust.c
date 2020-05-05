#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

s16 state_time_adjust(REPORT_E cb, void *args)
{
	u8 string[17] = {'s', 't', 'a', 't', 'e', '_', 't', 'i', 'm', 'e', '_', 'a', 'd', 'j', 'u', 't'};
	print(string, 17);


	return 0;
}
