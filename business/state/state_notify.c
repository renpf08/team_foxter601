#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

s16 state_notify(REPORT_E cb, void *args)
{
	STATE_E *state = (STATE_E *)args;

	u8 string[12] = {'s', 't', 'a', 't', 'e', '_', 'n', 'o', 't', 'i', 'f', 'y'};
	print(string, 12);

	
	motor_notify_to_position(cb);

	*state = CLOCK;
	return 0;
}
