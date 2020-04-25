#include <mem.h>
#include "ancs_service_data.h"
#include "serial_service.h"
#include "ancs_client.h"
#include "adapter/adapter.h"

#define ANCS_LOG_ERROR(...)    ancs_log(__FILE__, __func__, __LINE__, "<error>", __VA_ARGS__)
#define ANCS_LOG_WARNING(...)  ancs_log(__FILE__, __func__, __LINE__, "<warning>", __VA_ARGS__)
#define ANCS_LOG_INFO(...)     ancs_log(__FILE__, __func__, __LINE__, "<info>", __VA_ARGS__)
#define ANCS_LOG_DEBUG(...)    ancs_log(__FILE__, __func__, __LINE__, "<debug>", __VA_ARGS__)

/* Notification Source Event Flags */
#define ANCS_NS_EVENTFLAG_SILENT                 (0x01) 
#define ANCS_NS_EVENTFLAG_IMPORTANT              (0x02) 
#define ANCS_NS_EVENTFLAG_RESERVED               ((1<<2)-(1<<7))

#define MESSAGE_POSITION_NONE           0
#define MESSAGE_POSITION_LINE           0xFF
#define MESSAGE_POSITION_QQ             0xFF
#define MESSAGE_POSITION_FACEMESSAGE    0xFF
#define MESSAGE_POSITION_WECHAT         0xFF
#define MESSAGE_POSITION_FACEBOOK       5
#define MESSAGE_POSITION_EMAIL          4
#define MESSAGE_POSITION_SMS            6
#define MESSAGE_POSITION_SKYPE          1
#define MESSAGE_POSITION_COMMING_CALL   8
#define MESSAGE_POSITION_TWITTER        3
#define MESSAGE_POSITION_WHATSAPP       2
#define MESSAGE_POSITION_CALENDAR       0xFF
#define MESSAGE_POSITION_LINKEDIN       7
#define MESSAGE_POSITION_NEWS           0xFF

#define APP_ID_STRING_NONE          "none"
#define APP_ID_STRING_LINE          "jp.naver.line"
#define APP_ID_STRING_QQ_IPAD       "com.tencent.mipadqq"     //! iPad QQ message ID
#define APP_ID_STRING_QQ_IPHONE     "com.tencent.mqq"         //! iPhone QQ message ID
#define APP_ID_STRING_FACEMESSAGE   "com.facebook.Messenger"  //! Messenger(Facebook) message ID
#define APP_ID_STRING_WECHAT        "com.tencent.xin"
#define APP_ID_STRING_FACEBOOK      "com.facebook.Facebook"   //! Facebook message ID
#define APP_ID_STRING_EMAIL         "email" //...
#define APP_ID_STRING_SMS           "com.apple.MobileSMS"
#define APP_ID_STRING_SKYPE         "com.skype.tomskype"
#define APP_ID_STRING_COMMING_CALL  "com.apple.mobilephone"
#define APP_ID_STRING_TWITTER       "com.atebits.Tweetie2"
#define APP_ID_STRING_WHATSAPP      "net.whatsapp.WhatsApp"
#define APP_ID_STRING_CALENDAR      "calendar" //...
#define APP_ID_STRING_LINKEDIN      "com.linkedin.LinkedIn"
#define APP_ID_STRING_NEWS          "news" //...

typedef struct app_id_index_t
{
	const u8 app_index;
	const u8 app_id[MAX_LENGTH_APPID];
}APPIDINDEX;
static const APPIDINDEX app_msg_list[] =
{
    {MESSAGE_POSITION_NONE,         APP_ID_STRING_NONE},
    {MESSAGE_POSITION_LINE,         APP_ID_STRING_LINE},
    {MESSAGE_POSITION_QQ,           APP_ID_STRING_QQ_IPAD},     //! iPad QQ message ID
    {MESSAGE_POSITION_QQ,           APP_ID_STRING_QQ_IPHONE},   //! iPhone QQ message ID
    {MESSAGE_POSITION_FACEMESSAGE,  APP_ID_STRING_FACEMESSAGE}, //! Messenger(Facebook) message ID
    {MESSAGE_POSITION_WECHAT,       APP_ID_STRING_WECHAT},
    {MESSAGE_POSITION_FACEBOOK,     APP_ID_STRING_FACEBOOK},    //! Facebook message ID
    {MESSAGE_POSITION_EMAIL,        APP_ID_STRING_EMAIL},
    {MESSAGE_POSITION_SMS,          APP_ID_STRING_SMS},
    {MESSAGE_POSITION_SKYPE,        APP_ID_STRING_SKYPE},
    {MESSAGE_POSITION_COMMING_CALL, APP_ID_STRING_COMMING_CALL},
    {MESSAGE_POSITION_TWITTER,      APP_ID_STRING_TWITTER},
    {MESSAGE_POSITION_WHATSAPP,     APP_ID_STRING_WHATSAPP},
    {MESSAGE_POSITION_CALENDAR,     APP_ID_STRING_CALENDAR},
    {MESSAGE_POSITION_LINKEDIN,     APP_ID_STRING_LINKEDIN},
    {MESSAGE_POSITION_NEWS,         APP_ID_STRING_NEWS},

	{0, "\0"} //! Finish here
};

/* enum for notification attribute id 
 * NOTE: need to one-to-one correspondence with @ref ancs_event_id in ancs_service_data.h
*/
typedef enum
{
    appid = 0x0,
    title,
    subtitle,
    message,
    messageSize,
    date
}att_id_str;

typedef struct
{
    u8 recv_attr_id_fragment;                       //! receive msg count, total fragment = REQ_ANCS_NOTIF_ATT_ID_TOTAL
    u8 uuid[4];                                     //! message UUID
    u8 evt_id;                                      //! added, modified, removed, reserved
    u8 evt_flag;                                    //! silent, important, reserved
    u8 cat_id;                                      //! 0other,1incomingCall,2missedCall,3vmail,4social,5schedule,6email,7news,8health,9bussiness,10location,11entertainment,12~255reserved
    u8 cat_cnt;                                     //! message count indicated by catId
    u8 attr_id_app_id_data[MAX_LENGTH_ATTTDATA];    //! app name, e.g. QQ->com.tencent.mqq
    u8 attr_id_title_data[MAX_LENGTH_ATTTDATA];     //! contact name
    u8 attr_id_sub_title_data[MAX_LENGTH_ATTTDATA]; //! sub contact name
    u8 attr_id_message_data[MAX_LENGTH_ATTTDATA];   //! message content
    uint16 attr_id_message_size;                    //! message length
    u8 attr_id_date_data[MAX_LENGTH_ATTTDATA];      //! date and time that the message was received
} packing_msg_t;

typedef struct
{
    u8 uuid[4];
    u8 appid[MAX_LENGTH_APPID];
} last_data_map_t;

static last_data_map_t last_data;
static packing_msg_t pck_msg;
ancs_msg_t  ancs_msg = {.cmd = 0x07};


int ancs_log(const char* file, const char* func, unsigned line, const char* level, const char * sFormat, ...);
void ancs_business_handle(packing_msg_t* pack_msg);
void ancs_data_source_handle(u8 *p_data, u16 size_value, data_source_t *p_data_source);
void ancs_noti_source_handle(GATT_CHAR_VAL_IND_T *p_ind, noti_t *p_noti_source);

int ancs_log(const char* file, const char* func, unsigned line, const char* level, const char * sFormat, ...)
{
    return 0;
}

void ancs_business_handle(packing_msg_t* pack_msg)
{
    u8 i = 0;

    while(app_msg_list[i].app_id[0] != 0)
    {
        if(MemCmp(pack_msg->attr_id_app_id_data, app_msg_list[i].app_id, sizeof(app_msg_list[i].app_id)) == 0)
        {
            break;
        }
        i++;
    }
    
    MemCopy(last_data.uuid, pack_msg->uuid, 4);
    MemCopy(last_data.appid, app_msg_list[i].app_id, sizeof(app_msg_list[i].app_id));
    
    ANCS_LOG_INFO("-> uuid = %02X%02X%02X%02X\r\n", pack_msg->uuid[0],pack_msg->uuid[1],pack_msg->uuid[2],pack_msg->uuid[3]);
    ANCS_LOG_INFO("0> cat id          (notif type) = %d\r\n", pack_msg->cat_id);
    ANCS_LOG_INFO("1> event id       (notif state) = %d\r\n", pack_msg->evt_id);
    //ANCS_LOG_INFO("2> event flag (importance)      = %s\r\n", ancs_notif_event_flag_str[pack_msg->evt_flag]);
    if(app_msg_list[i].app_id[0] == 0)
    {
        ANCS_LOG_INFO("2> level           (importance) = <invalid>\r\n");
        ANCS_LOG_INFO("3> attr app id   (message type) = <not found>\r\n");
        ancs_msg.level = 255; //! invalid if proMst.msgType = 255
        ancs_msg.type = 255; //! indicated unknown message
    }
    else
    {
        ANCS_LOG_INFO("2> level           (importance) = %d\r\n", app_msg_list[i].app_index);
        ANCS_LOG_INFO("3> attr app id   (message type) = %s\r\n", app_msg_list[i].app_id);
        ancs_msg.level = app_msg_list[i].app_index;
        ancs_msg.type = i;
    }
    ANCS_LOG_INFO("4> cat cnt      (message count) = %d\r\n", pack_msg->cat_cnt); 
    
    ancs_msg.sta = pack_msg->evt_id;
    ancs_msg.cnt = pack_msg->cat_cnt;
    SerialSendNotification((u8 *)&ancs_msg, 5); //! send ANCS msg to peer, for test purpose
    //ancs_cb_handler();
}
void ancs_data_source_handle(u8 *p_data, u16 size_value, data_source_t *p_data_source)
{
    u8 i = 0;
    #if USE_MY_ANCS_DEBUG
    u8 *uuid = p_data_source->uuid;
    ANCS_LOG_DEBUG("++ uuid       = %02X%02X%02X%02X\r\n", uuid[0],uuid[1],uuid[2],uuid[3]);
    ANCS_LOG_DEBUG("++ attr id    = %d\r\n", p_data_source->attr_id);
    ANCS_LOG_DEBUG("++ attr len   = %d\r\n", p_data_source->attr_len);
    ANCS_LOG_DEBUG("++ attr data  = %s\r\n", p_data_source->attr_data);
    #endif

    if(p_data_source->attr_id == appid)
    {
        for(i = 0; i < p_data_source->attr_len; i++) pck_msg.attr_id_app_id_data[i] = p_data_source->attr_data[i];
    }
    else if(p_data_source->attr_id == title)
    {
        for(i = 0; i < p_data_source->attr_len; i++) pck_msg.attr_id_title_data[i] = p_data_source->attr_data[i];
    }
    else if(p_data_source->attr_id == subtitle)
    {
        for(i = 0; i < p_data_source->attr_len; i++) pck_msg.attr_id_sub_title_data[i] = p_data_source->attr_data[i];
    }
    else if(p_data_source->attr_id == message)
    {
        for(i = 0; i < p_data_source->attr_len; i++) pck_msg.attr_id_message_data[i] = p_data_source->attr_data[i];
    }
    else if(p_data_source->attr_id == messageSize)
    {
        for(i = 0; i < p_data_source->attr_len; i++)
        {
            pck_msg.attr_id_message_size *= 10;
            pck_msg.attr_id_message_size += (p_data_source->attr_data[i]-'0');
        }
    }
    else if(p_data_source->attr_id == date)
    {
        for(i = 0; i < p_data_source->attr_len; i++) pck_msg.attr_id_date_data[i] = p_data_source->attr_data[i];
    }
    
    if(++pck_msg.recv_attr_id_fragment >= REQ_ANCS_NOTIF_ATT_ID_TOTAL)
    {
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
    #if USE_MY_ANCS_DEBUG
    ANCS_LOG_DEBUG("-- uuid       = %02X%02X%02X%02X\r\n", noti_src->uuid[0],noti_src->uuid[1],noti_src->uuid[2],noti_src->uuid[3]);
    ANCS_LOG_DEBUG("-- event id   = %d\r\n", noti_src->evt_id);
    ANCS_LOG_DEBUG("-- event flag = %d\r\n", noti_src->evt_flag);
    ANCS_LOG_DEBUG("-- cat id     = %d\r\n", noti_src->cat_id);
    ANCS_LOG_DEBUG("*- cat cnt    = %d\r\n", noti_src->cat_cnt);
    #endif

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
            LogReport(__FILE__, __func__, __LINE__, M_Ancs_removed_event_occur);
            MemCopy(pck_msg.attr_id_app_id_data, last_data.appid, sizeof(APP_ID_STRING_COMMING_CALL));
            ancs_business_handle(&pck_msg);
        }
        else
        {
            /** actually, as many removed-events would comes togther, we just handle one whos uuid was match to the last added-event  */
            /*ANCS_LOG_DEBUG("removed event uuid not match:\r\n");
            ANCS_LOG_DEBUG("event id = %d\r\n", notiSrc->evtId);
            ANCS_LOG_DEBUG("f-b uuid = %02X%02X%02X%02X - %02X%02X%02X%02X\r\n", 
                            lastData.uuid[0],lastData.uuid[1],lastData.uuid[2],lastData.uuid[3],
                            pckMsg.uuid[0],pckMsg.uuid[1],pckMsg.uuid[2],pckMsg.uuid[3]);*/
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
           (pck_msg.cat_id == ancs_cat_id_email)||
           (pck_msg.cat_id == ancs_cat_id_schedule)||
           (pck_msg.cat_id == ancs_cat_id_news)) //! did news need to be request data source???
        {
			if((pck_msg.cat_id == ancs_cat_id_incoming_call)) MemCopy(pck_msg.attr_id_app_id_data, APP_ID_STRING_COMMING_CALL, sizeof(APP_ID_STRING_COMMING_CALL));
			if((pck_msg.cat_id == ancs_cat_id_missed_call)) MemCopy(pck_msg.attr_id_app_id_data, APP_ID_STRING_COMMING_CALL, sizeof(APP_ID_STRING_COMMING_CALL));
			if((pck_msg.cat_id == ancs_cat_id_email)) MemCopy(pck_msg.attr_id_app_id_data, APP_ID_STRING_EMAIL, sizeof(APP_ID_STRING_EMAIL));
			if((pck_msg.cat_id == ancs_cat_id_schedule)) MemCopy(pck_msg.attr_id_app_id_data, APP_ID_STRING_CALENDAR, sizeof(APP_ID_STRING_CALENDAR));
			if((pck_msg.cat_id == ancs_cat_id_news)) MemCopy(pck_msg.attr_id_app_id_data, APP_ID_STRING_NEWS, sizeof(APP_ID_STRING_NEWS));
            ancs_business_handle(&pck_msg);
        }
        
        /** if cat id did not indicate a known app, need to request to data source */
        else
        {
            LogReport(__FILE__, __func__, __LINE__, M_Ancs_send_data_source_request);
            AncsGetNotificationAttributeCmd(p_noti_source->cid);
        }
    }
}
ancs_msg_t *ancs_get(void)
{
    return &ancs_msg;
}
