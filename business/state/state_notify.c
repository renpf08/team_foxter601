#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

/**
* u8 cmd; //! fixed to 0x07
* u8 sta; //! fixed to: 0:added, 1:modified, 2:removed
* u8 level; //! 0~255, look appMsgList[] of MESSAGE_POSITION_xxx for details
* u8 type; //! look appMsgList[] of APP_ID_STRING_xxx's index for details
* u8 cnt; //! msg count
* notify test command
* notify type    notify switch   notify response         android cmd
* Skype:         04 00 01 00 00  07 xx xx 08 xx          07 00 00 08 01
* WhatsApp:      04 00 04 00 00  07 xx xx 0A xx          07 00 00 0A 01
* Twitter:       04 00 02 00 00  07 xx xx 09 xx          07 00 00 09 01
* email:         04 04 00 00 00  07 xx xx 02 xx          07 00 00 02 01
* Facebook:      04 20 00 00 00  07 xx xx 05 xx          07 00 00 05 01
* sms:           04 02 00 00 00  07 xx xx 01 xx          07 00 00 01 01
* linkedin:      04 00 10 00 00  07 xx xx 0C xx          07 00 00 0C 01
* call:          04 01 00 00 00  07 xx xx 00 xx          07 00 00 00 01
*/

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
    u8 log[6] = {0x5F, 0x03, 0, 0, 0, 0};
    motor_queue_t queue_param = {.user = QUEUE_USER_NOTIFY_RCVD, .intervel = 40, .mask = MOTOR_MASK_NOTIFY};

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
    i = 0;
    while(notif_convert_list[i].disp_msg != 0xFF) {
        if(notif_convert_list[i].disp_msg == ancs_msg->type) {
            break;
        }
        i++;
    }
    if(notif_convert_list[i].disp_msg >= NOTIFY_DONE) { // notify not use or out of range
        BLE_SEND_LOG((u8*)&"\x00\x00\xFF", 3);
        ancs_msg->sta = NOTIFY_RESERVE;
    } else if((msg_en & (1UL<<i)) == 0) { // notify switch off
        BLE_SEND_LOG((u8*)&"\x00\x00\x00", 3);
        ancs_msg->sta = NOTIFY_RESERVE;
    }
    if(ancs_msg->sta == NOTIFY_RESERVE) {
    	*state = CLOCK;
    	return 0;
    }
	if(NOTIFY_ADD == ancs_msg->sta) {
		queue_param.dest[notify_motor] = ancs_msg->type;
	}else if(NOTIFY_REMOVE == ancs_msg->sta) {
        queue_param.dest[notify_motor] = NOTIFY_NONE;
	}
    if(queue_param.dest[notify_motor] < NOTIFY_DONE) {
        motor_params_enqueue(&queue_param);
    }
    log[2] = ancs_msg->sta;
    log[3] = ancs_msg->level;
    log[4] = ancs_msg->type;
    log[5] = ancs_msg->cnt;
    BLE_SEND_LOG(log, 6);

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
