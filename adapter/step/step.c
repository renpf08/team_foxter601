#include <debug.h>          /* Simple host interface to the UART driver */
#include <mem.h>
#include "../adapter.h"

#define PRO_STEP_IDLE       0
#define PRO_STEP_START      1
#define PRO_STEP_RISING     2
#define PRO_STEP_STOP       3     /*jim add 20150819关掉计步处理*/

#define SAMPLE_TIME_UNINT   40         /*采样时间单位，40ms*/

#define FILTER_NUM          8          /*滤波数据数，8个数据平均，去掉突波*/
#define ONE_DATA_NUM        4          /*4个数求平均值*/
#define ALL_DATA_NUM        16         /*16组数求平均值*/
#define DATA_DELAY_NUM      8          /*15-7,16组数据提前7个数据值*/

#define ASLEEP_MOVE_OFFSET_MIN_VAL      1000       /*睡眠部分的运动阀值*/
#define ASLEEP_ONE_STEP_PERIOD_MAX_TIME (1000/SAMPLE_TIME_UNINT)    /*睡眠部分运动波峰最长时间1秒，1000ms/40ms=25 */

#define QUICKLY_STEP_MIN_VAL            16000/* 8000 */   /*睡眠运动最小加速阀值，大于此值认为是在快走*/
#define QUICKLY_STEP_OFFSET_MIN_VAL     2000 /*2400*/    /*快步跑的偏移值阀值2500*/
#define STEP_OFFSET_MIN_VAL             1100 /* 1600*/ /*1200 */       /*慢走时共模加速度值最小偏差值1300*/
#define START_STEP_OFFSET_MIN_VAL       950 /* 1600*/ /*1200 */       /*慢走时共模加速度值最小偏差值1300*/

#define QUICKLY_STEP_TIME_MAX           (250/SAMPLE_TIME_UNINT)/*(200/SAMPLE_TIME_UNINT)(300/SAMPLE_TIME_UNINT) */   /*快走最大波峰时间是0.2秒，200ms/40ms=5*/
#define SLOW_STEP_TIME_MIN              (200/SAMPLE_TIME_UNINT)/* //(250/SAMPLE_TIME_UNINT)*/ /*(400/SAMPLE_TIME_UNINT)*/    /*慢走两步之间的最小时间间隔是0.48秒，480ms/40ms=12*/

#define SLOW_ONE_STEP_PERIOD_MIN_TIME   (120/SAMPLE_TIME_UNINT)/*//(160/SAMPLE_TIME_UNINT)*/  /*一步慢走时最短的波峰时间是160ms*/
#define ONE_STEP_PERIOD_MAX_TIME        (1000/SAMPLE_TIME_UNINT) /*(1000/SAMPLE_TIME_UNINT)*/  /*一步最长的波峰时间是1秒，1000ms/40ms=25*/
    
#define TWO_STEP_BETWEEN_MIN_TIME       (120/SAMPLE_TIME_UNINT)    /*两步之间最小时间差0.12秒 120ms/40ms=3 */
#define TWO_STEP_BETWEEN_MAX_TIME       (1200/SAMPLE_TIME_UNINT)/*//(1200/SAMPLE_TIME_UNINT)*/ /*(1000/SAMPLE_TIME_UNINT)*/ /*(1200/SAMPLE_TIME_UNINT)*/  /*两步之间最大时间差1.0秒  1600*40ms=40 */

#define HOW_MANY_SECOND_BUFFER          (4800/SAMPLE_TIME_UNINT) /*  (4000/SAMPLE_TIME_UNINT)*/   /*4秒内完成5步的处理*/
#define HOW_MANY_STEP_BUFFER            7  /* 6 */      /*动前几步过滤，至少要大于3*/

#define MIDDLE_STEP_COUNT               4        /*减少误差，前1.8秒内要有至少3步*/
#define MIDDLE_STEP_SECOND              (2800/SAMPLE_TIME_UNINT)   /*1.8秒的时候要至少3步的值2.8*/

/***************************************************************************************/
/************************************运动数据初始化****************************************/
/***************************************************************************************/
typedef struct
{
    u32   StepCounts;           /*总步数 步*/
    u32   Distance;             /*总距离 米*/
    u32   Calorie;              /*总卡路里 千卡*/
    u16   FloorCounts;          /*爬楼数 层*/
    u16   AcuteSportTimeCounts; /*剧烈运动时间 分钟*/
}SPORT_INFO_T;  /*运动数据结构*/

SPORT_INFO_T Total_Sport_Info_data;
SPORT_INFO_T One_Minute_Sport_Info_data;      /*一分钟数据统计结束*/
SPORT_INFO_T Fifteen_Minutes_Sport_Info_data; /*十五分钟数据统计*/

/***************************************************************************************/
/**********************************十五分钟睡眠信息处理*************************************/
/***************************************************************************************/
typedef struct
{
     u16 Year;
     u8  Month;
     u8  Date;
     u8  Hour;
     u8  Minute;  
     u32 FifteenMinuteMove;  /*十五分钟的运动量*/
	 u16 AsleepInfo_Data_Table[12]; /*一天睡眠信息存储表*/
}ASLEEP_DATA_INFO_T;  /*睡眠时间点信息数据结构*/
ASLEEP_DATA_INFO_T Asleep_Data_Info;

/***************************************************************************************/
/************************************剧烈运动运算部分**************************************/
/***************************************************************************************/
#define ACUTE_MEASURE_OVER_TIME_VALUE     (5000/SAMPLE_TIME_UNINT)/* 300 */     /*剧烈运动，两步之间无变化的时间，6秒----此值根据采样时间调整*/ 

#define ACUTE_MEASURE_MINI_STEP_VALUE      100      /*1分钟至少要运动120步才算剧烈运动*/
typedef struct
{    
    u8   AcuteSportTimeProState;               /*测算步骤状态*/
    u16   OverTimeCount;                        /*测算超时计数器*/
    u8   MinuteTimeCount;                      /*分钟计数器*/
    u16  StepCountAcute;                       /*开始测剧烈运动步数累计*/
}ACUTE_SPORT_TIME_COUNT_T;
/*剧烈运动测算部分*/
ACUTE_SPORT_TIME_COUNT_T  Acute_Sport_Time_Count_data;

typedef struct
{
     u16  FiFo_DataString_Val[ALL_DATA_NUM];           /*三轴加速度连续16组共模值*/
     u16  FiFo_OneData_Val[ONE_DATA_NUM];              /*三轴加速度连续4个共模值*/
     u16  FiFo_Filter_Val[FILTER_NUM];                 /*三轴加速度连续4个共模值*/
     u16  XYZ_Across_Acce_Min;                  /*XYZ取模和值交叉点值*/
     u16  XYZ_Acce_Max_Offset;                  /*XYZ取模和值交叉点值最大偏离值*/
     
     
     u8   Pro_Step;                  /*测算步骤*/
     u8   DataGetCount;              /* 数据获取计数*/
     u8   RisingTime;                /*取样上升时间累积*/
     u8   FallingFlag;               /*取样下降交叉点标志*/ 
     u8   ChangeTime;                /*连续计步相邻步变之间的时间*/
     u8   LastChangeTime;            /*上一个连续计步相邻步变之间的时间，解决一些突变的问题*/
     u8   StepsChangeTimeBuffer;     /*前几秒的过滤处理计时器*/
     
     u8   StepsChangeCount;          /*步数前几步变化计算*/
     u8   StepsChangePreCount;       /*步数改变前的临时步数暂存*/
     
     u8   Read_3D_Error_Count;       /*增加一个读取3轴加速计出错计数*/
}STEP_COUNT_T;
STEP_COUNT_T Step_Count_data = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0},{0,0,0,0,0,0,0,0},0,0,0,0,0,0,0,0,0,0,0,0};  

/*个人信息获取部分*/
typedef struct
{
    u8   Gender;               /*性别，0为女，1为男*/
    u8   Height;               /*身高，单位cm*/
    u8   Weight;               /*体重，单位Kg*/
    u8   StridedDistance;      /*步幅，单位cm*/
    u32  One_Minu_BMR_Calorie; /*一分钟基础卡路里值，单位小卡*/
    u32  One_Minu_Heavy_Sport_Calorie;  /*一分钟强运动卡路里值，单位小卡*/
    u32  One_Minu_Medium_Sport_Calorie;  /*一分钟强运动卡路里值，单位小卡*/
    u32  One_Minu_Light_Sport_Calorie;  /*一分钟强运动卡路里值，单位小卡*/
}BODY_INFO_T;
BODY_INFO_T Body_Info_data;

u8 Z_Acce_Val=0;  /*模拟电子罗盘用*/

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
  /*  if(Acute_Sport_Time_Count_data.MinuteTimeCount)  Acute_Sport_Time_Count_data.MinuteTimeCount++; 更改到时间变化那里，提高准确度*/
    switch(Acute_Sport_Time_Count_data.AcuteSportTimeProState)
    {
        case 1:
             Acute_Sport_Time_Count_data.MinuteTimeCount=3;
             Acute_Sport_Time_Count_data.OverTimeCount=1;
             Acute_Sport_Time_Count_data.AcuteSportTimeProState=2;
        break;
        case 2:
             if(Acute_Sport_Time_Count_data.MinuteTimeCount>60)/*当时间达到1分钟时，统计处理*/
             {
                 if(Acute_Sport_Time_Count_data.StepCountAcute>ACUTE_MEASURE_MINI_STEP_VALUE)/*1分钟大于100步才算是剧烈运动*/
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
    /*超时处理，两步之间不能超过3秒*/
    if(Acute_Sport_Time_Count_data.OverTimeCount>ACUTE_MEASURE_OVER_TIME_VALUE)
    {      
       Acute_Sport_Time_Count_Init();
    }
}
/*计算带符号的8比特位数的平方*/
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
/*初始化数据缓冲区*/
void InitVal(u16 *ValS,u16 Val, u8 Length)
{
    u8 i;
    for(i=0;i<Length;i++)
    {
        ValS[i]=Val;
    }
}
/*求某数组的几个数平均值*/
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
/*求前4个采样值的平均值，把新的数据推入缓冲区*/
u16 AverageValPro(u16 *Val,u16 New,u8 Num)
{
    u32 ReturnVal,MaxVal,MinVal;
    u8 i;
    ReturnVal=0;/**/
    MaxVal=0; /*new caculate,去掉最大值，去掉最小值，后再求平均*/
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
   Step_Count_data.DataGetCount=0;/*获取0个值*/
   Step_Count_data.FallingFlag=0;
   Step_Count_data.RisingTime=0;
   Step_Count_data.StepsChangePreCount=0;
   Step_Count_data.ChangeTime=0;
   Step_Count_data.StepsChangeCount=0;        
   Step_Count_data.Pro_Step=PRO_STEP_IDLE;   
   /*按键处理部分*/
   Step_Count_data.Read_3D_Error_Count=0;/*增加一个读3轴加速计出错计数*/
   Step_Count_data.LastChangeTime=0;   /*解决一些步数突变的问题*/
   Step_Count_data.StepsChangeTimeBuffer=0; /*解决有时前几步变化的问题*/
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
        //Date_Time_Process();/*时间走动处理*/
        if(Step_Count_data.Pro_Step==PRO_STEP_START)
        {
            Temp=GetXYZ_Acce_Data();
            if(Temp<0xFFFE)/*取得数据*/
            {
                Step_Count_data_Init();
                Step_Count_data.Pro_Step=PRO_STEP_RISING;/**/
                InitVal(Step_Count_data.FiFo_DataString_Val,Temp,ALL_DATA_NUM);/*存储每组4个平均值的数据，目前存16组*/
                InitVal(Step_Count_data.FiFo_Filter_Val,Temp,FILTER_NUM);      /*数据做平滑处理*/
            }
        }
        else if(Step_Count_data.Pro_Step==PRO_STEP_RISING)
        {
            for(i=0;i<32;i++) /*原來是40*/
            {
                Temp=GetXYZ_Acce_Data();
                if(Temp<0xFFFE){ 
                    Temp=AverageValPro(Step_Count_data.FiFo_Filter_Val,Temp,FILTER_NUM);/*数据做平滑处理*/
                    if(Step_Count_data.DataGetCount<ONE_DATA_NUM) 
                    {
                        Step_Count_data.FiFo_OneData_Val[Step_Count_data.DataGetCount++]=Temp;
                    }
                    if(Step_Count_data.DataGetCount>=ONE_DATA_NUM)/*一小组数据获取完成，进入数据分析*/
                    {
                        Step_Count_data.DataGetCount=0;
                        if((Step_Count_data.RisingTime)&&(Step_Count_data.RisingTime<255))  Step_Count_data.RisingTime++;  
                        Temp=AverageVal(Step_Count_data.FiFo_OneData_Val,ONE_DATA_NUM);/*算出小组数据的平均值*/
                        Temp=AverageValPro(Step_Count_data.FiFo_DataString_Val,Temp,ALL_DATA_NUM);/*存入数组缓冲区，算出平均值*/ 
                        if(Step_Count_data.FiFo_DataString_Val[DATA_DELAY_NUM]>=Temp)/*平均值数据提前3个，设为7-3＝4,当即时数据超过平均值时*/
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
                                    if(Temp>=QUICKLY_STEP_MIN_VAL)/*快速运动*/
                                    {
                                        if((Step_Count_data.RisingTime<=QUICKLY_STEP_TIME_MAX)&&(Step_Count_data.XYZ_Acce_Max_Offset>=QUICKLY_STEP_OFFSET_MIN_VAL))/*超过阀值，*/
                                        {
                                            /*符合条件*/
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
                            }注意此处的作用*/
                            if(Step_Count_data.StepsChangeCount==0x01)
                            {
                              Step_Count_data.ChangeTime=1; 
                              Step_Count_data.StepsChangeTimeBuffer=1; 
                            }
                            if(Step_Count_data.StepsChangeCount==2)/*第二次有效步数的时候，记录下两步的时间差*//*解决步数突变的问题*/
                            {
                                Step_Count_data.LastChangeTime=Step_Count_data.ChangeTime;
                            }
                            if(Step_Count_data.LastChangeTime<6)
                            {
                                Step_Count_data.LastChangeTime=6;
                            }
                            StepFlag=0;
                        }
                        /*此段过滤前几步变化，只有连接出现6步变化后再进入真正计步*/  
                        if((Step_Count_data.ChangeTime>0)&&(Step_Count_data.ChangeTime<255))
                        {
                            Step_Count_data.ChangeTime++;/*40ms为一单位*/
                        }
                        /*以下是以时间为buffer部分*/
                        if((Step_Count_data.StepsChangeTimeBuffer>0)&&(Step_Count_data.StepsChangeTimeBuffer<255))
                        {
                            Step_Count_data.StepsChangeTimeBuffer++;/*40ms为一单位*/
                        }
                        if((Step_Count_data.StepsChangeTimeBuffer>MIDDLE_STEP_SECOND)&&(Step_Count_data.StepsChangeCount<MIDDLE_STEP_COUNT))
                        {
                            Step_Count_data.StepsChangeTimeBuffer=1;
                            Step_Count_data.StepsChangeCount=1;
                        }
                        if((Step_Count_data.ChangeTime>=TWO_STEP_BETWEEN_MAX_TIME)||((Step_Count_data.ChangeTime<=TWO_STEP_BETWEEN_MIN_TIME)&&(Step_Count_data.StepsChangeCount>1)&&(Step_Count_data.StepsChangeCount!=Step_Count_data.StepsChangePreCount)))/*1.6秒内无连续动作，认为是假走,小于0.2秒，也认为是假走*/
                        {
                            Step_Count_data.StepsChangeCount=0;
                            Step_Count_data.ChangeTime=0; 
                            Step_Count_data.StepsChangePreCount=Step_Count_data.StepsChangeCount;
                            Step_Count_data.LastChangeTime=0;/*解决步数突变的问题*/
                            Step_Count_data.StepsChangeTimeBuffer=0; /*解决有时前几步变化的问题*/
                        }
                        else if(Step_Count_data.StepsChangeCount!=Step_Count_data.StepsChangePreCount)
                        {
                            if(Step_Count_data.StepsChangeTimeBuffer<HOW_MANY_SECOND_BUFFER)/*小于3秒的处理*/
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
                            else if(Step_Count_data.StepsChangeTimeBuffer<0xFF)/*等于3秒的处理*/
                            {
                                if(Step_Count_data.StepsChangeCount>=HOW_MANY_STEP_BUFFER)/*前3秒内需要至少6步*/
                                {
                                    Total_Sport_Info_data.StepCounts+=Step_Count_data.StepsChangeCount;
                                    Total_Sport_Info_data.Distance+=Body_Info_data.StridedDistance*Step_Count_data.StepsChangeCount;
                                    One_Minute_Sport_Info_data.StepCounts+=Step_Count_data.StepsChangeCount;
                                    One_Minute_Sport_Info_data.Distance+=Body_Info_data.StridedDistance*Step_Count_data.StepsChangeCount;

                                    //SB100_data.UpdateDataFlag=0x02;/*数据有更新*//*jim*/
                                    Step_Count_data.StepsChangeTimeBuffer=0xFF;
                                    /*剧烈运动测算*/
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
                                else /*认为是无作用的动作*/
                                {
                                    Step_Count_data.StepsChangeCount=0;
                                    Step_Count_data.ChangeTime=0; 
                                    Step_Count_data.StepsChangePreCount=Step_Count_data.StepsChangeCount;
                                    Step_Count_data.LastChangeTime=0;/*解决步数突变的问题*/
                                    Step_Count_data.StepsChangeTimeBuffer=0; /*解决有时前几步变化的问题*/  
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
                                    else /*(Acute_Sport_Time_Count_data.AcuteSportTimeProState) 把剧烈运动的测量严格化*/
                                    {
                                        Acute_Sport_Time_Count_data.OverTimeCount=1;
                                        /*if(Acute_Sport_Time_Count_data.AcuteSportTimeProState==2)*/
                                        {
                                            Acute_Sport_Time_Count_data.StepCountAcute++; 
                                        }    
                                    }
                                    Step_Count_data.StepsChangeCount=HOW_MANY_STEP_BUFFER+1;
                                    //SB100_data.UpdateDataFlag=0x02;/*数据有更新*//*jim*/ 
                            }
                            Step_Count_data.ChangeTime=1;
                            Step_Count_data.StepsChangePreCount=Step_Count_data.StepsChangeCount;
                            /*高度步数累计*/
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
                        /*高度运算处理 */
                       #ifdef LPS25H_ENABLE_
                        Hight_Measure_Pro(); /*需要修改*/
                       #endif
                        /*剧烈运动测算*/
                        Acute_Sport_Time_Count_Pro();/*需要修改*/
                        /*以能读取到数据做为时间间隔*/
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

    #if 1
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

