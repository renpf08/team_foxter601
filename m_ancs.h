
#ifndef __M_ANCS_H__
#define __M_ANCS_H__

/**
*  @file    m_ancs.h
*  @author  maliwen
*  @date    2020/03/17
*  @history 1.0.0.0(°æ±¾ºÅ)
*           1.0.0.0: Creat              2020/03/17
*/

#include "discovered_gatt_service.h"

/*  Notification Source Data format
* -------------------------------------------------------
* |  Event  |  Event  |  Cat  |  Cat   |  Notification  |
* |  ID     |  Flag   |  ID   |  Count |  UUID          |
* |---------------------------------------------------- |
* |   1B    |   1B    |   1B  |   1B   |   4B           |
* -------------------------------------------------------
*/
typedef struct
{
    uint8 evtId;
    uint8 evtFlag;
    uint8 catId;
    uint8 catCnt;
    uint8 uuid[4];
} source_t;

typedef struct
{
    uint8 cid;
    source_t source;
} noti_t;

typedef struct
{
    uint8 uuid[4];
    uint8 attrId;
    uint16 attrLen;
    uint8 attrData[32];
} data_source_t;

typedef struct
{
    uint8 recvAttrIdFragment;       //! receive msg count, total fragment = REQ_ANCS_NOTIF_ATT_ID_TOTAL
    uint8 uuid[4];                  //! message UUID
    uint8 evtId;                    //! added, modified, removed, reserved
    uint8 evtFlag;                  //! silent, important, reserved
    uint8 catId;                    //! 0other,1incomingCall,2missedCall,3vmail,4social,5schedule,6email,7news,8health,9bussiness,10location,11entertainment,12~255reserved
    uint8 catCnt;                   //! message count indicated by catId
    uint8 attrIdAppIdData[32];      //! app name, e.g. QQ->com.tencent.mqq
    uint8 attrIdTitleData[32];      //! contact name
    uint8 attrIdSubTitleData[32];   //! sub contact name
    uint8 attrIdMessageData[32];    //! message content
    uint16 attrIdMessageSize;       //! message length
    uint8 attrIdDateData[32];       //! date and time that the message was received
} packing_msg_t;

void m_ancs_data_source_handle(uint8 *p_data, uint16 size_value, data_source_t *p_data_source);
void m_ancs_noti_source_handle(GATT_CHAR_VAL_IND_T *p_ind, noti_t *p_noti_source);
void m_ancs_get_notif_attr(uint16 cid);

#endif /** end of __M_ANCS_H__ */
