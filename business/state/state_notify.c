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
	NOTIFY_RECV_FACEMESSENGE,
	NOTIFY_RECV_LINE,
	NOTIFY_RECV_SKYPE,
	NOTIFY_RECV_TWITTER,
	NOTIFY_RECV_WHATSAPP,
	NOTIFY_RECV_CALENDER,
	NOTIFY_RECV_LINKIN,
	NOTIFY_RECV_MAX,
}NOTIFY_RECV_E;

typedef struct {
    NOTIFY_RECV_E recv_msg;
    NOTIFY_E disp_msg;
} notify_convert_t;

static const notify_convert_t notif_convert_list[] =
{
    {NOTIFY_RECV_CALL,          NOTIFY_COMMING_CALL},
    {NOTIFY_RECV_SMS,           NOTIFY_SMS},
    {NOTIFY_RECV_EMAIL,         NOTIFY_EMAIL},
    {NOTIFY_RECV_QQ,            NOTIFY_QQ},
    {NOTIFY_RECV_WECHAT,        NOTIFY_WECHAT},
    {NOTIFY_RECV_FACEBOOK,      NOTIFY_FACEBOOK},
    {NOTIFY_RECV_FACEMESSENGE,  NOTIFY_FACEBOOK},
    {NOTIFY_RECV_LINE,          NOTIFY_LINE},
    {NOTIFY_RECV_SKYPE,         NOTIFY_SKYPE},
    {NOTIFY_RECV_TWITTER,       NOTIFY_TWITTER},
    {NOTIFY_RECV_WHATSAPP,      NOTIFY_WHATSAPP},
    {NOTIFY_RECV_CALENDER,      NOTIFY_CALENDER},
    {NOTIFY_RECV_LINKIN,        NOTIFY_LINKIN},
    {0xFF,                      0xFF},
};

s16 state_notify(REPORT_E cb, void *args)
{
	STATE_E *state = (STATE_E *)args;
	app_msg_t *ancs_msg = NULL;
    volatile u8 msg_idx = 0xFF;
    u8 i = 0;
    u8 *en = cmd_get()->notify_switch.en;
    u32 msg_en = 0;

    for(i = 0; i < 4; i++) {
        msg_en  <<= 8;
        msg_en |= en[3-i];
    }
    if(cb == ANCS_NOTIFY_INCOMING) {
        ancs_msg = ancs_get();
    } else if(cb == ANDROID_NOTIFY) {
        ancs_msg = &cmd_get()->recv_notif;
        ancs_msg->type = (u16)notif_convert_list[ancs_msg->type].disp_msg;
    }
    i = 0;
    while(notif_convert_list[i].disp_msg != 0xFF) {
        if(notif_convert_list[i].disp_msg == ancs_msg->type) {
            msg_idx = notif_convert_list[i].recv_msg;
            break;
        }
        i++;
    }
    if(((msg_idx != 0xFF) && (msg_en & (1UL<<msg_idx)) == 0) || 
        (msg_idx == 0xFF)) {
        BLE_SEND_LOG((u8*)&"off", 3);
    	*state = CLOCK;
    	return 0;
    }
	if(NOTIFY_ADD == ancs_msg->sta) {
		if(ancs_msg->type < NOTIFY_DONE) {
			motor_notify_to_position(ancs_msg->type);
		}
	}else if(NOTIFY_REMOVE == ancs_msg->sta) {
		motor_notify_to_position(NOTIFY_NONE);
	}
    BLE_SEND_LOG((u8*)ancs_msg, sizeof(app_msg_t));

	*state = CLOCK;
	return 0;
}

#if 0
void notify_test(u16 id)
{
	state_notify(ANCS_NOTIFY_INCOMING, NULL);
	timer_event(1000, notify_test);
}
#endif
