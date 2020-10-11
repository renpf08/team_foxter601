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
    NOTIFY_RECV_E recv_msg; // no use
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
    u8 i = 0;
    u8 *en = cmd_get()->notify_switch.en;
    u32 msg_en = 0;
    LOG_SEND_NOTIFY_TYPE_VARIABLE_DEF(log_send, log_send_notify_t, LOG_CMD_SEND, LOG_SEND_NOTIFY_TYPE);

    for(i = 0; i < 4; i++) {
        msg_en  <<= 8;
        msg_en |= en[3-i];
    }
    if(cb == ANCS_NOTIFY_INCOMING) {
        ancs_msg = ancs_get();
    } else if(cb == ANDROID_NOTIFY) {
        ancs_msg = &cmd_get()->recv_notif;
        if(ancs_msg->type >= NOTIFY_MAX) ancs_msg->type = 0xFF;
        else ancs_msg->type = (u16)notif_convert_list[ancs_msg->type].disp_msg;
    }
    LOG_SEND_NOTIFY_TYPE_VALUE_SET(log_send.sta_report, cb);
    i = 0;
    while(notif_convert_list[i].disp_msg != 0xFF) {
        if(notif_convert_list[i].disp_msg == ancs_msg->type) {
            break;
        }
        i++;
    }
    LOG_SEND_NOTIFY_TYPE_VALUE_SET(log_send.msg_type, notif_convert_list[i].disp_msg);
    if(notif_convert_list[i].disp_msg >= NOTIFY_DONE) { // notify not use or out of range
        ancs_msg->sta = NOTIFY_RESERVE;
        LOG_SEND_NOTIFY_TYPE_VALUE_SET(log_send.result, 0xE1); // notify not use or out of range
    } else if((msg_en & (1UL<<i)) == 0) { // notify switch off
        ancs_msg->sta = NOTIFY_RESERVE;
        LOG_SEND_NOTIFY_TYPE_VALUE_SET(log_send.result, 0xE2); // notify switch off
    }
    if(ancs_msg->sta == NOTIFY_RESERVE) {
    	*state = CLOCK;
        LOG_SEND_NOTIFY_TYPE_VALUE_SEND(log_send.head);
    	return 0;
    }
    LOG_SEND_NOTIFY_TYPE_VALUE_SET(log_send.result, 0x01); // notify OK
	if(NOTIFY_ADD == ancs_msg->sta) {
		if(ancs_msg->type < NOTIFY_DONE) {
            #if USE_UART_PRINT
			print((u8 *)&ancs_msg->type, 1);
            #endif
			#if USE_ACTIVITY_NOTIFY
            motor_activity_to_position(ancs_msg->type);
            #else
			motor_notify_to_position(ancs_msg->type);
            #endif
		}
	}else if(NOTIFY_REMOVE == ancs_msg->sta) {
		#if USE_ACTIVITY_NOTIFY
        motor_activity_to_position(NOTIFY_NONE);
        #else
		motor_notify_to_position(NOTIFY_NONE);
        #endif
	}
    LOG_SEND_NOTIFY_TYPE_VALUE_SEND(log_send.head);
    vib_stop();
    vib_run(5, 0x03);

	*state = CLOCK;
	return 0;
}

#if 0
void notify_test_timeout(u16 id);
void notify_test_timeout(u16 id)
{
	motor_notify_to_position(NOTIFY_SMS);
}

void notify_test(void)
{
#if 0
	NOTIFY_NONE = 0,
	NOTIFY_SKYPE = 1,
	NOTIFY_WHATSAPP = 2,
	NOTIFY_TWITTER = 3,
	NOTIFY_EMAIL = 4,
	NOTIFY_FACEBOOK = 5,
	NOTIFY_SMS = 6,
	NOTIFY_LINKIN = 7,
	NOTIFY_COMMING_CALL = 8,
#endif
	motor_notify_to_position(NOTIFY_LINKIN);
	timer_event(2000, notify_test_timeout);
}
#endif
