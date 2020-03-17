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

/**
* @brief ANCS通知源处理
* @param [in] p_ind - ancs通知回调指针
* @param [out] none
* @return none
* @data 2020/03/17 15:22
* @author maliwen
* @note none
*/
void m_ancs_noti_source_handle(GATT_CHAR_VAL_IND_T *p_ind)
{
    return;
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
    uint8 evtIdBuf[4][9] = {{"Added\0"},{"Modified\0"},{"Removed\0"},{"Reserved\0"}};
    uint8 evtFlagBuf[4][10] = {{"\0"},{"Silent\0"},{"Important\0"},{"Reserved\0"}};
    uint8 catId[14][10] = {{"other\0"},{"incall\0"},{"miscall\0"},{"vicmail\0"},
                       {"social\0"},{"schedule\0"},{"email\0"},{"news\0"},
                       {"news\0"},{"hnf\0"},{"bnf\0"},{"location\0"},
                       {"entertamn\0"},{"reserved\0"}};
    noti_t noti;
    noti.cid = p_ind->cid;
    for(i = 0; i < sizeof(source_t); i++)
    {
        ((uint8*)&noti.source)[i] = p_ind->value[i];
    }
    noti.source.evtFlag = ((noti.source.evtFlag>2)||(noti.source.evtFlag<1))?3:noti.source.evtFlag;

    MANCS_LOG_DEBUG("-- event id = %s\r\n", evtIdBuf[noti.source.evtId]);
    MANCS_LOG_DEBUG("-- event flag = %s\r\n", evtFlagBuf[noti.source.evtFlag]);
    MANCS_LOG_DEBUG("-- cat id = %s\r\n", catId[noti.source.catId]);
    MANCS_LOG_DEBUG("-- cat cnt = %d\r\n", noti.source.catCnt);
    if(noti.source.evtId != ancs_event_id_notif_removed)
        MANCS_LOG_DEBUG("-- uuid = %02X%02X%02X%02X\r\n", noti.source.uuid[0],
                        noti.source.uuid[1],noti.source.uuid[2],noti.source.uuid[3]);
}
