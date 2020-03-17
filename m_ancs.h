
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

void m_ancs_noti_source_handle(GATT_CHAR_VAL_IND_T *p_ind);

#endif /** end of __M_ANCS_H__ */
