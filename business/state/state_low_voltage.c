#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

s16 state_low_voltage(REPORT_E cb, void *args)
{
	STATE_E *state = (STATE_E *)args;

	u8 string[13] = {'s', 't', 'a', 't', 'e', '_', 'l', 'o', 'w', '_', 'b', 'a', 't'};
	print(string, 13);

	*state = CLOCK;
	return 0;
}
