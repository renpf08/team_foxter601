#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

s16 state_notify(REPORT_E cb, void *args)
{
	STATE_E *state = (STATE_E *)args;
	ancs_msg_t *ancs_msg = ancs_get();

	u8 string[12] = {'s', 't', 'a', 't', 'e', '_', 'n', 'o', 't', 'i', 'f', 'y'};
	print(string, 12);

	if(NOTIFY_ADD == ancs_msg->sta) {
		if(ancs_msg->type < NOTIFY_DONE) {
			motor_notify_to_position(ancs_msg->type);
		}
	}else if(NOTIFY_REMOVE == ancs_msg->sta) {
		motor_notify_to_position(NOTIFY_NONE);
	}

	*state = CLOCK;
	return 0;
}
