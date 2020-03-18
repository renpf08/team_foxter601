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

#define MANCS_LOG_ERROR(...)        M_LOG_ERROR(__VA_ARGS__)
#define MANCS_LOG_WARNING(...)      M_LOG_WARNING(__VA_ARGS__)
#define MANCS_LOG_INFO(...)         M_LOG_INFO(__VA_ARGS__)
#define MANCS_LOG_DEBUG(...)        M_LOG_DEBUG(__VA_ARGS__)

/* enum for event id */
typedef enum 
{
    added     = 0x0,
    modified,
    removed,
    reserved1
}event_id_str;

/* enum for category id */
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

/* enum for notification attribute id */
typedef enum
{
    appId = 0x0,
    title,
    subtitle,
    message,
    messageSize,
    date
}att_id_str;

/* enum for notification event flag */
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
    uint8 *pUuid = p_data_source->uuid;
    MANCS_LOG_DEBUG("++ uuid       = %02X%02X%02X%02X\r\n", pUuid[0],pUuid[1],pUuid[2],pUuid[3]);
    MANCS_LOG_DEBUG("++ attr id    = %s\r\n", ancs_notif_att_id_str[p_data_source->attrId]);
    MANCS_LOG_DEBUG("++ attr len   = %d\r\n", p_data_source->attrLen);
    MANCS_LOG_DEBUG("++ attr data  = %s\r\n", p_data_source->attrData);
}

/**
* @brief ANCS通知源处理
* @param [in] p_ind - ancs通知回调指针; p_data - uuid used to ask for data source
* @param [out] none
* @return none
* @data 2020/03/17 15:22
* @author maliwen
* @note none
*/
void m_ancs_noti_source_handle(GATT_CHAR_VAL_IND_T *p_ind, uint8 *p_data)
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
    
    uint8 i = 0;
    noti_t noti;
    noti.cid = p_ind->cid;
    
    /** copy notification source msg to struct */
    for(i = 0; i < sizeof(source_t); i++)
    {
        ((uint8*)&noti.source)[i] = p_ind->value[i];
    }
    
    /** if noti.source.evtFlag is other value than 1 and 2, then just set it to 3 */
    noti.source.evtFlag = ((noti.source.evtFlag>2)||(noti.source.evtFlag<1))?3:noti.source.evtFlag;
    
    /** uuid_data.data is used when ask for data source */
    MemSet(p_data,0,ANCS_NS_NOTIF_UUID_SIZE);
    for(i = 0; i < ANCS_NS_NOTIF_UUID_SIZE; i++)
    {
        p_data[i] = noti.source.uuid[i];
    }

    uint8 *pUuid = noti.source.uuid;
    MANCS_LOG_DEBUG("-- event id   = %s\r\n", ancs_event_id_str[noti.source.evtId]);
    MANCS_LOG_DEBUG("-- event flag = %s\r\n", ancs_notif_event_flag_str[noti.source.evtFlag]);
    MANCS_LOG_DEBUG("-- cat id     = %s\r\n", ancs_category_id_str[noti.source.catId]);
    MANCS_LOG_DEBUG("-- cat cnt    = %d\r\n", noti.source.catCnt);
    MANCS_LOG_DEBUG("-- uuid       = %02X%02X%02X%02X\r\n", pUuid[0],pUuid[1],pUuid[2],pUuid[3]);
    
    if(noti.source.evtId != ancs_event_id_notif_removed)
        AncsGetNotificationAttributeCmd(noti.cid);
}
