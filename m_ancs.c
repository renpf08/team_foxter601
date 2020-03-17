/**
*  @file    m_ancs.c
*  @author  maliwen
*  @date    2020/03/17
*  @history 1.0.0.0(�汾��)
*           1.0.0.0: Creat              2020/03/17
*/

#include "m_ancs.h"


/**
* @brief ANCS֪ͨԴ����
* @param [in] p_ind - ancs֪ͨ�ص�ָ��
* @param [out] none
* @return none
* @data 2020/03/17 15:22
* @author maliwen
* @note none
*/
void m_ancs_noti_source_handle(GATT_CHAR_VAL_IND_T *p_ind)
{
    /*  Notification Source Data format
     * -------------------------------------------------------
     * |  Event  |  Event  |  Cat  |  Cat   |  Notification  |
     * |  ID     |  Flag   |  ID   |  Count |  UUID          |
     * |---------------------------------------------------- |
     * |   1B    |   1B    |   1B  |   1B   |   4B           |
     * -------------------------------------------------------
     */
    //noti_source_t notiSrc;
    
    if(p_ind->value == NULL)
    {
        return;
    }
}
