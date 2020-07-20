#include <debug.h>          /* Simple host interface to the UART driver */
#include <mem.h>
#include "../adapter.h"

#define PRO_STEP_IDLE       0
#define PRO_STEP_START      1
#define PRO_STEP_RISING     2
//#define PRO_STEP_STOP       3     /*jim add 20150819关掉计步处理*/

#define SAMPLE_TIME_UNINT   40         /*采样时间单位，40ms*/

#define FILTER_NUM          8          /*滤波数据数，8个数据平均，去掉突波*/
#define ONE_DATA_NUM        4          /*4个数求平均值*/
#define ALL_DATA_NUM        16         /*16组数求平均值*/
#define DATA_DELAY_NUM      8          /*15-7,16组数据提前7个数据值*/

//#define ASLEEP_MOVE_OFFSET_MIN_VAL      1000       /*睡眠部分的运动阀值*/
//#define ASLEEP_ONE_STEP_PERIOD_MAX_TIME (1000/SAMPLE_TIME_UNINT)    /*睡眠部分运动波峰最长时间1秒，1000ms/40ms=25 */

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
}STEP_COUNT_T;
SPORT_INFO_T Total_Sport_Info_data;
STEP_COUNT_T Step_Count_data = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0},{0,0,0,0,0,0,0,0},0,0,0,0,0,0,0,0,0,0,0};  

static adapter_callback steps_cb = NULL;
static cmd_user_info_t* user_info = NULL;

u16 CaculateAsbSquare(u8 Temp);
void InitVal(u16 *ValS,u16 Val, u8 Length);
u16 AverageVal(u16 *Val,u8 Num);
u16 AverageValPro(u16 *Val,u16 New,u8 Num);
u16 GetXYZ_Acce_Data(void);
void Step_Count_data_Init(void);
void step_count_proce(void);
s16 step_sample_init(adapter_callback cb);

/*????′?·?o?μ?8±èì???êyμ???·?*/
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
   Step_Count_data.LastChangeTime=0;   /*解决一些步数突变的问题*/
   Step_Count_data.StepsChangeTimeBuffer=0; /*解决有时前几步变化的问题*/
}

static void StepCountProce(void);
static void StepCountProce(void)
{
    uint8 i=0,StepFlag=0;
    uint16 Temp=0xFFFF;    
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
        for(i=0;i<32;i++) /*原來?0*/
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
                        Step_Count_data.StepsChangeCount++;
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
                                Step_Count_data.StepsChangeTimeBuffer=0xFF;
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
                            Step_Count_data.StepsChangeCount=HOW_MANY_STEP_BUFFER+1;
                        }
                        Step_Count_data.ChangeTime=1;
                        Step_Count_data.StepsChangePreCount=Step_Count_data.StepsChangeCount;
                    }
                }
            }
        }
    }
}
static void step_sample_handler(u16 id)
{
    StepCountProce();
    static u32 step_count = 0;
    
	get_driver()->timer->timer_start(280, step_sample_handler);
    if(step_count != Total_Sport_Info_data.StepCounts)
    {
        steps_cb(READ_STEPS_CURRENT, NULL);
    }
    step_count = Total_Sport_Info_data.StepCounts;
    #if 0
    u8 val[4] = {0};
    if(step_count != Total_Sport_Info_data.StepCounts)
    {
        val[0] = (Total_Sport_Info_data.StepCounts>>24) & 0x000000FF;
        val[1] = (Total_Sport_Info_data.StepCounts>>16) & 0x000000FF;
        val[2] = (Total_Sport_Info_data.StepCounts>>8) & 0x000000FF;
        val[3] = Total_Sport_Info_data.StepCounts & 0x000000FF;
        BLE_SEND_LOG(val, 4);
    }
    #endif    
}
s16 step_sample_init(adapter_callback cb)
{
	get_driver()->timer->timer_start(280, step_sample_handler); 

    Step_Count_data.Pro_Step=PRO_STEP_START;  
    steps_cb = cb;
	return 0;
}
#if USE_DEV_CALORIE
void sport_minute_calc(void)
{
    u32 bmr_colorie = 0;
    u32 calorie = 0;
    static u32 last_steps = 0;
    static u32 steps = 0;
    
    if(Total_Sport_Info_data.StepCounts == 0) {
        last_steps = 0;
    }
    steps = (Total_Sport_Info_data.StepCounts - last_steps);
    
    if(user_info->gender == 1) {/*男*/                       
       bmr_colorie = user_info->weight*50/3;                    // 16.7*70  =1169  /*1000 / 60*/
       if(steps > 200) calorie = user_info->weight*125/4;       // 31.25*70 =2187.5  /*1000 x 45 /24/60*/ // heavy_colorie
       else if(steps > 100) calorie = user_info->weight*250/9;  // 27.8*70  =1946  /*1000 x 40 /24/60*/ // medium_colorie
       else if(steps > 0) calorie = user_info->weight*875/36;   // 24.3*70  =1701  /*1000 x 35 /24/60*/  // light_colorie
       else calorie = 0;
    } else {/*女*/
       bmr_colorie = user_info->weight*95/6;                    // 15.8*70  =1106  /*950 / 60*/
       if(steps > 200) calorie = user_info->weight*250/9;       // 27.78*70 =1944.6  /*1000 x40 /24/60*/ // heavy_colorie
       else if(steps > 100) calorie = user_info->weight*875/36; // 24.3*70  =1701 /*1000 x35 /24/60*/ // medium_colorie
       else if(steps > 0) calorie = user_info->weight*125/6;    // 20.8*70  =1456   /*1000 x 30 /24/60*/  // light_colorie
       else calorie = 0;
    }
    if(steps >100) {
        Total_Sport_Info_data.AcuteSportTimeCounts++;
    }
    
    bmr_colorie += calorie;
    Total_Sport_Info_data.Calorie += bmr_colorie;    
    last_steps = Total_Sport_Info_data.StepCounts;
}
#endif
SPORT_INFO_T* sport_get(void)
{
    return &Total_Sport_Info_data;
}
void sport_set(cmd_user_info_t *user)
{
    user_info = user;
}
void sport_clear(void)
{
    MemSet(&Total_Sport_Info_data, 0, sizeof(SPORT_INFO_T));
}
