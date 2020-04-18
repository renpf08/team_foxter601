/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      ancs_service_data.c
 *
 *  DESCRIPTION
 *      This file keeps information related to Apple Notification Control
 *      Service
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *===========================================================================*/
#include <gatt.h>
#include <mem.h>
#include <gatt_uuid.h>
#include <buf_utils.h>
/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "ancs_service_data.h"
#include "discovered_ancs_service.h"
#include "app_gatt.h"
#include "ancs_client_hw.h"
#include "adapter/adapter.h"
/*============================================================================*
 *  Private Definitions
 *===========================================================================*/

/* Following macros define the offset of various fields in Notification Source
 * Characteristic.
 */
#define ANCS_NS_OFFSET_EVENT_ID                       (0)
#define ANCS_NS_OFFSET_EVENT_FLAGS                    (1)
#define ANCS_NS_OFFSET_CAT_ID                         (2)
#define ANCS_NS_OFFSET_CAT_COUNT                      (3)
#define ANCS_NS_OFFSET_NOTIF_UUID                     (4)

/* Following macro define the data source header length */
#define ANCS_DS_HDR_LEN                               (5)

/* Maximum Length of the notification attribute
 * Note: Do not increase length beyond (DEFAULT_ATT_MTU -3 = 20)
 * octets as GAP service at the moment doesn't support handling of Prepare
 * write and Execute write procedures.
 */
#define ANCS_MAX_NOTIF_ATT_LENGTH                     (20)

#define ANCSS_LOG_ERROR(...)
#define ANCSS_LOG_WARNING(...)
#define ANCSS_LOG_INFO(...)
#define ANCSS_LOG_DEBUG(...)

/*============================================================================*
 *  Private Data Types
 *===========================================================================*/

/* Structure used in decoding and displaying Attribute data
*/
typedef struct
{
    /* Attribute length - used to know the length of the attribute length */

    uint16 attr_len;

    /* One more byte pending in the attribute length */
    bool pending_len;

    /* Attribute remaining/pending length - used to know the length of the
       remaining attribute length in the new data fragment
    */
    uint16 rem_attr_len;

    /* Attribute data pending - used to know the length of the
       remaining attribute data in the new data fragment
    */
    uint16 pending_attr_data;

    /* Attribute data state - used for decoding the attribute data
    */
    uint16 ds_decoder_state;

   /* Used to identify the command processed */
    uint16 pending_cmd;

    /* Used to store the Attribute ID */
    uint16 attr_id;

}ANCS_ATTRIBUTE_DATA_T;


/* Structure used for sending Get Notification Attribute request to peer*/
typedef struct
{
    /* Data to store and send the Notification
       attribute request
     */
    uint8 data[ANCS_MAX_NOTIF_ATT_LENGTH + 1];
}ANCS_NOTIF_ATTR_CMD_T;


/* Structure used for sending Get Application Attribute request to peer */
typedef struct
{
    /* Data to store and send the Get App Attribute
       request
     */
    uint8 data[(ANCS_MAX_NOTIF_ATT_LENGTH * 2) + 1];

}ANCS_APP_ATTR_CMD_T;


/* Structure used for storing last UUID notification data */
typedef struct
{
    /* Array to store last UUID data */
    uint8 data[ANCS_NS_NOTIF_UUID_SIZE];

}ANCS_NOTIF_UUID_DATA_T;

/*============================================================================*
 *  Private Data
 *============================================================================*/
/* Attribute data used for decoding and displaying peer notification data */
static ANCS_ATTRIBUTE_DATA_T attribute_data;

/* Used for sending Get Notification Attribute request data
*/
static ANCS_NOTIF_ATTR_CMD_T notif_attr_req;

/* Used for storing the last UUID received */
static ANCS_NOTIF_UUID_DATA_T uuid_data;

/* current connection identifier */
static uint16 g_cid;

/* Buffer to hold the received ANCS data for display purpose */
uint8 g_ancs_data[ANCS_MAX_NOTIF_ATT_LENGTH + 1];

/* variable to track the last received notification */
uint16 g_last_received_notification;

/*=============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* This function handles the NS notifications */
static bool ancsHandleNotificationSourceData(GATT_CHAR_VAL_IND_T *p_ind);

/* This function handles the DS notifications */
static bool ancsHandleDataSourceData(GATT_CHAR_VAL_IND_T *p_ind);

/* This function parses and displays the data source & ANCS notification data */
static bool ancsParseData(uint8 *p_data, uint16 size_value);

/* This function initialises the Attribute data structure */
static void ancsClearAttributeData(void);

/*=============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      ancsParseData
 *
 *  DESCRIPTION
 *      This function parses and displays the DS notification data
 *
 *  RETURNS
 *      TRUE, if valid else FALSE.
 *
 *---------------------------------------------------------------------------*/
static bool ancsParseData(uint8 *p_data, uint16 size_value)
{
    uint16 count = 0;
    uint8 attrId = 0;
    uint16 i = 0;
    uint16 state = attribute_data.ds_decoder_state;
    uint16 data_len = 0;
    bool b_skip_reserved = FALSE;
    static data_source_t dataSrc;
    
    while(count < size_value)
    {
        if(b_skip_reserved)
        break;
        
        switch(state)
        {
            case ds_decoder_hdr :
            count = ANCS_DS_HDR_LEN;
            state = ds_decoder_attrid;
            MemSet(&dataSrc, 0, sizeof(data_source_t));
            for(i = 0; i < ANCS_NS_NOTIF_UUID_SIZE; i++)
            {
                dataSrc.uuid[i] = p_data[i+1];
            }
            #if !USE_MY_ANCS
            ANCSS_LOG_DEBUG("## uuid = %02X%02X%02X%02X\r\n", p_data[1], p_data[2], p_data[3], p_data[4]);
            #endif
            break;
            
            case ds_decoder_attrid :
                /* Get the Attribute ID */
                attrId = p_data[count++];
                attribute_data.attr_id = attrId;
                dataSrc.attrId = attrId;

                #if !USE_MY_ANCS
                switch(attrId)
                {
                    case ancs_notif_att_id_app_id :
                    LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_attr_id_app_id);
                    break;
                    
                    case ancs_notif_att_id_title :
                    LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_attr_id_title);
                    break;
                    
                    case ancs_notif_att_id_subtitle :
                    LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_attr_id_sub_title);
                    break;
                    
                    case ancs_notif_att_id_message :
                    LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_attr_id_message);
                    break;
                    
                    case ancs_notif_att_id_message_size :
                    LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_attr_id_message_size);
                    break;
                    
                    case ancs_notif_att_id_date :
                    LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_attr_id_date);
                    break;
                    
                    default :
                    LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_attr_id_reserved);
                    b_skip_reserved = TRUE;
                    break;
                }
                #endif
                
                if(!b_skip_reserved)
                {
                    state = ds_decoder_attrlen;
                }
                else
                {
                    state = ds_decoder_attrid;
                    
                    /* Invalid */
                    attribute_data.attr_id = 0xff;
                }
            break;
            
            case ds_decoder_attrlen :
            
                if(attribute_data.pending_len)
                {
                    /* Length was incomplete in the last fragment,
                    so the first byte will complete the length
                    data */
                    attribute_data.attr_len = ((p_data[count++]<< 8) | \
                    attribute_data.rem_attr_len);
                    attribute_data.rem_attr_len = 0;
                    attribute_data.pending_len = FALSE;
                
                }
                else if((count + 1) < size_value)
                {
                    /* Get the Attribute length ( 2 bytes size) */
                    attribute_data.attr_len = (p_data[count] |\
                    (p_data[count + 1] << 8));
                    count += 2;
                }
                else
                {
                    attribute_data.rem_attr_len = p_data[count++];
                    /* Length is 2 bytes, so copy the byte and wait for
                    the next byte in the new fragment*/
                    attribute_data.pending_len = TRUE;
                }
                
                if(!attribute_data.pending_len)
                {
                    #if !USE_MY_ANCS
                    ANCSS_LOG_DEBUG("** Attr Len = 0x%04X\r\n", attribute_data.attr_len);
                    
                    if(attribute_data.attr_len > 0)
                    {                                      //! let this work to be done bellow
                    }
                    else
                    {
                        ANCSS_LOG_DEBUG("** \r\n");
                    }
                    #endif
                    state = ds_decoder_attrdata;
                }
            break;
            
            case ds_decoder_attrdata:
            {
                /* Data was incomplete in the last fragment */
                if(attribute_data.pending_attr_data)
                {
                    /* Get the actual data length */
                    data_len = size_value - count;
                    
                    
                    /* If the received length is greater than
                    * the overall attribute length, just copy
                    * data till attribute length
                    */
                    if(attribute_data.pending_attr_data < data_len)
                    {
                        data_len  = attribute_data.pending_attr_data;
                    }
                    
                    /* Reset the g_ancs_data */
                    MemSet(g_ancs_data,0,ANCS_MAX_NOTIF_ATT_LENGTH + 1);
                    
                    /* Copy the data */
                    for(i=0;i<data_len;i++)
                    {
                        g_ancs_data[i] = p_data[count + i];
                        dataSrc.attrData[dataSrc.attrLen++] = p_data[count + i];
                    }
                    g_ancs_data[i] = '\0';

                    #ifdef ENABLE_LCD_DISPLAY
                    if(attribute_data.attr_id  == ancs_notif_att_id_subtitle)
                    {
                        WriteDataToLcdDisplay((const char*)&g_ancs_data[0],data_len,TRUE);
                    }
                    #endif /* ENABLE_LCD_DISPLAY */
                    
                    /* Update till, what we have read */
                    count += data_len;
                    attribute_data.pending_attr_data -= data_len;
                }
                else
                {
                    if(attribute_data.attr_len > 0)
                    {
                        /* Get the actual data length */
                        data_len = size_value - count;
                        
                        /* If the received length is greater than
                        * the overall attribute length, just copy
                        * data till attribute length
                        */
                        if(attribute_data.pending_attr_data < data_len)
                        {
                            data_len  = attribute_data.pending_attr_data;
                        }
                        
                        /* Reset the g_ancs_data */
                        MemSet(g_ancs_data,0,ANCS_MAX_NOTIF_ATT_LENGTH + 1);
                        
                        /* Copy the data */
                        for(i=0;i<data_len;i++)
                        {
                            g_ancs_data[i] = p_data[count + i];
                            dataSrc.attrData[dataSrc.attrLen++] = p_data[count + i];
                        }
                        
                        g_ancs_data[i] = '\0';

                        #ifdef ENABLE_LCD_DISPLAY
                        if(attribute_data.attr_id  == ancs_notif_att_id_subtitle)
                        {
                            WriteDataToLcdDisplay((const char*)&g_ancs_data[0],data_len,TRUE);
                        }
                        #endif /* ENABLE_LCD_DISPLAY */
                        
                        /* Is more data remaining? */
                        attribute_data.pending_attr_data =
                        (attribute_data.attr_len - data_len);
                        attribute_data.attr_len = 0;
                        count += data_len;
                    }   
                }
                
                if((attribute_data.pending_attr_data == 0) && (attribute_data.attr_len == 0))
                {
                    #if !USE_MY_ANCS
                    ANCSS_LOG_INFO("** Attribute Data = %s\r\n", (const char*)&dataSrc.attrData); //! be care to use packed-data here
                    #endif
                    ancs_data_source_handle(p_data, size_value, &dataSrc);
                    MemSet(&dataSrc.attrData, 0, sizeof(dataSrc.attrData));
                    dataSrc.attrLen = 0;
                    /* We are done reading data.Move to next attribute */
                    state = ds_decoder_attrid;
                    /* Invalid */
                    attribute_data.attr_id = 0xff;
                }
                break;
            }
        }
    }
    attribute_data.ds_decoder_state = state;
    
    return TRUE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      ancsHandleNotificationSourceData
 *
 *  DESCRIPTION
 *      This function handles the NS notifications
 *
 *  RETURNS
 *      TRUE, if valid else FALSE.
 *
 *---------------------------------------------------------------------------*/
static bool ancsHandleNotificationSourceData(GATT_CHAR_VAL_IND_T *p_ind)
{
    #if !USE_MY_ANCS
	bool notif_removed = FALSE;
    #endif
	uint16 curr_data = 0;
	uint16 count = 0;
	noti_t notiSrc;

	/*  Notification Source Data format
	* -------------------------------------------------------
	* |  Event  |  Event  |  Cat  |  Cat   |  Notification  |
	* |  ID     |  Flag   |  ID   |  Count |  UUID          |
	* |---------------------------------------------------- |
	* |   1B    |   1B    |   1B  |   1B   |   4B           |
	* -------------------------------------------------------
	*/
	if(p_ind->value != NULL)
	{
		g_cid = p_ind->cid;
		notiSrc.cid = p_ind->cid;

		#if !USE_MY_ANCS
		ANCSS_LOG_DEBUG("** \r\n");
		ANCSS_LOG_DEBUG("** Conn ID = %04X\r\n", g_cid);
		#endif

		/* 1st byte of the Notification - Event ID */
		curr_data = p_ind->value[ANCS_NS_OFFSET_EVENT_ID];
		notiSrc.source.evtId = p_ind->value[ANCS_NS_OFFSET_EVENT_ID];
		#if !USE_MY_ANCS
		if(curr_data == ancs_event_id_notif_added)
		{
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_event_id_added);
			#ifdef ENABLE_LCD_DISPLAY
			ClearLCDDisplay();
			#endif  /* ENABLE_LCD_DISPLAY */
		}
		else if(curr_data == ancs_event_id_notif_modified)
		{
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_event_id_modified);
			#ifdef ENABLE_LCD_DISPLAY
			ClearLCDDisplay();
			#endif  /* ENABLE_LCD_DISPLAY */
		}
		else if(curr_data == ancs_event_id_notif_removed)
		{
			notif_removed = TRUE;
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_event_id_removed);
		}
		else
		{
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_event_id_reserved);
		}
		#endif

		/* 2nd byte of the Notification- Event Flags */
		curr_data = p_ind->value[ANCS_NS_OFFSET_EVENT_FLAGS];
		notiSrc.source.evtFlag = p_ind->value[ANCS_NS_OFFSET_EVENT_FLAGS];
		#if !USE_MY_ANCS
		if(curr_data == ANCS_NS_EVENTFLAG_SILENT)
		{
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_event_flags_silent);
		}
		else if(curr_data == ANCS_NS_EVENTFLAG_IMPORTANT)
		{
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_event_flags_important);
		}
		else /* Reserved */
		{
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_event_flags_reserved);
		}
		#endif

		/* 3rd byte of the Notification - Cat ID */
		curr_data = p_ind->value[ANCS_NS_OFFSET_CAT_ID];
		notiSrc.source.catId = p_ind->value[ANCS_NS_OFFSET_CAT_ID];
		#if !USE_MY_ANCS
		switch(curr_data)
		{
			case ancs_cat_id_other:
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_cat_id_other);
			break;

			case ancs_cat_id_incoming_call:
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_cat_id_incoming_call);

			#ifdef ENABLE_LCD_DISPLAY
			if(!notif_removed)
			{
				WriteDataToLcdDisplay("Incoming call",StrLen("Incoming call"),FALSE);
			}
			#endif /* ENABLE_LCD_DISPLAY */
			break;

			case ancs_cat_id_missed_call:
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_cat_id_missed_call);
			#ifdef ENABLE_LCD_DISPLAY
			if(!notif_removed)
			{
				WriteDataToLcdDisplay("Missed call",StrLen("Missed call"),FALSE);
			}
			#endif /* ENABLE_LCD_DISPLAY */
			break;

			case ancs_cat_id_vmail:
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_cat_id_vmail);
			#ifdef ENABLE_LCD_DISPLAY
			if(!notif_removed)
			{
				WriteDataToLcdDisplay("vmail",StrLen("vmail"),FALSE);
			}
			#endif /* ENABLE_LCD_DISPLAY */
			break;

			case ancs_cat_id_social:
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_cat_id_social);
			#ifdef ENABLE_LCD_DISPLAY
			if(!notif_removed)
			{
				WriteDataToLcdDisplay("Message",StrLen("Message"),FALSE);
			}
			#endif /* ENABLE_LCD_DISPLAY */
			break;

			case ancs_cat_id_schedule:
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_cat_id_schedule);
			#ifdef ENABLE_LCD_DISPLAY
			if(!notif_removed)
			{
				WriteDataToLcdDisplay("schedule",StrLen("schedule"),FALSE);
			}
			#endif /* ENABLE_LCD_DISPLAY */
			break;

			case ancs_cat_id_email:
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_cat_id_email);
			#ifdef ENABLE_LCD_DISPLAY
			if(!notif_removed)
			{
				WriteDataToLcdDisplay("email",StrLen("email"),FALSE);
			}
			#endif /* ENABLE_LCD_DISPLAY */
			break;

			case ancs_cat_id_news:
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_cat_id_news);
			#ifdef ENABLE_LCD_DISPLAY
			if(!notif_removed)
			{
				WriteDataToLcdDisplay("news",StrLen("news"),FALSE);
			}
			#endif /* ENABLE_LCD_DISPLAY */
			break;

			case ancs_cat_id_hnf:
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_cat_id_hnf);
			#ifdef ENABLE_LCD_DISPLAY
			if(!notif_removed)
			{
				WriteDataToLcdDisplay("hnf",StrLen("hnf"),FALSE);
			}
			#endif /* ENABLE_LCD_DISPLAY */
			break;

			case ancs_cat_id_bnf:
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_cat_id_buf);
			#ifdef ENABLE_LCD_DISPLAY
			if(!notif_removed)
			{
				WriteDataToLcdDisplay("Business and Finance",StrLen("Business and Finance"),FALSE);
			}
			#endif /* ENABLE_LCD_DISPLAY */
			break;

			case ancs_cat_id_location:
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_cat_id_location);
			#ifdef ENABLE_LCD_DISPLAY
			if(!notif_removed)
			{
				WriteDataToLcdDisplay("location",StrLen("location"),FALSE);
			}
			#endif /* ENABLE_LCD_DISPLAY */
			break;

			case ancs_cat_id_entertainment:
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_cat_id_entertainment);
			#ifdef ENABLE_LCD_DISPLAY
			if(!notif_removed)
			{
				WriteDataToLcdDisplay("entertainment",StrLen("entertainment"),FALSE);
			}
			#endif /* ENABLE_LCD_DISPLAY */
			break;
			default:
            LogReport(__FILE__, __func__, __LINE__, Ancs_Service_Data_cat_id_reserved);
			break;
		}

		#ifdef ENABLE_LCD_DISPLAY
		if(notif_removed)
		{
			if( curr_data == g_last_received_notification )
			{
				ClearLCDDisplay();
			}
		}
		#endif /* ENABLE_LCD_DISPLAY */
		#endif /** !USE_MY_ANCS */

		/* 4rd byte of the Notification - Cat Count */
        #if !USE_MY_ANCS
		ANCSS_LOG_DEBUG("** Cat Count = %d\r\n", p_ind->value[ANCS_NS_OFFSET_CAT_COUNT]);
        #endif
		notiSrc.source.catCnt = p_ind->value[ANCS_NS_OFFSET_CAT_COUNT];

		/* Clear the UUID notification buffer */
		MemSet(uuid_data.data,0,ANCS_NS_NOTIF_UUID_SIZE);

		/* 5th to 8th bytes (4 bytes) of the Notification-Notification UUID */
		for(count = 0;count < ANCS_NS_NOTIF_UUID_SIZE;count++)
		{
			uuid_data.data[count] = p_ind->value[ANCS_NS_OFFSET_NOTIF_UUID+count];
            notiSrc.source.uuid[count] = p_ind->value[ANCS_NS_OFFSET_NOTIF_UUID+count];
		}

		#if !USE_MY_ANCS
		ANCSS_LOG_DEBUG("** UUID = %02X%02X%02X%02X\r\n", uuid_data.data[0],uuid_data.data[1],uuid_data.data[2],uuid_data.data[3]);
		g_last_received_notification = curr_data;
		#endif
        
        ancs_noti_source_handle(p_ind, &notiSrc);
        
        #if 0
		if(!notif_removed) //! modified by mlw at 20200319 09:51
		{
			/* Send Notification Attribute Request */
			AncsGetNotificationAttributeCmd(g_cid);
		}
        #endif
	}

	return TRUE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      ancsHandleDataSourceData
 *
 *  DESCRIPTION
 *      This function handles the DS notifications
 *
 *  RETURNS
 *      TRUE, if valid else FALSE.
 *
 *---------------------------------------------------------------------------*/
static bool ancsHandleDataSourceData(GATT_CHAR_VAL_IND_T *p_ind)
{
    uint16 count = 0;
    uint16 state = attribute_data.ds_decoder_state;

    /* if cmd id = 1*/
    /* parse the data */
    if(attribute_data.pending_cmd == ancs_cmd_get_notification_att)
    {
        if(((p_ind->value[count] == ancs_cmd_get_notification_att) &&
           (attribute_data.pending_attr_data == 0) &&
           (attribute_data.attr_len == 0) && (!attribute_data.pending_len)))
        {
            if((state == ds_decoder_hdr ) || (state == ds_decoder_attrid ))
            {
                attribute_data.ds_decoder_state = ds_decoder_hdr;
            }
        }
        ancsParseData(p_ind->value,p_ind->size_value);
    }
    else
    {
    }
    return TRUE;
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      ancsCheckCharUuid
 *
 *  DESCRIPTION
 *      This function checks if the UUID provided is one of the ANCS
 *      Characteristics
 *  RETURNS
 *       ancs_type
 *
 *---------------------------------------------------------------------------*/

static void ancsClearAttributeData(void)
{
    /* Reset the attribute data */
    attribute_data.ds_decoder_state = ds_decoder_hdr;
    attribute_data.attr_len = 0;
    attribute_data.pending_len = FALSE;
    attribute_data.rem_attr_len = 0;
    attribute_data.pending_attr_data = 0;
}

/*============================================================================*
 *  Public Function Implementations
 *===========================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      ancsServiceDataInit
 *
 *  DESCRIPTION
 *      This function initialises the Apple Notification Control Service data
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern void AncsServiceDataInit(void)
{
    /* Reset the Attribute data used */
    ancsClearAttributeData();
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AncsGetNotificationAttributeCmd
 *
 *  DESCRIPTION
 *      This function sends the Get Notification Attributes command to the
 *      peer device based on the UUID  and features requested.
 *
 *  RETURNS
 *     Nothing
 *
 *---------------------------------------------------------------------------*/

extern void AncsGetNotificationAttributeCmd(uint16 cid)
{
    /**
      * NOTE: there is many bugs in the official template!!!
      * fixed by mlw, 20200319 15:05
      */
    
    uint16 count = 0;
    uint16 loop_count = 0;
    uint8 *value = NULL;
    /* "ancs_notif_att_id_app_id" is not added as it is sent by default by the IOS
    * device
    * bug: actually,"ancs_notif_att_id_app_id" did not sent by default, need
    * to add manually!!! by mlw, 20200318 14:15
    */
    uint16 features =(  (1<<ancs_notif_att_id_app_id)*REQ_ANCS_NOTIF_ATT_ID_APP_ID|
                        (1<<ancs_notif_att_id_title)*REQ_ANCS_NOTIF_ATT_ID_TITLE |
                        (1<<ancs_notif_att_id_subtitle)*REQ_ANCS_NOTIF_ATT_ID_SUBTITLE |
                        (1<<ancs_notif_att_id_message)*REQ_ANCS_NOTIF_ATT_ID_MESSAGE |
                        (1<<ancs_notif_att_id_message_size)*REQ_ANCS_NOTIF_ATT_ID_MESSAGE_SIZE |
                        (1<<ancs_notif_att_id_date)*REQ_ANCS_NOTIF_ATT_ID_DATE);
    
    /* Clear the buffer each time */
    MemSet(notif_attr_req.data,0,ANCS_MAX_NOTIF_ATT_LENGTH + 1);
    
    value = &notif_attr_req.data[0];
    
    /* Fill in the command -id (0) for the Notification Attribute request */
    value[count++] = ancs_cmd_get_notification_att;
    
    /* Copy the Notification UUID */
    for(loop_count = 0;loop_count < ANCS_NS_NOTIF_UUID_SIZE;loop_count++)
    {
        value[loop_count + count] = uuid_data.data[loop_count];
    }
    
    /* Add the length of UUID */
    count += ANCS_NS_NOTIF_UUID_SIZE;
    
    /* "ancs_notif_att_id_app_id" is sent by default, so no need to request the
    * attribute separately. Attribute App Identifier has no length to be filled
    * in
    * bug: actually,"ancs_notif_att_id_app_id" did not sent by default, need
    * to add manually!!! by mlw, 20200318 14:15
    */
    if(features & (1<<ancs_notif_att_id_app_id))
    {
        /* Add ancs_notif_att_id_app_id */  
        value[count++] = ancs_notif_att_id_app_id;
    }
    
    if(features & (1<<ancs_notif_att_id_title))
    {
        /* Add Attribute ID for Title */
        value[count++] = ancs_notif_att_id_title;
        
        /* Add Attribute size for Title - 0x14 bytes requested */
        value[count++] = 0x14;
        value[count++] = 0;
    }
    
    if(features & (1<<ancs_notif_att_id_subtitle))
    {
        /* Add Attribute ID for Sub title */
        value[count++] = ancs_notif_att_id_subtitle;
        
        /* Attribute size - 0x14 bytes requested */
        value[count++] = 0x14;
        value[count++] = 0;
    }
    
    if(features & (1<<ancs_notif_att_id_message))
    {
        /* Add Attribute ID for Message */
        value[count++] = ancs_notif_att_id_message;
        
        /** bug: needs to be followed by a 2-bytes max length, the official 
        *        template never do this!!!
        *   add by mlw, 20200319 15:11
        */
        /* Attribute size - 0x14 bytes requested */
        value[count++] = 0x14;
        value[count++] = 0;
    }
    
    if(features & (1<<ancs_notif_att_id_message_size))
    {
        /* Add Attribute ID for Message size */
        value[count++] = ancs_notif_att_id_message_size;
    }
    
    if(features & (1<<ancs_notif_att_id_date))
    {
        /* Add Attribute ID - Date is sent as UTF-35# support
        Date format : YYYYMMDD'T'HHMMSS
        */
        value[count++] = ancs_notif_att_id_date;
    }

    /* Notification Attribute Request format
    * -------------------------------------------------------------------------
    * |  CMD    |  Notification  |  Attr    |  Attr  |   Attr  |  Attr   | Attr|
    * |  ID(0)  |  UUID          |  ID-1    |  ID-2  |   Len   |  ID-n   | Len |
    * |         |                | (App ID) |        |         |         |     |
    * |------------------------------------------------------------------------
    * |   1B    |      4B        |   1B     |   1B   |   2B    |    1B   |  2B |
    * --------------------------------------------------------------------------
    */
    AncsWriteRequest(ancs_control_point, &notif_attr_req.data[0], count,cid);
    
    /* Set the command requested for */
    attribute_data.pending_cmd = ancs_cmd_get_notification_att;
}

/*-----------------------------------------------------------------------------
 *  NAME
 *      ConfigureAncsNotifications
 *
 *  DESCRIPTION
 *      This function configures indications on the ANCS Notification
 *      Characteristics.
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern sys_status ConfigureAncsNotifications(uint16 ucid)
{
    uint8 val[2], *p_val;
    p_val = val;
    BufWriteUint16(&p_val, gatt_client_config_notification);

    /* Store the ANCS Notification Characteristic Client Configuration in
     * the temporary handle. Write confirmation event does not contain any
     * handle value. This handle will tell the application which attribute was
     * being written.
     */
    SetTempReadWriteHandle(GetAncsNotificationCCDHandle());

    return GattWriteCharValueReq(ucid,
                          GATT_WRITE_REQUEST,
                          GetAncsNotificationCCDHandle(),
                          2,
                          val);
}

/*-----------------------------------------------------------------------------
 *  NAME
 *      ConfigureAncsDataSourceNotification
 *
 *  DESCRIPTION
 *      This function configures indications on the ANCS Data Source
 *      Notification Characteristics.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern sys_status ConfigureAncsDataSourceNotification(uint16 ucid)
{
    uint8 val[2], *p_val;
    p_val = val;
    BufWriteUint16(&p_val, gatt_client_config_notification);

    /* Store the ANCS data source notification characteristic client
     * configuration in the temporary handle. Write confirmation event does not
     * contain any  handle value. This handle will tell the application
     * which attribute was being written.
     */
    SetTempReadWriteHandle(GetAncsDataSourceCCDHandle());

    return GattWriteCharValueReq(ucid,
                          GATT_WRITE_REQUEST,
                          GetAncsDataSourceCCDHandle(),
                          2,
                          val);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AncsHandlerNotifInd
 *
 *  DESCRIPTION
 *      This function handles all the registered notifications for ANCS service
 *
 *  RETURNS
 *      TRUE, if successful else FALSE
 *
 *---------------------------------------------------------------------------*/
bool AncsHandlerNotifInd(GATT_CHAR_VAL_IND_T *p_ind)
{

    if(p_ind->handle == GetAncsNotificationHandle())
    {  /* Notification has arrived */
       ancsHandleNotificationSourceData(p_ind);
       return TRUE;
    }
    else if(p_ind->handle == GetAncsDataSourceHandle())
    {
       /* Detailed data about the previous notification has arrived */
       ancsHandleDataSourceData(p_ind);
       return TRUE;
    }
    else
    {
       return FALSE;
    }
}

/*-----------------------------------------------------------------------------
 *  NAME
 *      DoesHandleBelongsToAncsService
 *
 *  DESCRIPTION
 *      This function checks if the passed handle belongs to ANCS Service.
 *
 *  RETURNS
 *      TRUE if it belongs to ANCS Service
 *
 *----------------------------------------------------------------------------*/
extern bool DoesHandleBelongsToAncsService(uint16 handle)
{
    return ((handle >= GetRemoteDiscAncsServiceStartHandle()) &&
            (handle <= GetRemoteDiscAncsServiceEndHandle()))
            ? TRUE : FALSE;
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      AncsWriteRequest
 *
 *  DESCRIPTION
 *      This function initiates a GATT Write procedure for the characteristic
 *      that supports Write procedure
 *
 *  RETURNS
 *      TRUE, if successful else FALSE
 *
 *---------------------------------------------------------------------------*/
bool AncsWriteRequest(uint16 type, uint8 *data, uint16 size,uint16 cid)
{
    GattWriteCharValueReq(cid,
                          GATT_WRITE_REQUEST,
                          GetAncsControlPointHandle(),size,
                          data);
    return TRUE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetCid
 *
 *  DESCRIPTION
 *      This function provides the connection id
 *
 *  RETURNS
 *      TRUE, if successful else FALSE
 *
 *---------------------------------------------------------------------------*/
extern uint16 GetCid(void)
{
    return g_cid;
}