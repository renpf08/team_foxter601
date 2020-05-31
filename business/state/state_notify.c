#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

s16 state_notify(REPORT_E cb, void *args)
{
	STATE_E *state = (STATE_E *)args;
	app_msg_t *ancs_msg = NULL;

    if(cb == ANCS_NOTIFY_INCOMING) {
        ancs_msg = ancs_get();
    } else if(cb == ANDROID_NOTIFY) {
        ancs_msg = &cmd_get()->recv_notif;
    }

    print_str_dec((u8*)&"notify type=", (u16)ancs_msg->type);
    print_str_dec((u8*)&"notify sta=", (u16)ancs_msg->sta);
    
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
