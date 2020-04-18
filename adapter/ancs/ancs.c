/**
*  @file    m_ancs.c
*  @author  maliwen
*  @date    2020/03/17
*  @history 1.0.0.0(版本号)
*           1.0.0.0: Creat              2020/03/17
*/
#include <mem.h>
#include "ancs_service_data.h"
#include "m_ancs.h"
#include "serial_service.h"
#include "ancs_client.h"

#define MANCS_LOG_ERROR(...)
#define MANCS_LOG_WARNING(...)
#define MANCS_LOG_INFO(...)
#define MANCS_LOG_DEBUG(...)

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
#define MESSAGE_POSITION_NEWS           0xFF //! add by mlw

#define APP_ID_STRING_NONE          "none"
#define APP_ID_STRING_LINE          "jp.naver.line"
#define APP_ID_STRING_QQ_IPAD       "com.tencent.mipadqq"     //! iPad QQ message ID
#define APP_ID_STRING_QQ_IPHONE     "com.tencent.mqq"         //! iPhone QQ message ID
#define APP_ID_STRING_FACEMESSAGE   "com.facebook.Messenger"  //! Messenger(Facebook) message ID
#define APP_ID_STRING_WECHAT        "com.tencent.xin"
#define APP_ID_STRING_FACEBOOK      "com.facebook.Facebook"   //! Facebook message ID
#define APP_ID_STRING_EMAIL         "email"
#define APP_ID_STRING_SMS           "com.apple.MobileSMS"
#define APP_ID_STRING_SKYPE         "com.skype.tomskype"
#define APP_ID_STRING_COMMING_CALL  "com.apple.mobilephone"
#define APP_ID_STRING_TWITTER       "com.atebits.Tweetie2"
#define APP_ID_STRING_WHATSAPP      "net.whatsapp.WhatsApp"
#define APP_ID_STRING_CALENDAR      "calendar"
#define APP_ID_STRING_LINKEDIN      "com.linkedin.LinkedIn"
#define APP_ID_STRING_NEWS          "news"

typedef struct AppidIndex_T
{
	const uint8 appIndex;
	const uint8 appId[MAX_LENGTH_APPID];
}APPIDINDEX;
static const APPIDINDEX appMsgList[] =
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

/* enum for event id 
 * NOTE: need to one-to-one correspondence with @ref ancs_event_id in ancs_service_data.h
*/
typedef enum 
{
    added     = 0x0,
    modified,
    removed,
    reserved1
}event_id_str;

/* enum for category id 
 * NOTE: need to one-to-one correspondence with @ref ancs_category_id in ancs_service_data.h
*/
typedef enum
{
    other             = 0x0,   /* Other : updates */
    incomingCall,              /*  Call: Incoming call */
    missedCall,                /* Missed call: Missed Call */
    vmail,                     /* Voice mail: Voice mail*/
    social,                    /* Social message indications */
    schedule,                  /* Schedule: Calendar, planner */
    email,                     /* Email: mail message arrives */
    news,                      /* News: RSS/Atom feeds etc */
    hnf,                       /* Health and Fitness: updates  */
    bnf,                       /* Business and Finance: updates */
    location,                  /* Location: updates */
    entertainment,             /* Entertainment: updates */
    reserved2                  /* Reserved */
}category_id_str;

/* enum for notification attribute id 
 * NOTE: need to one-to-one correspondence with @ref ancs_event_id in ancs_service_data.h
*/
typedef enum
{
    appId = 0x0,
    title,
    subtitle,
    message,
    messageSize,
    date
}att_id_str;

/* enum for notification event flag 
 * NOTE: need to one-to-one correspondence with @ref ANCS_NS_EVENTFLAG_XXX in ancs_service_data.h
*/
typedef enum
{
    none = 0x0,
    silent,
    important,
    reserved
}event_flag_str;

#if 0
#define STRINGIFY(val) #val
#define M_VALUE_TO_STR(name) [name] = STRINGIFY(name)

static const char * ancs_event_id_str[] =
{
    M_VALUE_TO_STR(added),
    M_VALUE_TO_STR(modified),
    M_VALUE_TO_STR(removed),
    M_VALUE_TO_STR(reserved1)
};
static const char * ancs_category_id_str[] =
{
    M_VALUE_TO_STR(other),
    M_VALUE_TO_STR(incomingCall),
    M_VALUE_TO_STR(missedCall),
    M_VALUE_TO_STR(vmail),
    M_VALUE_TO_STR(social),
    M_VALUE_TO_STR(schedule),
    M_VALUE_TO_STR(email),
    M_VALUE_TO_STR(news),
    M_VALUE_TO_STR(hnf),
    M_VALUE_TO_STR(bnf),
    M_VALUE_TO_STR(location),
    M_VALUE_TO_STR(entertainment),
    M_VALUE_TO_STR(reserved2)
};
#if USE_MY_ANCS_DEBUG
static const char * ancs_notif_att_id_str[] =
{
    M_VALUE_TO_STR(appId),
    M_VALUE_TO_STR(title),
    M_VALUE_TO_STR(subtitle),
    M_VALUE_TO_STR(message),
    M_VALUE_TO_STR(messageSize),
    M_VALUE_TO_STR(date)
};
static const char * ancs_notif_event_flag_str[] =
{
    M_VALUE_TO_STR(none),
    M_VALUE_TO_STR(silent),
    M_VALUE_TO_STR(important),
    M_VALUE_TO_STR(reserved)
};
#endif
#endif //! end with 0
typedef enum
{
    ANCS_MSG_TYPE_ADDED_MODIFIED_KNOWN,
    ANCS_MSG_TYPE_ADDED_MODIFIED_UNKNOWN,
    ANCS_MSG_TYPE_REMOVED
} ancs_msg_type_t;

typedef struct
{
    uint8 uuid[4];
    uint8 appid[MAX_LENGTH_APPID];
} last_data_map_t;

static last_data_map_t lastData;
static packing_msg_t pckMsg;

/**
* @brief do the business handle when after a complete packet of data source receved
* @param [in] packMsg - the complete packet of data source
*             isDataSrc - TRUE:data source, FALSE:notif source
* @param [out] none
* @return none
* @data 2020/03/19 16:16
* @author maliwen
* @note none
*/
void m_ancs_business_handle(ancs_msg_type_t msgTpye, packing_msg_t* packMsg);
void m_ancs_business_handle(ancs_msg_type_t msgTpye, packing_msg_t* packMsg)
{
    uint8 i = 0;
    protocol_msg_t  proMsg = {.cmd = 0x07};

    while(appMsgList[i].appId[0] != 0)
    {
        if(MemCmp(packMsg->attrIdAppIdData, appMsgList[i].appId, sizeof(appMsgList[i].appId)) == 0)
        {
            //MANCS_LOG_INFO("list len = %d, cmp index = %d, app index = %d\r\n", sizeof(appMsgList), i, appMsgList[i].appIndex);
            //MANCS_LOG_INFO("sizeof = %d\r\n", sizeof(appMsgList[i].appId));
            //MANCS_LOG_INFO("index = %d\r\n", i);
            //MANCS_LOG_INFO("in id = %s\r\n", packMsg->attrIdAppIdData);
            //MANCS_LOG_INFO("cp id = %s\r\n", appMsgList[i].appId);
            break;
        }
        i++;
    }
    
    MemCopy(lastData.uuid, packMsg->uuid, 4);
    MemCopy(lastData.appid, appMsgList[i].appId, sizeof(appMsgList[i].appId));
    
    MANCS_LOG_INFO("-> uuid = %02X%02X%02X%02X\r\n", packMsg->uuid[0],packMsg->uuid[1],packMsg->uuid[2],packMsg->uuid[3]);
    MANCS_LOG_INFO("0> cat id          (notif type) = %s\r\n", ancs_category_id_str[packMsg->catId]);
    MANCS_LOG_INFO("1> event id       (notif state) = %s\r\n", ancs_event_id_str[packMsg->evtId]);
    //MANCS_LOG_INFO("2> event flag (importance)      = %s\r\n", ancs_notif_event_flag_str[packMsg->evtFlag]);
    if(appMsgList[i].appId[0] == 0)
    {
        MANCS_LOG_INFO("2> level           (importance) = <invalid>\r\n");
        MANCS_LOG_INFO("3> attr app id   (message type) = <not found>\r\n");
        proMsg.level = 255; //! invalid if proMst.msgType = 255
        proMsg.type = 255; //! indicated unknown message
    }
    else
    {
        MANCS_LOG_INFO("2> level           (importance) = %d\r\n", appMsgList[i].appIndex);
        MANCS_LOG_INFO("3> attr app id   (message type) = %s\r\n", appMsgList[i].appId);
        proMsg.level = appMsgList[i].appIndex;
        proMsg.type = i;
    }
    MANCS_LOG_INFO("4> cat cnt      (message count) = %d\r\n", packMsg->catCnt); 
    
    proMsg.sta = packMsg->evtId;
    proMsg.cnt = packMsg->catCnt;
    SerialSendNotification((uint8 *)&proMsg, 5); //! send ANCS msg to peer, for test purpose

    #if USE_MY_ANCS_DEBUG
    if(msgTpye == ANCS_MSG_TYPE_ADDED_MODIFIED_UNKNOWN)
    {
        #if REQ_ANCS_NOTIF_ATT_ID_TITLE
        MANCS_LOG_INFO("-> attr title        = %s\r\n", packMsg->attrIdTitleData);
        #endif
        #if REQ_ANCS_NOTIF_ATT_ID_SUBTITLE
        MANCS_LOG_INFO("-> attr sub title    = %s\r\n", packMsg->attrIdSubTitleData);
        #endif
        #if REQ_ANCS_NOTIF_ATT_ID_MESSAGE
        MANCS_LOG_INFO("-> attr message      = %s\r\n", packMsg->attrIdMessageData);
        #endif
        #if REQ_ANCS_NOTIF_ATT_ID_MESSAGE_SIZE
        MANCS_LOG_INFO("-> attr message size = %d\r\n", packMsg->attrIdMessageSize);
        #endif
        #if REQ_ANCS_NOTIF_ATT_ID_DATE
        MANCS_LOG_INFO("-> attr date         = %s\r\n", packMsg->attrIdDateData);
        #endif
    }
    #endif
}

/**
* @brief ANCS通知源处理
* @param [in] p_data - ancs data source original packet
              size_value - ancs data source original packet length
              p_data_source - ancs data source parsed packet
* @param [out] none
* @return none
* @data 2020/03/18 21:24
* @author maliwen
* @note none
*/
void m_ancs_data_source_handle(uint8 *p_data, uint16 size_value, data_source_t *p_data_source)
{
    uint8 i = 0;
    #if USE_MY_ANCS_DEBUG
    uint8 *uuid = p_data_source->uuid;
    MANCS_LOG_DEBUG("++ uuid       = %02X%02X%02X%02X\r\n", uuid[0],uuid[1],uuid[2],uuid[3]);
    MANCS_LOG_DEBUG("++ attr id    = %s\r\n", ancs_notif_att_id_str[p_data_source->attrId]);
    MANCS_LOG_DEBUG("++ attr len   = %d\r\n", p_data_source->attrLen);
    MANCS_LOG_DEBUG("++ attr data  = %s\r\n", p_data_source->attrData);
    #endif

    /*for(i = 0; i < 4; i++)
    {
        pckMsg.uuid[i] = uuid[i];
    }*/
    if(p_data_source->attrId == appId)
    {
        for(i = 0; i < p_data_source->attrLen; i++) pckMsg.attrIdAppIdData[i] = p_data_source->attrData[i];
    }
    else if(p_data_source->attrId == title)
    {
        for(i = 0; i < p_data_source->attrLen; i++) pckMsg.attrIdTitleData[i] = p_data_source->attrData[i];
    }
    else if(p_data_source->attrId == subtitle)
    {
        for(i = 0; i < p_data_source->attrLen; i++) pckMsg.attrIdSubTitleData[i] = p_data_source->attrData[i];
    }
    else if(p_data_source->attrId == message)
    {
        for(i = 0; i < p_data_source->attrLen; i++) pckMsg.attrIdMessageData[i] = p_data_source->attrData[i];
    }
    else if(p_data_source->attrId == messageSize)
    {
        for(i = 0; i < p_data_source->attrLen; i++)
        {
            pckMsg.attrIdMessageSize *= 10;
            pckMsg.attrIdMessageSize += (p_data_source->attrData[i]-'0');
        }
    }
    else if(p_data_source->attrId == date)
    {
        for(i = 0; i < p_data_source->attrLen; i++) pckMsg.attrIdDateData[i] = p_data_source->attrData[i];
    }
    
    if(++pckMsg.recvAttrIdFragment >= REQ_ANCS_NOTIF_ATT_ID_TOTAL)
    {
        m_ancs_business_handle(ANCS_MSG_TYPE_ADDED_MODIFIED_UNKNOWN, &pckMsg);
        MemSet(&pckMsg, 0, sizeof(packing_msg_t));
    }
}

/**
* @brief ANCS通知源处理
* @param [in] p_ind - ancs通知回调指针
*             p_noti_source - noti source was received
* @param [out] none
* @return none
* @data 2020/03/17 15:22
* @author maliwen
* @note none
*/
void m_ancs_noti_source_handle(GATT_CHAR_VAL_IND_T *p_ind, noti_t *p_noti_source)
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
    
    source_t *notiSrc = (source_t*)&p_noti_source->source;
    /** if noti.source.evtFlag is other value than 1 and 2, then just set it to 3 */
    notiSrc->evtFlag = ((notiSrc->evtFlag>2)||(notiSrc->evtFlag<1))?3:notiSrc->evtFlag;
    #if USE_MY_ANCS_DEBUG
    MANCS_LOG_DEBUG("-- uuid       = %02X%02X%02X%02X\r\n", notiSrc->uuid[0],notiSrc->uuid[1],notiSrc->uuid[2],notiSrc->uuid[3]);
    MANCS_LOG_DEBUG("-- event id   = %s\r\n", ancs_event_id_str[notiSrc->evtId]);
    MANCS_LOG_DEBUG("-- event flag = %s\r\n", ancs_notif_event_flag_str[notiSrc->evtFlag]);
    MANCS_LOG_DEBUG("-- cat id     = %s\r\n", ancs_category_id_str[notiSrc->catId]);
    MANCS_LOG_DEBUG("*- cat cnt    = %d\r\n", notiSrc->catCnt);
    #endif

    /** packing stage 1: pack the notif soure */
    MemSet(&pckMsg, 0, sizeof(packing_msg_t));
    uint8 i =  0;
    for(i =  0; i < 4; i++) pckMsg.uuid[i] = notiSrc->uuid[i];
    pckMsg.evtId = notiSrc->evtId;
    pckMsg.evtFlag = notiSrc->evtFlag;
    pckMsg.catId = notiSrc->catId;
    pckMsg.catCnt = notiSrc->catCnt;
    
    /** when incoming call is rejected by receiver, there is no data source to request */
    if(notiSrc->evtId == ancs_event_id_notif_removed)
    {
        /** we just can only handle the newly removed message just after the newly received one 
         *  (means who has the same uuin, but not include missed call)
         */
        //if((pckMsg.catId == ancs_cat_id_missed_call) || (MemCmp(lastData.uuid, pckMsg.uuid, 4) == 0))
        if(MemCmp(lastData.uuid, pckMsg.uuid, 4) == 0)
        {
            LogReport(__FILE__, __func__, __LINE__, M_Ancs_removed_event_occur);
            MemCopy(pckMsg.attrIdAppIdData, lastData.appid, sizeof(APP_ID_STRING_COMMING_CALL));
            m_ancs_business_handle(ANCS_MSG_TYPE_REMOVED, &pckMsg);
        }
        else
        {
            /** actually, as many removed-events would comes togther, we just handle one whos uuid was match to the last added-event  */
            /*MANCS_LOG_DEBUG("removed event uuid not match:\r\n");
            MANCS_LOG_DEBUG("event id = %d\r\n", notiSrc->evtId);
            MANCS_LOG_DEBUG("f-b uuid = %02X%02X%02X%02X - %02X%02X%02X%02X\r\n", 
                            lastData.uuid[0],lastData.uuid[1],lastData.uuid[2],lastData.uuid[3],
                            pckMsg.uuid[0],pckMsg.uuid[1],pckMsg.uuid[2],pckMsg.uuid[3]);*/
        }
        MemSet(&lastData, 0, sizeof(last_data_map_t));
    }
    else
    {
        /** we don't need to handle the missed-call event followed by a removed-incomingCall event had handled above */
        if(pckMsg.catId == ancs_cat_id_missed_call)
        {
            /** do nothing here */
        }
        /** the cat id indicate the known app, so don't need to request to data source */
        else if((pckMsg.catId == ancs_cat_id_incoming_call)||
           (pckMsg.catId == ancs_cat_id_email)||
           (pckMsg.catId == ancs_cat_id_schedule)||
           (pckMsg.catId == ancs_cat_id_news)) //! did news need to be request data source???
        {
			if((pckMsg.catId == ancs_cat_id_incoming_call)) MemCopy(pckMsg.attrIdAppIdData, APP_ID_STRING_COMMING_CALL, sizeof(APP_ID_STRING_COMMING_CALL));
			if((pckMsg.catId == ancs_cat_id_missed_call)) MemCopy(pckMsg.attrIdAppIdData, APP_ID_STRING_COMMING_CALL, sizeof(APP_ID_STRING_COMMING_CALL));
			if((pckMsg.catId == ancs_cat_id_email)) MemCopy(pckMsg.attrIdAppIdData, APP_ID_STRING_EMAIL, sizeof(APP_ID_STRING_EMAIL));
			if((pckMsg.catId == ancs_cat_id_schedule)) MemCopy(pckMsg.attrIdAppIdData, APP_ID_STRING_CALENDAR, sizeof(APP_ID_STRING_CALENDAR));
			if((pckMsg.catId == ancs_cat_id_news)) MemCopy(pckMsg.attrIdAppIdData, APP_ID_STRING_NEWS, sizeof(APP_ID_STRING_NEWS));
            m_ancs_business_handle(ANCS_MSG_TYPE_ADDED_MODIFIED_KNOWN, &pckMsg);
        }
        
        /** if cat id did not indicate a known app, need to request to data source */
        else
        {
            LogReport(__FILE__, __func__, __LINE__, M_Ancs_send_data_source_request);
            AncsGetNotificationAttributeCmd(p_noti_source->cid);
        }
    }
}
