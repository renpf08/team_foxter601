#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

typedef enum {
	NOTIFY_RECV_CALL,
	NOTIFY_RECV_SMS,
	NOTIFY_RECV_EMAIL,
	NOTIFY_RECV_QQ,
	NOTIFY_RECV_WECHAT,
	NOTIFY_RECV_FACEBOOK,
	NOTIFY_RECV_FACEMESSAGE,
	NOTIFY_RECV_LINE,
	NOTIFY_RECV_SKYPE,
	NOTIFY_RECV_TWITTER,
	NOTIFY_RECV_WHATSAPP,
	NOTIFY_RECV_CALENDER,
	NOTIFY_RECV_LINKIN,
	NOTIFY_RECV_MAX,
}NOTIFY_RECV_E;

typedef struct {
    NOTIFY_RECV_E msg;
    NOTIFY_E idx;
} notify_convert_t;

static const notify_convert_t notif_convert_list[] =
{
    {NOTIFY_RECV_CALL,          NOTIFY_COMMING_CALL},
    {NOTIFY_RECV_SMS,           NOTIFY_SMS},
    {NOTIFY_RECV_EMAIL,         NOTIFY_EMAIL},
    {NOTIFY_RECV_QQ,            NOTIFY_QQ},
    {NOTIFY_RECV_WECHAT,        NOTIFY_WECHAT},
    {NOTIFY_RECV_FACEBOOK,      NOTIFY_FACEBOOK},
    {NOTIFY_RECV_FACEMESSAGE,   NOTIFY_FACEBOOK},
    {NOTIFY_RECV_LINE,          NOTIFY_LINE},
    {NOTIFY_RECV_SKYPE,         NOTIFY_SKYPE},
    {NOTIFY_RECV_TWITTER,       NOTIFY_TWITTER},
    {NOTIFY_RECV_WHATSAPP,      NOTIFY_WHATSAPP},
    {NOTIFY_RECV_CALENDER,      NOTIFY_CALENDER},
    {NOTIFY_RECV_LINKIN,        NOTIFY_LINKIN},
};
s16 state_notify(REPORT_E cb, void *args)
{
	STATE_E *state = (STATE_E *)args;
	app_msg_t *ancs_msg = NULL;

    if(cb == ANCS_NOTIFY_INCOMING) {
        ancs_msg = ancs_get();
    } else if(cb == ANDROID_NOTIFY) {
        ancs_msg = &cmd_get()->recv_notif;
        //print_str_dec((u8*)&"android type=", (u16)ancs_msg->type);
        ancs_msg->type = (u16)notif_convert_list[ancs_msg->type].idx;
    }

    //print_str_dec((u8*)&"notify type=", (u16)ancs_msg->type);
    //print_str_dec((u8*)&"notify sta=", (u16)ancs_msg->sta);
    
	if(NOTIFY_ADD == ancs_msg->sta) {
		if(ancs_msg->type < NOTIFY_DONE) {
			motor_notify_to_position(ancs_msg->type);
		}
	}else if(NOTIFY_REMOVE == ancs_msg->sta) {
	    //print((u8*)&"clear", 5);
		motor_notify_to_position(NOTIFY_NONE);
	}

	*state = CLOCK;
	return 0;
}
