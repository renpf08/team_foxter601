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
#include "m_printf.h"
#include "m_timer.h"

#define MANCS_LOG_ERROR(...)        M_LOG_ERROR(__VA_ARGS__)
#define MANCS_LOG_WARNING(...)      M_LOG_WARNING(__VA_ARGS__)
#define MANCS_LOG_INFO(...)         M_LOG_INFO(__VA_ARGS__)
#define MANCS_LOG_DEBUG(...)        M_LOG_DEBUG(__VA_ARGS__)

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
    incomingCall,             /*  Call: Incoming call */
    missedCall,               /* Missed call: Missed Call */
    vmail,                     /* Voice mail: Voice mail*/
    social,                    /* Social message indications */
    schedule,                  /* Schedule: Calendar, planner */
    email,                     /* Email: mail message arrives */
    news,                      /* News: RSS/Atom feeds etc */
    hnf,                       /* Health and Fitness: updates  */
    bnf,                       /* Business and Finance: updates */
    location,                  /* Location: updates */
    entertainment,             /* Entertainment: updates */
    reserved2                   /* Reserved */
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
    reserved3
}event_flag_str;
;
/* Notification Source Event Flags */
#define ANCS_NS_EVENTFLAG_SILENT                 (0x01) 
#define ANCS_NS_EVENTFLAG_IMPORTANT              (0x02) 
#define ANCS_NS_EVENTFLAG_RESERVED               ((1<<2)-(1<<7))

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
    M_VALUE_TO_STR(reserved3)
};

static packing_msg_t pckMsg;

/**
* @brief do the business handle when after a complete packet of data source receved
* @param [in] packMsg - the complete packet of data source
* @param [out] none
* @return none
* @data 2020/03/19 16:16
* @author maliwen
* @note none
*/
void m_ancs_business_handle(packing_msg_t* packMsg);
void m_ancs_business_handle(packing_msg_t* packMsg)
{
    MANCS_LOG_INFO("-> uuid              = %02X%02X%02X%02X\r\n", packMsg->uuid[0],packMsg->uuid[1],packMsg->uuid[2],packMsg->uuid[3]);
    MANCS_LOG_INFO("-> event id          = %s\r\n", ancs_event_id_str[packMsg->evtId]);
    MANCS_LOG_INFO("-> event flag        = %s\r\n", ancs_notif_event_flag_str[packMsg->evtFlag]);
    MANCS_LOG_INFO("-> cat id            = %s\r\n", ancs_category_id_str[packMsg->catId]);
    MANCS_LOG_INFO("-> cat cnt           = %04d\r\n", packMsg->catCnt);
    MANCS_LOG_INFO("-> attr app id       = %s\r\n", packMsg->attrIdAppIdData);
    MANCS_LOG_INFO("-> attr title        = %s\r\n", packMsg->attrIdTitleData);
    MANCS_LOG_INFO("-> attr sub title    = %s\r\n", packMsg->attrIdSubTitleData);
    MANCS_LOG_INFO("-> attr message      = %s\r\n", packMsg->attrIdMessageData);
    MANCS_LOG_INFO("-> attr message size = %02d\r\n", packMsg->attrIdMessageSize);
    MANCS_LOG_INFO("-> attr date         = %s\r\n", packMsg->attrIdDateData);
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
    //return;
    uint8 i = 0;
    uint8 *pUuid = p_data_source->uuid;
    MANCS_LOG_DEBUG("++ uuid       = %02X%02X%02X%02X\r\n", pUuid[0],pUuid[1],pUuid[2],pUuid[3]);
    MANCS_LOG_DEBUG("++ attr id    = %s\r\n", ancs_notif_att_id_str[p_data_source->attrId]);
    MANCS_LOG_DEBUG("++ attr len   = %d\r\n", p_data_source->attrLen);
    MANCS_LOG_DEBUG("++ attr data  = %s\r\n", p_data_source->attrData);

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
        /** msg time error, dont do any followed handle */
        if(m_timer_cmp(p_data_source->attrData) < 0)
        {
            MANCS_LOG_WARNING("msg time error!\r\n");
            MemSet(&pckMsg, 0, sizeof(packing_msg_t));
            return;
        }
        for(i = 0; i < p_data_source->attrLen; i++) pckMsg.attrIdDateData[i] = p_data_source->attrData[i];
    }
    
    if(++pckMsg.recvAttrIdFragment >= REQ_ANCS_NOTIF_ATT_ID_TOTAL)
    {
        m_ancs_business_handle(&pckMsg);
        MemSet(&pckMsg, 0, sizeof(packing_msg_t));
    }
    
    if(p_data_source->attrId == date)
        m_timer_set((uint8*)&p_data_source->attrData);
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

    MANCS_LOG_DEBUG("-- event id   = %s\r\n", ancs_event_id_str[notiSrc->evtId]);
    MANCS_LOG_DEBUG("-- event flag = %s\r\n", ancs_notif_event_flag_str[notiSrc->evtFlag]);
    MANCS_LOG_DEBUG("-- cat id     = %s\r\n", ancs_category_id_str[notiSrc->catId]);
    MANCS_LOG_DEBUG("-- cat cnt    = %d\r\n", notiSrc->catCnt);
    MANCS_LOG_DEBUG("-- uuid       = %02X%02X%02X%02X\r\n", notiSrc->uuid[0],notiSrc->uuid[1],notiSrc->uuid[2],notiSrc->uuid[3]);
    
    /** packing stage 1: pack the notif soure */
    uint8 i = 0;
    MemSet(&pckMsg, 0, sizeof(packing_msg_t));
    for(i =  0; i < 4; i++) pckMsg.uuid[i] = notiSrc->uuid[1];
    pckMsg.evtId = notiSrc->evtId;
    pckMsg.evtFlag = notiSrc->evtFlag;
    pckMsg.catId = notiSrc->catId;
    pckMsg.catCnt = notiSrc->catCnt;
    
    if(notiSrc->evtId != ancs_event_id_notif_removed)
        AncsGetNotificationAttributeCmd(p_noti_source->cid);
}

/**
* @brief sent the control point to SP to ask for data source
* @param [in] cid - current connection id
* @param [out] none
* @return none
* @data 2020/03/19 12:51
* @author maliwen
* @note none
*/
void m_ancs_get_notif_attr(uint16 cid)
{
}
