#include <debug.h>          /* Simple host interface to the UART driver */
#include <mem.h>
#include "../adapter.h"

#define PRO_STEP_IDLE       0
#define PRO_STEP_START      1
#define PRO_STEP_RISING     2
#define PRO_STEP_STOP       3     /*jim add 20150819¹Øµô¼Æ²½´¦Àí*/

#define SAMPLE_TIME_UNINT   40         /*²ÉÑùÊ±¼äµ¥Î»£¬40ms*/

#define FILTER_NUM          8          /*ÂË²¨Êý¾ÝÊý£¬8¸öÊý¾ÝÆ½¾ù£¬È¥µôÍ»²¨*/
#define ONE_DATA_NUM        4          /*4¸öÊýÇóÆ½¾ùÖµ*/
#define ALL_DATA_NUM        16         /*16×éÊýÇóÆ½¾ùÖµ*/
#define DATA_DELAY_NUM      8          /*15-7,16×éÊý¾ÝÌáÇ°7¸öÊý¾ÝÖµ*/

#define ASLEEP_MOVE_OFFSET_MIN_VAL      1000       /*Ë¯Ãß²¿·ÖµÄÔË¶¯·§Öµ*/
#define ASLEEP_ONE_STEP_PERIOD_MAX_TIME (1000/SAMPLE_TIME_UNINT)    /*Ë¯Ãß²¿·ÖÔË¶¯²¨·å×î³¤Ê±¼ä1Ãë£¬1000ms/40ms=25 */

#define QUICKLY_STEP_MIN_VAL            16000/* 8000 */   /*Ë¯ÃßÔË¶¯×îÐ¡¼ÓËÙ·§Öµ£¬´óÓÚ´ËÖµÈÏÎªÊÇÔÚ¿ì×ß*/
#define QUICKLY_STEP_OFFSET_MIN_VAL     2000 /*2400*/    /*¿ì²½ÅÜµÄÆ«ÒÆÖµ·§Öµ2500*/
#define STEP_OFFSET_MIN_VAL             1100 /* 1600*/ /*1200 */       /*Âý×ßÊ±¹²Ä£¼ÓËÙ¶ÈÖµ×îÐ¡Æ«²îÖµ1300*/
#define START_STEP_OFFSET_MIN_VAL       950 /* 1600*/ /*1200 */       /*Âý×ßÊ±¹²Ä£¼ÓËÙ¶ÈÖµ×îÐ¡Æ«²îÖµ1300*/

#define QUICKLY_STEP_TIME_MAX           (250/SAMPLE_TIME_UNINT)/*(200/SAMPLE_TIME_UNINT)(300/SAMPLE_TIME_UNINT) */   /*¿ì×ß×î´ó²¨·åÊ±¼äÊÇ0.2Ãë£¬200ms/40ms=5*/
#define SLOW_STEP_TIME_MIN              (200/SAMPLE_TIME_UNINT)/* //(250/SAMPLE_TIME_UNINT)*/ /*(400/SAMPLE_TIME_UNINT)*/    /*Âý×ßÁ½²½Ö®¼äµÄ×îÐ¡Ê±¼ä¼ä¸ôÊÇ0.48Ãë£¬480ms/40ms=12*/

#define SLOW_ONE_STEP_PERIOD_MIN_TIME   (120/SAMPLE_TIME_UNINT)/*//(160/SAMPLE_TIME_UNINT)*/  /*Ò»²½Âý×ßÊ±×î¶ÌµÄ²¨·åÊ±¼äÊÇ160ms*/
#define ONE_STEP_PERIOD_MAX_TIME        (1000/SAMPLE_TIME_UNINT) /*(1000/SAMPLE_TIME_UNINT)*/  /*Ò»²½×î³¤µÄ²¨·åÊ±¼äÊÇ1Ãë£¬1000ms/40ms=25*/
    
#define TWO_STEP_BETWEEN_MIN_TIME       (120/SAMPLE_TIME_UNINT)    /*Á½²½Ö®¼ä×îÐ¡Ê±¼ä²î0.12Ãë 120ms/40ms=3 */
#define TWO_STEP_BETWEEN_MAX_TIME       (1200/SAMPLE_TIME_UNINT)/*//(1200/SAMPLE_TIME_UNINT)*/ /*(1000/SAMPLE_TIME_UNINT)*/ /*(1200/SAMPLE_TIME_UNINT)*/  /*Á½²½Ö®¼ä×î´óÊ±¼ä²î1.0Ãë  1600*40ms=40 */

#define HOW_MANY_SECOND_BUFFER          (4800/SAMPLE_TIME_UNINT) /*  (4000/SAMPLE_TIME_UNINT)*/   /*4ÃëÄÚÍê³É5²½µÄ´¦Àí*/
#define HOW_MANY_STEP_BUFFER            7  /* 6 */      /*¶¯Ç°¼¸²½¹ýÂË£¬ÖÁÉÙÒª´óÓÚ3*/

#define MIDDLE_STEP_COUNT               4        /*¼õÉÙÎó²î£¬Ç°1.8ÃëÄÚÒªÓÐÖÁÉÙ3²½*/
#define MIDDLE_STEP_SECOND              (2800/SAMPLE_TIME_UNINT)   /*1.8ÃëµÄÊ±ºòÒªÖÁÉÙ3²½µÄÖµ2.8*/

/***************************************************************************************/
/************************************ÔË¶¯Êý¾Ý³õÊ¼»¯****************************************/
/***************************************************************************************/
typedef struct
{
    u32   StepCounts;           /*×Ü²½Êý ²½*/
    u32   Distance;             /*×Ü¾àÀë Ã×*/
    u32   Calorie;              /*×Ü¿¨Â·Àï Ç§¿¨*/
    u16   FloorCounts;          /*ÅÀÂ¥Êý ²ã*/
    u16   AcuteSportTimeCounts; /*¾çÁÒÔË¶¯Ê±¼ä ·ÖÖÓ*/
}SPORT_INFO_T;  /*ÔË¶¯Êý¾Ý½á¹¹*/

SPORT_INFO_T Total_Sport_Info_data;
SPORT_INFO_T One_Minute_Sport_Info_data;      /*Ò»·ÖÖÓÊý¾ÝÍ³¼Æ½áÊø*/
SPORT_INFO_T Fifteen_Minutes_Sport_Info_data; /*Ê®Îå·ÖÖÓÊý¾ÝÍ³¼Æ*/

/***************************************************************************************/
/**********************************Ê®Îå·ÖÖÓË¯ÃßÐÅÏ¢´¦Àí*************************************/
/***************************************************************************************/
typedef struct
{
     u16 Year;
     u8  Month;
     u8  Date;
     u8  Hour;
     u8  Minute;  
     u32 FifteenMinuteMove;  /*Ê®Îå·ÖÖÓµÄÔË¶¯Á¿*/
	 u16 AsleepInfo_Data_Table[12]; /*Ò»ÌìË¯ÃßÐÅÏ¢´æ´¢±í*/
}ASLEEP_DATA_INFO_T;  /*Ë¯ÃßÊ±¼äµãÐÅÏ¢Êý¾Ý½á¹¹*/

ASLEEP_DATA_INFO_T Asleep_Data_Info;

/***************************************************************************************/
/************************************¾çÁÒÔË¶¯ÔËËã²¿·Ö**************************************/
/***************************************************************************************/
#define ACUTE_MEASURE_OVER_TIME_VALUE     (5000/SAMPLE_TIME_UNINT)/* 300 */     /*¾çÁÒÔË¶¯£¬Á½²½Ö®¼äÎÞ±ä»¯µÄÊ±¼ä£¬6Ãë----´ËÖµ¸ù¾Ý²ÉÑùÊ±¼äµ÷Õû*/ 

#define ACUTE_MEASURE_MINI_STEP_VALUE      100      /*1·ÖÖÓÖÁÉÙÒªÔË¶¯120²½²ÅËã¾çÁÒÔË¶¯*/
typedef struct
{    
    u8   AcuteSportTimeProState;               /*²âËã²½Öè×´Ì¬*/
    u16   OverTimeCount;                        /*²âËã³¬Ê±¼ÆÊýÆ÷*/
    u8   MinuteTimeCount;                      /*·ÖÖÓ¼ÆÊýÆ÷*/
    u16  StepCountAcute;                       /*¿ªÊ¼²â¾çÁÒÔË¶¯²½ÊýÀÛ¼Æ*/
}ACUTE_SPORT_TIME_COUNT_T;
/*¾çÁÒÔË¶¯²âËã²¿·Ö*/
ACUTE_SPORT_TIME_COUNT_T  Acute_Sport_Time_Count_data;

typedef struct
{
     u16  FiFo_DataString_Val[ALL_DATA_NUM];           /*ÈýÖá¼ÓËÙ¶ÈÁ¬Ðø16×é¹²Ä£Öµ*/
     u16  FiFo_OneData_Val[ONE_DATA_NUM];              /*ÈýÖá¼ÓËÙ¶ÈÁ¬Ðø4¸ö¹²Ä£Öµ*/
     u16  FiFo_Filter_Val[FILTER_NUM];                 /*ÈýÖá¼ÓËÙ¶ÈÁ¬Ðø4¸ö¹²Ä£Öµ*/
     u16  XYZ_Across_Acce_Min;                  /*XYZÈ¡Ä£ºÍÖµ½»²æµãÖµ*/
     u16  XYZ_Acce_Max_Offset;                  /*XYZÈ¡Ä£ºÍÖµ½»²æµãÖµ×î´óÆ«ÀëÖµ*/
     
     
     u8   Pro_Step;                  /*²âËã²½Öè*/
     u8   DataGetCount;              /* Êý¾Ý»ñÈ¡¼ÆÊý*/
     u8   RisingTime;                /*È¡ÑùÉÏÉýÊ±¼äÀÛ»ý*/
     u8   FallingFlag;               /*È¡ÑùÏÂ½µ½»²æµã±êÖ¾*/ 
     u8   ChangeTime;                /*Á¬Ðø¼Æ²½ÏàÁÚ²½±äÖ®¼äµÄÊ±¼ä*/
     u8   LastChangeTime;            /*ÉÏÒ»¸öÁ¬Ðø¼Æ²½ÏàÁÚ²½±äÖ®¼äµÄÊ±¼ä£¬½â¾öÒ»Ð©Í»±äµÄÎÊÌâ*/
     u8   StepsChangeTimeBuffer;     /*Ç°¼¸ÃëµÄ¹ýÂË´¦Àí¼ÆÊ±Æ÷*/
     
     u8   StepsChangeCount;          /*²½ÊýÇ°¼¸²½±ä»¯¼ÆËã*/
     u8   StepsChangePreCount;       /*²½Êý¸Ä±äÇ°µÄÁÙÊ±²½ÊýÔÝ´æ*/
     
     u8   Read_3D_Error_Count;       /*Ôö¼ÓÒ»¸ö¶ÁÈ¡3Öá¼ÓËÙ¼Æ³ö´í¼ÆÊý*/
}STEP_COUNT_T;

STEP_COUNT_T Step_Count_data = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0},{0,0,0,0,0,0,0,0},0,0,0,0,0,0,0,0,0,0,0,0};  

/*¸öÈËÐÅÏ¢»ñÈ¡²¿·Ö*/
typedef struct
{
    u8   Gender;               /*ÐÔ±ð£¬0ÎªÅ®£¬1ÎªÄÐ*/
    u8   Height;               /*Éí¸ß£¬µ¥Î»cm*/
    u8   Weight;               /*ÌåÖØ£¬µ¥Î»Kg*/
    u8   StridedDistance;      /*²½·ù£¬µ¥Î»cm*/
    u32  One_Minu_BMR_Calorie; /*Ò»·ÖÖÓ»ù´¡¿¨Â·ÀïÖµ£¬µ¥Î»Ð¡¿¨*/
    u32  One_Minu_Heavy_Sport_Calorie;  /*Ò»·ÖÖÓÇ¿ÔË¶¯¿¨Â·ÀïÖµ£¬µ¥Î»Ð¡¿¨*/
    u32  One_Minu_Medium_Sport_Calorie;  /*Ò»·ÖÖÓÇ¿ÔË¶¯¿¨Â·ÀïÖµ£¬µ¥Î»Ð¡¿¨*/
    u32  One_Minu_Light_Sport_Calorie;  /*Ò»·ÖÖÓÇ¿ÔË¶¯¿¨Â·ÀïÖµ£¬µ¥Î»Ð¡¿¨*/
}BODY_INFO_T;

BODY_INFO_T Body_Info_data;

u8 Z_Acce_Val=0;  /*Ä£Äâµç×ÓÂÞÅÌÓÃ*/

void Acute_Sport_Time_Count_Init(void);
void Acute_Sport_Time_Count_Pro(void);
u16 CaculateAsbSquare(u8 Temp);
void InitVal(u16 *ValS,u16 Val, u8 Length);
u16 AverageVal(u16 *Val,u8 Num);
u16 AverageValPro(u16 *Val,u16 New,u8 Num);
u16 GetXYZ_Acce_Data(void);
void Step_Count_data_Init(void);
void step_count_proce(void);
s16 step_sample_init(void);

void Acute_Sport_Time_Count_Init(void)
{
    Acute_Sport_Time_Count_data.AcuteSportTimeProState=0;
    Acute_Sport_Time_Count_data.StepCountAcute=0;
    Acute_Sport_Time_Count_data.MinuteTimeCount=0;
    Acute_Sport_Time_Count_data.OverTimeCount=0;
}

void Acute_Sport_Time_Count_Pro(void)
{
    if(Acute_Sport_Time_Count_data.OverTimeCount)  Acute_Sport_Time_Count_data.OverTimeCount++;
  /*  if(Acute_Sport_Time_Count_data.MinuteTimeCount)  Acute_Sport_Time_Count_data.MinuteTimeCount++; ¸ü¸Äµ½Ê±¼ä±ä»¯ÄÇÀï£¬Ìá¸ß×¼È·¶È*/
    switch(Acute_Sport_Time_Count_data.AcuteSportTimeProState)
    {
        case 1:
             Acute_Sport_Time_Count_data.MinuteTimeCount=3;
             Acute_Sport_Time_Count_data.OverTimeCount=1;
             Acute_Sport_Time_Count_data.AcuteSportTimeProState=2;
        break;
        case 2:
             if(Acute_Sport_Time_Count_data.MinuteTimeCount>60)/*µ±Ê±¼ä´ïµ½1·ÖÖÓÊ±£¬Í³¼Æ´¦Àí*/
             {
                 if(Acute_Sport_Time_Count_data.StepCountAcute>ACUTE_MEASURE_MINI_STEP_VALUE)/*1·ÖÖÓ´óÓÚ100²½²ÅËãÊÇ¾çÁÒÔË¶¯*/
                 {
                     /*Acute_Sport_Time_Count_data.AcuteSportTimeCount++;*/
                     Total_Sport_Info_data.AcuteSportTimeCounts++;
                     One_Minute_Sport_Info_data.AcuteSportTimeCounts++;
                 }
                 Acute_Sport_Time_Count_data.StepCountAcute=0;
                 Acute_Sport_Time_Count_data.MinuteTimeCount=1;
             }
        break;
        default:
        break;
    }
    /*³¬Ê±´¦Àí£¬Á½²½Ö®¼ä²»ÄÜ³¬¹ý3Ãë*/
    if(Acute_Sport_Time_Count_data.OverTimeCount>ACUTE_MEASURE_OVER_TIME_VALUE)
    {      
       Acute_Sport_Time_Count_Init();
    }
}

/*¼ÆËã´ø·ûºÅµÄ8±ÈÌØÎ»ÊýµÄÆ½·½*/
u16 CaculateAsbSquare(u8 Temp)
{
    u16  ReturnVal=0,TempVal=0;
        if(Temp&0x80)
        {
            TempVal=0x100-Temp;
        }
        else
        {
            TempVal=Temp;
        }
        ReturnVal=TempVal*TempVal;
     return ReturnVal;
}

/*³õÊ¼»¯Êý¾Ý»º³åÇø*/
void InitVal(u16 *ValS,u16 Val, u8 Length)
{
    u8 i;
    for(i=0;i<Length;i++)
    {
        ValS[i]=Val;
    }
}

/*ÇóÄ³Êý×éµÄ¼¸¸öÊýÆ½¾ùÖµ*/
u16 AverageVal(u16 *Val,u8 Num)
{
    u32 ReturnVal;
    u8 i;
    ReturnVal=0;/**/
   if(Num>0)
   {
    for(i=0;i<Num;i++)
    {
        ReturnVal+=Val[i];
    }
    ReturnVal=ReturnVal/Num;
   }
    return ReturnVal;
}

/*ÇóÇ°4¸ö²ÉÑùÖµµÄÆ½¾ùÖµ£¬°ÑÐÂµÄÊý¾ÝÍÆÈë»º³åÇø*/
u16 AverageValPro(u16 *Val,u16 New,u8 Num)
{
    u32 ReturnVal,MaxVal,MinVal;
    u8 i;
    ReturnVal=0;/**/
    MaxVal=0; /*new caculate,È¥µô×î´óÖµ£¬È¥µô×îÐ¡Öµ£¬ºóÔÙÇóÆ½¾ù*/
    MinVal=0xFFFF; /*new caculate*/
  if(Num>2)
  {
    for(i=0;i<(Num-1);i++)
    {
        Val[i]=Val[i+1];
        ReturnVal+=Val[i];
        if(Val[i]>MaxVal) MaxVal=Val[i]; /*new caculate*/
        if(Val[i]<MinVal) MinVal=Val[i]; /*new caculate*/
    }
    Val[Num-1]=New;
    ReturnVal+=Val[Num-1];
    if(New>MaxVal) MaxVal=New; /*new caculate*/
    if(New<MinVal) MinVal=New; /*new caculate*/
    ReturnVal-=MaxVal;   /*new caculate*/
    ReturnVal-=MinVal;   /*new caculate*/
    i=Num-2; /*new caculate*/
    ReturnVal=ReturnVal/i;
  }
    return ReturnVal;
}

u16 GetXYZ_Acce_Data(void)
{
    u32  Return=0xFFFF;
    static u8 err_cnt = 0;
	gsensor_data_t data = {0,0,0,0,0,0};

    //if(get_driver()->gsensor->gsensor_read((void*)&data) == -1)
    {
        //return 0xFFFE;
    }
    while(get_driver()->gsensor->gsensor_read((void*)&data) == -1) {
        if(err_cnt++ >= 255) {
            return 0xFFFE;
        }
    }
    err_cnt = 0;
    
    if((data.x_h<0xFF)||(data.y_h<0xFF)||(data.z_h<0xFF))
    {
        Return = CaculateAsbSquare(data.x_h);
        Return += CaculateAsbSquare(data.y_h);
	    Return += CaculateAsbSquare(data.z_h);
            
	    Z_Acce_Val=data.z_h^0x80;	
    }
    
    return Return;
}

void Step_Count_data_Init(void)
{
   Step_Count_data.DataGetCount=0;/*»ñÈ¡0¸öÖµ*/
   Step_Count_data.FallingFlag=0;
   Step_Count_data.RisingTime=0;
   Step_Count_data.StepsChangePreCount=0;
   Step_Count_data.ChangeTime=0;
   Step_Count_data.StepsChangeCount=0;        
   Step_Count_data.Pro_Step=PRO_STEP_IDLE;   
   /*°´¼ü´¦Àí²¿·Ö*/
   Step_Count_data.Read_3D_Error_Count=0;/*Ôö¼ÓÒ»¸ö¶Á3Öá¼ÓËÙ¼Æ³ö´í¼ÆÊý*/
   Step_Count_data.LastChangeTime=0;   /*½â¾öÒ»Ð©²½ÊýÍ»±äµÄÎÊÌâ*/
   Step_Count_data.StepsChangeTimeBuffer=0; /*½â¾öÓÐÊ±Ç°¼¸²½±ä»¯µÄÎÊÌâ*/
}

static void StepCountProce(void);
static void StepCountProce(void)
{
    uint8 i=0,StepFlag=0;
    uint16 Temp=0xFFFF;    
    //if(tid==LIS3DH_Sampling_Timer)
    {  
        //TimerDelete(LIS3DH_Sampling_Timer);/**/
        //LIS3DH_Sampling_Timer=TIMER_INVALID;
        //Date_Time_Process();/*Ê±¼ä×ß¶¯´¦Àí*/
        if(Step_Count_data.Pro_Step==PRO_STEP_START)
        {
            Temp=GetXYZ_Acce_Data();
            if(Temp<0xFFFE)/*È¡µÃÊý¾Ý*/
            {
                Step_Count_data_Init();
                Step_Count_data.Pro_Step=PRO_STEP_RISING;/**/
                InitVal(Step_Count_data.FiFo_DataString_Val,Temp,ALL_DATA_NUM);/*´æ´¢Ã¿×é4¸öÆ½¾ùÖµµÄÊý¾Ý£¬Ä¿Ç°´æ16×é*/
                InitVal(Step_Count_data.FiFo_Filter_Val,Temp,FILTER_NUM);      /*Êý¾Ý×öÆ½»¬´¦Àí*/
            }
        }
        else if(Step_Count_data.Pro_Step==PRO_STEP_RISING)
        {
            for(i=0;i<32;i++) /*Ô­íÊ?0*/
            {
                Temp=GetXYZ_Acce_Data();
                if(Temp<0xFFFE){ 
                    Temp=AverageValPro(Step_Count_data.FiFo_Filter_Val,Temp,FILTER_NUM);/*Êý¾Ý×öÆ½»¬´¦Àí*/
                    if(Step_Count_data.DataGetCount<ONE_DATA_NUM) 
                    {
                        Step_Count_data.FiFo_OneData_Val[Step_Count_data.DataGetCount++]=Temp;
                    }
                    if(Step_Count_data.DataGetCount>=ONE_DATA_NUM)/*Ò»Ð¡×éÊý¾Ý»ñÈ¡Íê³É£¬½øÈëÊý¾Ý·ÖÎö*/
                    {
                        Step_Count_data.DataGetCount=0;
                        if((Step_Count_data.RisingTime)&&(Step_Count_data.RisingTime<255))  Step_Count_data.RisingTime++;  
                        Temp=AverageVal(Step_Count_data.FiFo_OneData_Val,ONE_DATA_NUM);/*Ëã³öÐ¡×éÊý¾ÝµÄÆ½¾ùÖµ*/
                        Temp=AverageValPro(Step_Count_data.FiFo_DataString_Val,Temp,ALL_DATA_NUM);/*´æÈëÊý×é»º³åÇø£¬Ëã³öÆ½¾ùÖµ*/ 
                        if(Step_Count_data.FiFo_DataString_Val[DATA_DELAY_NUM]>=Temp)/*Æ½¾ùÖµÊý¾ÝÌáÇ°3¸ö£¬ÉèÎª7-3£½4,µ±¼´Ê±Êý¾Ý³¬¹ýÆ½¾ùÖµÊ±*/
                        {
                            if(Step_Count_data.RisingTime==0)
                            {
                                Step_Count_data.RisingTime++;
                                Step_Count_data.XYZ_Across_Acce_Min=Step_Count_data.FiFo_DataString_Val[DATA_DELAY_NUM-1];  
                                Step_Count_data.XYZ_Acce_Max_Offset=0;
                            }
                            if(Step_Count_data.FiFo_DataString_Val[DATA_DELAY_NUM]>Step_Count_data.XYZ_Across_Acce_Min+Step_Count_data.XYZ_Acce_Max_Offset)
                            {
                                Step_Count_data.XYZ_Acce_Max_Offset=Step_Count_data.FiFo_DataString_Val[DATA_DELAY_NUM]-Step_Count_data.XYZ_Across_Acce_Min;
                            }
                            Step_Count_data.FallingFlag=0;
                        }
                        else
                        {
                            if(Step_Count_data.FallingFlag==0)
                            {
                                Step_Count_data.FallingFlag++;
                                if(Step_Count_data.XYZ_Acce_Max_Offset>=ASLEEP_MOVE_OFFSET_MIN_VAL)
                                {
                                    if(Step_Count_data.RisingTime<=ASLEEP_ONE_STEP_PERIOD_MAX_TIME)
                                    {
                                        Asleep_Data_Info.FifteenMinuteMove++;
                                    }
                                }
                                if((Step_Count_data.XYZ_Acce_Max_Offset>=STEP_OFFSET_MIN_VAL)||(Step_Count_data.XYZ_Acce_Max_Offset>=START_STEP_OFFSET_MIN_VAL))
                                {
                                    if(Temp>=QUICKLY_STEP_MIN_VAL)/*¿ìËÙÔË¶¯*/
                                    {
                                        if((Step_Count_data.RisingTime<=QUICKLY_STEP_TIME_MAX)&&(Step_Count_data.XYZ_Acce_Max_Offset>=QUICKLY_STEP_OFFSET_MIN_VAL))/*³¬¹ý·§Öµ£¬*/
                                        {
                                            /*·ûºÏÌõ¼þ*/
                                            StepFlag=1;
                                        }
                                    }
                                    else 
                                    {
                                        if((Step_Count_data.RisingTime>=SLOW_ONE_STEP_PERIOD_MIN_TIME)&&(Step_Count_data.RisingTime<=ONE_STEP_PERIOD_MAX_TIME)&&((Step_Count_data.ChangeTime==0)||(Step_Count_data.ChangeTime>=SLOW_STEP_TIME_MIN)))
                                        {
                                            if(((Step_Count_data.StepsChangeCount<HOW_MANY_STEP_BUFFER)&&(Step_Count_data.XYZ_Acce_Max_Offset>=START_STEP_OFFSET_MIN_VAL))||(Step_Count_data.StepsChangeCount>=HOW_MANY_STEP_BUFFER))
                                                StepFlag=2;
                                        }
                                    }
                                } 
                             }
                             Step_Count_data.RisingTime=0;
                        }
                        if((StepFlag==2)&&(Step_Count_data.StepsChangeCount>=2))
                       /* if((StepFlag>0)&&(Step_Count_data.StepsChangeCount>=2))*/
                        {
                            if((Step_Count_data.ChangeTime<(Step_Count_data.LastChangeTime/2))||(Step_Count_data.ChangeTime>(Step_Count_data.LastChangeTime*3/2)))
                            {
                                StepFlag=0;
                            }
                            else 
                            {
                                Step_Count_data.LastChangeTime+=Step_Count_data.ChangeTime;
                                Step_Count_data.LastChangeTime=Step_Count_data.LastChangeTime/2;
                            }      
                        }
                        if(StepFlag>0)
                        {
                            /*if(Vibrator_data.VibrationProState==0)*/ 
                            //if(Vibrator_data.VibrationCounter==0)  /*jim modify 20150817*/
                            {
                                Step_Count_data.StepsChangeCount++;
                            }
                           /* if(Step_Count_data.ChangeTime==0) 
                            {
                                Step_Count_data.ChangeTime=1; 
                            }×¢Òâ´Ë´¦µÄ×÷ÓÃ*/
                            if(Step_Count_data.StepsChangeCount==0x01)
                            {
                              Step_Count_data.ChangeTime=1; 
                              Step_Count_data.StepsChangeTimeBuffer=1; 
                            }
                            if(Step_Count_data.StepsChangeCount==2)/*µÚ¶þ´ÎÓÐÐ§²½ÊýµÄÊ±ºò£¬¼ÇÂ¼ÏÂÁ½²½µÄÊ±¼ä²î*//*½â¾ö²½ÊýÍ»±äµÄÎÊÌâ*/
                            {
                                Step_Count_data.LastChangeTime=Step_Count_data.ChangeTime;
                            }
                            if(Step_Count_data.LastChangeTime<6)
                            {
                                Step_Count_data.LastChangeTime=6;
                            }
                            StepFlag=0;
                        }
                        /*´Ë¶Î¹ýÂËÇ°¼¸²½±ä»¯£¬Ö»ÓÐÁ¬½Ó³öÏÖ6²½±ä»¯ºóÔÙ½øÈëÕæÕý¼Æ²½*/  
                        if((Step_Count_data.ChangeTime>0)&&(Step_Count_data.ChangeTime<255))
                        {
                            Step_Count_data.ChangeTime++;/*40msÎªÒ»µ¥Î»*/
                        }
                        /*ÒÔÏÂÊÇÒÔÊ±¼äÎªbuffer²¿·Ö*/
                        if((Step_Count_data.StepsChangeTimeBuffer>0)&&(Step_Count_data.StepsChangeTimeBuffer<255))
                        {
                            Step_Count_data.StepsChangeTimeBuffer++;/*40msÎªÒ»µ¥Î»*/
                        }
                        if((Step_Count_data.StepsChangeTimeBuffer>MIDDLE_STEP_SECOND)&&(Step_Count_data.StepsChangeCount<MIDDLE_STEP_COUNT))
                        {
                            Step_Count_data.StepsChangeTimeBuffer=1;
                            Step_Count_data.StepsChangeCount=1;
                        }
                        if((Step_Count_data.ChangeTime>=TWO_STEP_BETWEEN_MAX_TIME)||((Step_Count_data.ChangeTime<=TWO_STEP_BETWEEN_MIN_TIME)&&(Step_Count_data.StepsChangeCount>1)&&(Step_Count_data.StepsChangeCount!=Step_Count_data.StepsChangePreCount)))/*1.6ÃëÄÚÎÞÁ¬Ðø¶¯×÷£¬ÈÏÎªÊÇ¼Ù×ß,Ð¡ÓÚ0.2Ãë£¬Ò²ÈÏÎªÊÇ¼Ù×ß*/
                        {
                            Step_Count_data.StepsChangeCount=0;
                            Step_Count_data.ChangeTime=0; 
                            Step_Count_data.StepsChangePreCount=Step_Count_data.StepsChangeCount;
                            Step_Count_data.LastChangeTime=0;/*½â¾ö²½ÊýÍ»±äµÄÎÊÌâ*/
                            Step_Count_data.StepsChangeTimeBuffer=0; /*½â¾öÓÐÊ±Ç°¼¸²½±ä»¯µÄÎÊÌâ*/
                        }
                        else if(Step_Count_data.StepsChangeCount!=Step_Count_data.StepsChangePreCount)
                        {
                            if(Step_Count_data.StepsChangeTimeBuffer<HOW_MANY_SECOND_BUFFER)/*Ð¡ÓÚ3ÃëµÄ´¦Àí*/
                            {
                                if(Step_Count_data.StepsChangeCount==MIDDLE_STEP_COUNT)
                                {
                                    if(Step_Count_data.StepsChangeTimeBuffer>MIDDLE_STEP_SECOND)
                                    {
                                        Step_Count_data.StepsChangeTimeBuffer=1;
                                        Step_Count_data.StepsChangeCount=1;
                                    }
                                }
                            }
                            else if(Step_Count_data.StepsChangeTimeBuffer<0xFF)/*µÈÓÚ3ÃëµÄ´¦Àí*/
                            {
                                if(Step_Count_data.StepsChangeCount>=HOW_MANY_STEP_BUFFER)/*Ç°3ÃëÄÚÐèÒªÖÁÉÙ6²½*/
                                {
                                    Total_Sport_Info_data.StepCounts+=Step_Count_data.StepsChangeCount;
                                    Total_Sport_Info_data.Distance+=Body_Info_data.StridedDistance*Step_Count_data.StepsChangeCount;
                                    One_Minute_Sport_Info_data.StepCounts+=Step_Count_data.StepsChangeCount;
                                    One_Minute_Sport_Info_data.Distance+=Body_Info_data.StridedDistance*Step_Count_data.StepsChangeCount;

                                    //SB100_data.UpdateDataFlag=0x02;/*Êý¾ÝÓÐ¸üÐÂ*//*jim*/
                                    Step_Count_data.StepsChangeTimeBuffer=0xFF;
                                    /*¾çÁÒÔË¶¯²âËã*/
                                    if(Acute_Sport_Time_Count_data.AcuteSportTimeProState==0)
                                    {
                                        Acute_Sport_Time_Count_data.StepCountAcute=Step_Count_data.StepsChangeCount;
                                        Acute_Sport_Time_Count_data.AcuteSportTimeProState=1;
                                    }
                                    else
                                    {
                                        Acute_Sport_Time_Count_data.OverTimeCount=1;
                                        Acute_Sport_Time_Count_data.StepCountAcute+=Step_Count_data.StepsChangeCount;
                                    }
                                }
                                else /*ÈÏÎªÊÇÎÞ×÷ÓÃµÄ¶¯×÷*/
                                {
                                    Step_Count_data.StepsChangeCount=0;
                                    Step_Count_data.ChangeTime=0; 
                                    Step_Count_data.StepsChangePreCount=Step_Count_data.StepsChangeCount;
                                    Step_Count_data.LastChangeTime=0;/*½â¾ö²½ÊýÍ»±äµÄÎÊÌâ*/
                                    Step_Count_data.StepsChangeTimeBuffer=0; /*½â¾öÓÐÊ±Ç°¼¸²½±ä»¯µÄÎÊÌâ*/  
                                }
                            }
                            else 
                            {
                                    Total_Sport_Info_data.StepCounts++;
                                    Total_Sport_Info_data.Distance+=Body_Info_data.StridedDistance;
                                    One_Minute_Sport_Info_data.StepCounts++;
                                    One_Minute_Sport_Info_data.Distance+=Body_Info_data.StridedDistance;

                                    if(Acute_Sport_Time_Count_data.AcuteSportTimeProState==0)
                                    {
                                        Acute_Sport_Time_Count_data.StepCountAcute=1;
                                        Acute_Sport_Time_Count_data.AcuteSportTimeProState=1;                                        
                                    }
                                    else /*(Acute_Sport_Time_Count_data.AcuteSportTimeProState) °Ñ¾çÁÒÔË¶¯µÄ²âÁ¿ÑÏ¸ñ»¯*/
                                    {
                                        Acute_Sport_Time_Count_data.OverTimeCount=1;
                                        /*if(Acute_Sport_Time_Count_data.AcuteSportTimeProState==2)*/
                                        {
                                            Acute_Sport_Time_Count_data.StepCountAcute++; 
                                        }    
                                    }
                                    Step_Count_data.StepsChangeCount=HOW_MANY_STEP_BUFFER+1;
                                    //SB100_data.UpdateDataFlag=0x02;/*Êý¾ÝÓÐ¸üÐÂ*//*jim*/ 
                            }
                            Step_Count_data.ChangeTime=1;
                            Step_Count_data.StepsChangePreCount=Step_Count_data.StepsChangeCount;
                            /*¸ß¶È²½ÊýÀÛ¼Æ*/
						#ifdef LPS25H_ENABLE_
                            if((Step_Count_data.StepsChangeCount>=MIDDLE_STEP_COUNT)&&(Hight_Count_data.StepCountHight==0)&&(Hight_Count_data.Hight_Pro_State!=HIGHT_MEASURE_PRO))
                            {
                                    Hight_Count_data.Hight_Pro_State=HIGHT_MEASURE_START;
                                    Hight_Count_data.StartStepCountHight=Total_Sport_Info_data.StepCounts;
                            }
                            if(Hight_Count_data.StepCountHight)
                            {
                                Hight_Count_data.OverTimeCount=1;/**/
                            }
						 #endif
                        }
                        /************************/
                        /*¸ß¶ÈÔËËã´¦Àí */
                       #ifdef LPS25H_ENABLE_
                        Hight_Measure_Pro(); /*ÐèÒªÐÞ¸Ä*/
                       #endif
                        /*¾çÁÒÔË¶¯²âËã*/
                        Acute_Sport_Time_Count_Pro();/*ÐèÒªÐÞ¸Ä*/
                        /*ÒÔÄÜ¶ÁÈ¡µ½Êý¾Ý×öÎªÊ±¼ä¼ä¸ô*/
                    }
                    Step_Count_data.Read_3D_Error_Count=0;        
                }
                else 
                {
                    Step_Count_data.Read_3D_Error_Count++;         
                    break;
                }
            }
        }
    }
}

static void step_sample_handler(u16 id)
{
    StepCountProce();

    #if 0
    u8 buf[16] = {0};
    u8 val[4] = {0};
    static u8 cnt = 0;
    static u32 step_count = 0;
    if(step_count != Total_Sport_Info_data.StepCounts)
    {
        val[0] = (Total_Sport_Info_data.StepCounts>>24) & 0x000000FF;
        val[1] = (Total_Sport_Info_data.StepCounts>>16) & 0x000000FF;
        val[2] = (Total_Sport_Info_data.StepCounts>>8) & 0x000000FF;
        val[3] = Total_Sport_Info_data.StepCounts & 0x000000FF;
        sprintf((char*)buf, "%03d: %d%d%d%d\r\n", cnt++, val[0], val[1], val[2], val[3]);
        send_ble(val, 4);
        printf("%s", buf);
    }
    step_count = Total_Sport_Info_data.StepCounts;
    #endif
    
	get_driver()->timer->timer_start(280, step_sample_handler);
}

s16 step_sample_init(void)
{
	get_driver()->timer->timer_start(280, step_sample_handler);  
    Step_Count_data.Pro_Step=PRO_STEP_START;  
	return 0;
}

u32 step_get(void)
{
    return Total_Sport_Info_data.StepCounts;
}
