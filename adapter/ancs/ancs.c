#include <mem.h>
#include "ancs_service_data.h"
#include "serial_service.h"
#include "ancs_client.h"
#include "adapter/adapter.h"

#define MESSAGE_IMPORTANCE_NONE           0
#define MESSAGE_IMPORTANCE_LINE           0xFF
#define MESSAGE_IMPORTANCE_QQ             0xFF
#define MESSAGE_IMPORTANCE_FACEMESSAGE    0xFF
#define MESSAGE_IMPORTANCE_WECHAT         0xFF
#define MESSAGE_IMPORTANCE_FACEBOOK       5
#define MESSAGE_IMPORTANCE_EMAIL          4
#define MESSAGE_IMPORTANCE_SMS            6
#define MESSAGE_IMPORTANCE_SKYPE          1
#define MESSAGE_IMPORTANCE_COMMING_CALL   8
#define MESSAGE_IMPORTANCE_TWITTER        3
#define MESSAGE_IMPORTANCE_WHATSAPP       2
#define MESSAGE_IMPORTANCE_CALENDAR       0xFF
#define MESSAGE_IMPORTANCE_LINKEDIN       7
#define MESSAGE_IMPORTANCE_NEWS           0xFF

#define APP_ID_STRING_NONE          "none"
#define APP_ID_STRING_LINE          "jp.naver.line"
#define APP_ID_STRING_QQ_IPAD       "com.tencent.mipadqq"     //! iPad QQ message ID
#define APP_ID_STRING_QQ_IPHONE     "com.tencent.mqq"         //! iPhone QQ message ID
#define APP_ID_STRING_FACEMESSAGE   "com.facebook.Messenger"  //! Messenger(Facebook) message ID
#define APP_ID_STRING_WECHAT        "com.tencent.xin"
#define APP_ID_STRING_FACEBOOK      "com.facebook.Facebook"   //! Facebook message ID
#define APP_ID_STRING_EMAIL         "com.apple.mobilemail"
#define APP_ID_STRING_SMS           "com.apple.MobileSMS"
#define APP_ID_STRING_SKYPE         "com.skype.tomskype"
#define APP_ID_STRING_SKYPE2        "com.skype.skype"
#define APP_ID_STRING_COMMING_CALL  "com.apple.mobilephone"
#define APP_ID_STRING_TWITTER       "com.atebits.Tweetie2"
#define APP_ID_STRING_WHATSAPP      "net.whatsapp.WhatsApp"
#define APP_ID_STRING_CALENDAR      "calendar" //...
#define APP_ID_STRING_LINKEDIN      "com.linkedin.LinkedIn"
#define APP_ID_STRING_LINKEDIN2     "com.linkedin.Zephyr"
#define APP_ID_STRING_NEWS          "news" //...

typedef struct app_id_index_t
{
	const u8 app_index;
    const u8 msg_level;
	const u8 app_id[MAX_LENGTH_APPID];
}APPIDINDEX;

static const APPIDINDEX app_msg_list[] =
{
    {NOTIFY_NONE,         MESSAGE_IMPORTANCE_NONE,          APP_ID_STRING_NONE},
    {NOTIFY_LINE,         MESSAGE_IMPORTANCE_LINE,          APP_ID_STRING_LINE},
    {NOTIFY_QQ,           MESSAGE_IMPORTANCE_QQ,            APP_ID_STRING_QQ_IPAD},     //! iPad QQ message ID
    {NOTIFY_QQ,           MESSAGE_IMPORTANCE_QQ,            APP_ID_STRING_QQ_IPHONE},   //! iPhone QQ message ID
    {NOTIFY_FACEMESSAGE,  MESSAGE_IMPORTANCE_FACEMESSAGE,   APP_ID_STRING_FACEMESSAGE}, //! Messenger(Facebook) message ID
    {NOTIFY_WECHAT,       MESSAGE_IMPORTANCE_WECHAT,        APP_ID_STRING_WECHAT},
    {NOTIFY_FACEBOOK,     MESSAGE_IMPORTANCE_FACEBOOK,      APP_ID_STRING_FACEBOOK},    //! Facebook message ID
    {NOTIFY_EMAIL,        MESSAGE_IMPORTANCE_EMAIL,         APP_ID_STRING_EMAIL},
    {NOTIFY_SMS,          MESSAGE_IMPORTANCE_SMS,           APP_ID_STRING_SMS},
    {NOTIFY_SKYPE,        MESSAGE_IMPORTANCE_SKYPE,         APP_ID_STRING_SKYPE},
    {NOTIFY_SKYPE,        MESSAGE_IMPORTANCE_SKYPE,         APP_ID_STRING_SKYPE2},
    {NOTIFY_COMMING_CALL, MESSAGE_IMPORTANCE_COMMING_CALL,  APP_ID_STRING_COMMING_CALL},
    {NOTIFY_TWITTER,      MESSAGE_IMPORTANCE_TWITTER,       APP_ID_STRING_TWITTER},
    {NOTIFY_WHATSAPP,     MESSAGE_IMPORTANCE_WHATSAPP,      APP_ID_STRING_WHATSAPP},
    {NOTIFY_CALENDER,     MESSAGE_IMPORTANCE_CALENDAR,      APP_ID_STRING_CALENDAR},
    {NOTIFY_LINKIN,       MESSAGE_IMPORTANCE_LINKEDIN,      APP_ID_STRING_LINKEDIN},
    {NOTIFY_LINKIN,       MESSAGE_IMPORTANCE_LINKEDIN,      APP_ID_STRING_LINKEDIN2},
    {NOTIFY_NEWS,         MESSAGE_IMPORTANCE_NEWS,          APP_ID_STRING_NEWS},

	{0, 0, "\0"} //! Finish here
};

typedef struct
{
    u8 uuid[4];                                     //! message UUID
    u8 evt_id;                                      //! added, modified, removed, reserved
    u8 evt_flag;                                    //! silent, important, reserved
    u8 cat_id;                                      //! 0other,1incomingCall,2missedCall,3vmail,4social,5schedule,6email,7news,8health,9bussiness,10location,11entertainment,12~255reserved
    u8 cat_cnt;                                     //! message count indicated by catId
    u8 attr_id_app_id[MAX_LENGTH_APPID];       //! app name, e.g. QQ->com.tencent.mqq
} packing_msg_t;

typedef struct
{
    u8 uuid[4];
    u8 appid[MAX_LENGTH_APPID];
} last_data_map_t;

static last_data_map_t last_data;
static packing_msg_t pck_msg;
app_msg_t  ancs_msg = {.cmd = 0x07};

static adapter_callback ancs_cb = NULL;

void ancs_business_handle(packing_msg_t* pack_msg);
void ancs_data_source_handle(u8 *p_data, u16 size_value, data_source_t *p_data_source);
void ancs_noti_source_handle(GATT_CHAR_VAL_IND_T *p_ind, noti_t *p_noti_source);
s16 ancs_init(adapter_callback cb);

void ancs_business_handle(packing_msg_t* pack_msg)
{
    u8 i = 0;
    u8 app_id_len = 0;
    u8 app_id_max_len = (20-LOG_PREFIX_LENGTH);
    log_send_ancs_id_t log_send = {.head = {LOG_SEND_FLAG, LOG_SEND_ANCS_APP_ID, sizeof(log_send_ancs_id_t), 0}};
    
    while(app_msg_list[i].app_id[0] != 0)
    {
        if(MemCmp(pack_msg->attr_id_app_id, app_msg_list[i].app_id, sizeof(app_msg_list[i].app_id)) == 0)
        {
            break;
        }
        i++;
    }
    
    MemCopy(last_data.uuid, pack_msg->uuid, 4);
    MemCopy(last_data.appid, app_msg_list[i].app_id, sizeof(app_msg_list[i].app_id));
    
    if(app_msg_list[i].app_id[0] == 0)
    {
        ancs_msg.level = 255; //! invalid if proMst.msgType = 255
        ancs_msg.type = 255; //! indicated unknown message
        log_send.recognized = 0x00; // app id not recognized

    }
    else
    {
        ancs_msg.level = app_msg_list[i].msg_level;
        ancs_msg.type = app_msg_list[i].app_index;
        log_send.recognized = 0x01; // app id recognized
    }
    
    ancs_msg.sta = pack_msg->evt_id;
    ancs_msg.cnt = pack_msg->cat_cnt;
    app_id_len = StrLen((char*)pack_msg->attr_id_app_id);
    if(app_id_len > app_id_max_len) {
        MemCopy(log_send.app_id, pack_msg->attr_id_app_id, app_id_max_len);
        log_send_initiate(&log_send.head);
        MemSet(log_send.app_id, 0, app_id_max_len);
        MemCopy(log_send.app_id, &pack_msg->attr_id_app_id[app_id_max_len], (app_id_len-app_id_max_len)); // assume the remain bytes less then 18
        log_send_initiate(&log_send.head);
    } else {
        MemCopy(log_send.app_id, pack_msg->attr_id_app_id, app_id_len);
        log_send_initiate(&log_send.head);
    }
    if(NULL != ancs_cb) {
		ancs_cb(ANCS_NOTIFY_INCOMING, NULL);
    }
}
void ancs_data_source_handle(u8 *p_data, u16 size_value, data_source_t *p_data_source)
{
    u8 i = 0;

    if(p_data_source->attr_id == 0)
    {
        for(i = 0; i < p_data_source->attr_len; i++) {
            pck_msg.attr_id_app_id[i] = p_data_source->attr_data[i];
        }
        ancs_business_handle(&pck_msg);
        MemSet(&pck_msg, 0, sizeof(packing_msg_t));
    }
}
void ancs_noti_source_handle(GATT_CHAR_VAL_IND_T *p_ind, noti_t *p_noti_source)
{
/*  Notification Source Data format
 * -------------------------------------------------------
 * |  Event  |  Event  |  Cat  |  Cat   |  Notification  |
 * |  ID     |  Flag   |  ID   |  Count |  UUID          |
 * |---------------------------------------------------- |
 * |   1B    |   1B    |   1B  |   1B   |   4B           |
 * -------------------------------------------------------
 */
    if(p_ind->value == NULL)
    {
        return;
    }
    
    source_t *noti_src = (source_t*)&p_noti_source->source;
    /** if noti.source.evtFlag is other value than 1 and 2, then just set it to 3 */
    noti_src->evt_flag = ((noti_src->evt_flag>2)||(noti_src->evt_flag<1))?3:noti_src->evt_flag;

    /** packing stage 1: pack the notif soure */
    MemSet(&pck_msg, 0, sizeof(packing_msg_t));
    u8 i =  0;
    for(i =  0; i < 4; i++) pck_msg.uuid[i] = noti_src->uuid[i];
    pck_msg.evt_id = noti_src->evt_id;
    pck_msg.evt_flag = noti_src->evt_flag;
    pck_msg.cat_id = noti_src->cat_id;
    pck_msg.cat_cnt = noti_src->cat_cnt;
    
    /** when incoming call is rejected by receiver, there is no data source to request */
    if(noti_src->evt_id == ancs_event_id_notif_removed)
    {
        /** we just can only handle the newly removed message just after the newly received one 
         *  (means who has the same uuin, but not include missed call)
         */
        //if((pck_msg.cat_id == ancs_cat_id_missed_call) || (MemCmp(last_data.uuid, pck_msg.uuid, 4) == 0))
        if(MemCmp(last_data.uuid, pck_msg.uuid, 4) == 0)
        {
            MemCopy(pck_msg.attr_id_app_id, last_data.appid, sizeof(APP_ID_STRING_COMMING_CALL));
            ancs_business_handle(&pck_msg);
        }
        else
        {
            /** actually, as many removed-events would comes togther, we just handle one whos uuid was match to the last added-event  */
        }
        MemSet(&last_data, 0, sizeof(last_data_map_t));
    }
    else
    {
        /** we don't need to handle the missed-call event followed by a removed-incomingCall event had handled above */
        if(pck_msg.cat_id == ancs_cat_id_missed_call)
        {
            /** do nothing here */
        }
        /** the cat id indicate the known app, so don't need to request to data source */
        else if((pck_msg.cat_id == ancs_cat_id_incoming_call)||
           /*(pck_msg.cat_id == ancs_cat_id_email)||*/
           (pck_msg.cat_id == ancs_cat_id_schedule)/*||
           (pck_msg.cat_id == ancs_cat_id_news)*/) //! did news need to be request data source???
        {
			if((pck_msg.cat_id == ancs_cat_id_incoming_call)) MemCopy(pck_msg.attr_id_app_id, APP_ID_STRING_COMMING_CALL, sizeof(APP_ID_STRING_COMMING_CALL));
			if((pck_msg.cat_id == ancs_cat_id_missed_call)) MemCopy(pck_msg.attr_id_app_id, APP_ID_STRING_COMMING_CALL, sizeof(APP_ID_STRING_COMMING_CALL));
			if((pck_msg.cat_id == ancs_cat_id_email)) MemCopy(pck_msg.attr_id_app_id, APP_ID_STRING_EMAIL, sizeof(APP_ID_STRING_EMAIL));
			if((pck_msg.cat_id == ancs_cat_id_schedule)) MemCopy(pck_msg.attr_id_app_id, APP_ID_STRING_CALENDAR, sizeof(APP_ID_STRING_CALENDAR));
			/*if((pck_msg.cat_id == ancs_cat_id_news)) MemCopy(pck_msg.attr_id_app_id, APP_ID_STRING_NEWS, sizeof(APP_ID_STRING_NEWS));*/
            ancs_business_handle(&pck_msg);
        }
        
        /** if cat id did not indicate a known app, need to request to data source */
        else
        {
            AncsGetNotificationAttributeCmd(p_noti_source->cid);
        }
    }
}
app_msg_t *ancs_get(void)
{
    return &ancs_msg;
}

s16 ancs_init(adapter_callback cb)
{
	ancs_cb = cb;
	return 0;
}
