
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

#define MAX_LENGTH_APPID    25
#define MAX_LENGTH_ATTTDATA 32

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
    uint8 attrData[MAX_LENGTH_ATTTDATA];
} data_source_t;

typedef struct
{
    uint8 recvAttrIdFragment;                       //! receive msg count, total fragment = REQ_ANCS_NOTIF_ATT_ID_TOTAL
    uint8 uuid[4];                                  //! message UUID
    uint8 evtId;                                    //! added, modified, removed, reserved
    uint8 evtFlag;                                  //! silent, important, reserved
    uint8 catId;                                    //! 0other,1incomingCall,2missedCall,3vmail,4social,5schedule,6email,7news,8health,9bussiness,10location,11entertainment,12~255reserved
    uint8 catCnt;                                   //! message count indicated by catId
    uint8 attrIdAppIdData[MAX_LENGTH_ATTTDATA];     //! app name, e.g. QQ->com.tencent.mqq
    uint8 attrIdTitleData[MAX_LENGTH_ATTTDATA];     //! contact name
    uint8 attrIdSubTitleData[MAX_LENGTH_ATTTDATA];  //! sub contact name
    uint8 attrIdMessageData[MAX_LENGTH_ATTTDATA];   //! message content
    uint16 attrIdMessageSize;                       //! message length
    uint8 attrIdDateData[MAX_LENGTH_ATTTDATA];      //! date and time that the message was received
} packing_msg_t;

/** reference to the android notif packet mode */
typedef struct
{
    uint8 cmd; //! fixed to 0x07
    uint8 sta; //! fixed to: 0:added, 1:modified, 2:removed
    uint8 level; //! 0~255, look appMsgList[] of MESSAGE_POSITION_xxx for details
    uint8 type; //! look appMsgList[] of APP_ID_STRING_xxx's index for details
    uint8 cnt; //! msg count
} protocol_msg_t;

void m_ancs_data_source_handle(uint8 *p_data, uint16 size_value, data_source_t *p_data_source);
void m_ancs_noti_source_handle(GATT_CHAR_VAL_IND_T *p_ind, noti_t *p_noti_source);

#endif /** end of __M_ANCS_H__ */
