#include <debug.h>          /* Simple host interface to the UART driver */
#include <mem.h>
#include "../adapter.h"

#define PRO_STEP_IDLE       0
#define PRO_STEP_START      1
#define PRO_STEP_RISING     2
//#define PRO_STEP_STOP       3     /*jim add 20150819�ص��Ʋ�����*/

#define SAMPLE_TIME_UNINT   40         /*����ʱ�䵥λ��40ms*/

#define FILTER_NUM          8          /*�˲���������8������ƽ����ȥ��ͻ��*/
#define ONE_DATA_NUM        4          /*4������ƽ��ֵ*/
#define ALL_DATA_NUM        16         /*16������ƽ��ֵ*/
#define DATA_DELAY_NUM      8          /*15-7,16��������ǰ7������ֵ*/

//#define ASLEEP_MOVE_OFFSET_MIN_VAL      1000       /*˯�߲��ֵ��˶���ֵ*/
//#define ASLEEP_ONE_STEP_PERIOD_MAX_TIME (1000/SAMPLE_TIME_UNINT)    /*˯�߲����˶������ʱ��1�룬1000ms/40ms=25 */

#define QUICKLY_STEP_MIN_VAL            16000/* 8000 */   /*˯���˶���С���ٷ�ֵ�����ڴ�ֵ��Ϊ���ڿ���*/
#define QUICKLY_STEP_OFFSET_MIN_VAL     2000 /*2400*/    /*�첽�ܵ�ƫ��ֵ��ֵ2500*/
#define STEP_OFFSET_MIN_VAL             1100 /* 1600*/ /*1200 */       /*����ʱ��ģ���ٶ�ֵ��Сƫ��ֵ1300*/
#define START_STEP_OFFSET_MIN_VAL       950 /* 1600*/ /*1200 */       /*����ʱ��ģ���ٶ�ֵ��Сƫ��ֵ1300*/

#define QUICKLY_STEP_TIME_MAX           (250/SAMPLE_TIME_UNINT)/*(200/SAMPLE_TIME_UNINT)(300/SAMPLE_TIME_UNINT) */   /*������󲨷�ʱ����0.2�룬200ms/40ms=5*/
#define SLOW_STEP_TIME_MIN              (200/SAMPLE_TIME_UNINT)/* //(250/SAMPLE_TIME_UNINT)*/ /*(400/SAMPLE_TIME_UNINT)*/    /*��������֮�����Сʱ������0.48�룬480ms/40ms=12*/

#define SLOW_ONE_STEP_PERIOD_MIN_TIME   (120/SAMPLE_TIME_UNINT)/*//(160/SAMPLE_TIME_UNINT)*/  /*һ������ʱ��̵Ĳ���ʱ����160ms*/
#define ONE_STEP_PERIOD_MAX_TIME        (1000/SAMPLE_TIME_UNINT) /*(1000/SAMPLE_TIME_UNINT)*/  /*һ����Ĳ���ʱ����1�룬1000ms/40ms=25*/
    
#define TWO_STEP_BETWEEN_MIN_TIME       (120/SAMPLE_TIME_UNINT)    /*����֮����Сʱ���0.12�� 120ms/40ms=3 */
#define TWO_STEP_BETWEEN_MAX_TIME       (1200/SAMPLE_TIME_UNINT)/*//(1200/SAMPLE_TIME_UNINT)*/ /*(1000/SAMPLE_TIME_UNINT)*/ /*(1200/SAMPLE_TIME_UNINT)*/  /*����֮�����ʱ���1.0��  1600*40ms=40 */

#define HOW_MANY_SECOND_BUFFER          (4800/SAMPLE_TIME_UNINT) /*  (4000/SAMPLE_TIME_UNINT)*/   /*4�������5���Ĵ���*/
#define HOW_MANY_STEP_BUFFER            7  /* 6 */      /*��ǰ�������ˣ�����Ҫ����3*/

#define MIDDLE_STEP_COUNT               4        /*������ǰ1.8����Ҫ������3��*/
#define MIDDLE_STEP_SECOND              (2800/SAMPLE_TIME_UNINT)   /*1.8���ʱ��Ҫ����3����ֵ2.8*/

typedef struct
{
     u16  FiFo_DataString_Val[ALL_DATA_NUM];           /*������ٶ�����16�鹲ģֵ*/
     u16  FiFo_OneData_Val[ONE_DATA_NUM];              /*������ٶ�����4����ģֵ*/
     u16  FiFo_Filter_Val[FILTER_NUM];                 /*������ٶ�����4����ģֵ*/
     u16  XYZ_Across_Acce_Min;                  /*XYZȡģ��ֵ�����ֵ*/
     u16  XYZ_Acce_Max_Offset;                  /*XYZȡģ��ֵ�����ֵ���ƫ��ֵ*/
     
     u8   Pro_Step;                  /*���㲽��*/
     u8   DataGetCount;              /* ���ݻ�ȡ����*/
     u8   RisingTime;                /*ȡ������ʱ���ۻ�*/
     u8   FallingFlag;               /*ȡ���½�������־*/ 
     u8   ChangeTime;                /*�����Ʋ����ڲ���֮���ʱ��*/
     u8   LastChangeTime;            /*��һ�������Ʋ����ڲ���֮���ʱ�䣬���һЩͻ�������*/
     u8   StepsChangeTimeBuffer;     /*ǰ����Ĺ��˴����ʱ��*/
     
     u8   StepsChangeCount;          /*����ǰ�����仯����*/
     u8   StepsChangePreCount;       /*�����ı�ǰ����ʱ�����ݴ�*/
}STEP_COUNT_T;
STEP_COUNT_T Step_Count_data = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0},{0,0,0,0,0,0,0,0},0,0,0,0,0,0,0,0,0,0,0};  

static adapter_callback steps_cb = NULL;
static u32 acc_steps;

u16 CaculateAsbSquare(u8 Temp);
void InitVal(u16 *ValS,u16 Val, u8 Length);
u16 AverageVal(u16 *Val,u8 Num);
u16 AverageValPro(u16 *Val,u16 New,u8 Num);
u16 GetXYZ_Acce_Data(void);
s16 step_sample_init(adapter_callback cb);

/*????��?��?o?��?8������???��y��???��?*/
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
/*��ʼ�����ݻ�����*/
void InitVal(u16 *ValS,u16 Val, u8 Length)
{
    u8 i;
    for(i=0;i<Length;i++)
    {
        ValS[i]=Val;
    }
}
/*��ĳ����ļ�����ƽ��ֵ*/
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
/*��ǰ4������ֵ��ƽ��ֵ�����µ��������뻺����*/
u16 AverageValPro(u16 *Val,u16 New,u8 Num)
{
    u32 ReturnVal,MaxVal,MinVal;
    u8 i;
    ReturnVal=0;/**/
    MaxVal=0; /*new caculate,ȥ�����ֵ��ȥ����Сֵ��������ƽ��*/
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
static void StepCountProce(void)
{
    return;
    uint8 i=0,StepFlag=0;
    uint16 Temp=0xFFFF;    
    if(Step_Count_data.Pro_Step==PRO_STEP_START)
    {
        Temp=GetXYZ_Acce_Data();
        if(Temp<0xFFFE)/*ȡ������*/
        {
            MemSet(&Step_Count_data, 0, sizeof(STEP_COUNT_T));
            Step_Count_data.Pro_Step=PRO_STEP_RISING;/**/
            InitVal(Step_Count_data.FiFo_DataString_Val,Temp,ALL_DATA_NUM);/*�洢ÿ��4��ƽ��ֵ�����ݣ�Ŀǰ��16��*/
            InitVal(Step_Count_data.FiFo_Filter_Val,Temp,FILTER_NUM);      /*������ƽ������*/
        }
    }
    else if(Step_Count_data.Pro_Step==PRO_STEP_RISING)
    {
        for(i=0;i<32;i++) /*ԭ��?0*/
        {
            Temp=GetXYZ_Acce_Data();
            if(Temp<0xFFFE){ 
                Temp=AverageValPro(Step_Count_data.FiFo_Filter_Val,Temp,FILTER_NUM);/*������ƽ������*/
                if(Step_Count_data.DataGetCount<ONE_DATA_NUM) 
                {
                    Step_Count_data.FiFo_OneData_Val[Step_Count_data.DataGetCount++]=Temp;
                }
                if(Step_Count_data.DataGetCount>=ONE_DATA_NUM)/*һС�����ݻ�ȡ��ɣ��������ݷ���*/
                {
                    Step_Count_data.DataGetCount=0;
                    if((Step_Count_data.RisingTime)&&(Step_Count_data.RisingTime<255))  Step_Count_data.RisingTime++;  
                    Temp=AverageVal(Step_Count_data.FiFo_OneData_Val,ONE_DATA_NUM);/*���С�����ݵ�ƽ��ֵ*/
                    Temp=AverageValPro(Step_Count_data.FiFo_DataString_Val,Temp,ALL_DATA_NUM);/*�������黺���������ƽ��ֵ*/ 
                    if(Step_Count_data.FiFo_DataString_Val[DATA_DELAY_NUM]>=Temp)/*ƽ��ֵ������ǰ3������Ϊ7-3��4,����ʱ���ݳ���ƽ��ֵʱ*/
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
                                if(Temp>=QUICKLY_STEP_MIN_VAL)/*�����˶�*/
                                {
                                    if((Step_Count_data.RisingTime<=QUICKLY_STEP_TIME_MAX)&&(Step_Count_data.XYZ_Acce_Max_Offset>=QUICKLY_STEP_OFFSET_MIN_VAL))/*������ֵ��*/
                                    {
                                        /*��������*/
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
                        if(Step_Count_data.StepsChangeCount==2)/*�ڶ�����Ч������ʱ�򣬼�¼��������ʱ���*//*�������ͻ�������*/
                        {
                            Step_Count_data.LastChangeTime=Step_Count_data.ChangeTime;
                        }
                        if(Step_Count_data.LastChangeTime<6)
                        {
                            Step_Count_data.LastChangeTime=6;
                        }
                        StepFlag=0;
                    }
                    /*�˶ι���ǰ�����仯��ֻ�����ӳ���6���仯���ٽ��������Ʋ�*/  
                    if((Step_Count_data.ChangeTime>0)&&(Step_Count_data.ChangeTime<255))
                    {
                        Step_Count_data.ChangeTime++;/*40msΪһ��λ*/
                    }
                    /*��������ʱ��Ϊbuffer����*/
                    if((Step_Count_data.StepsChangeTimeBuffer>0)&&(Step_Count_data.StepsChangeTimeBuffer<255))
                    {
                        Step_Count_data.StepsChangeTimeBuffer++;/*40msΪһ��λ*/
                    }
                    if((Step_Count_data.StepsChangeTimeBuffer>MIDDLE_STEP_SECOND)&&(Step_Count_data.StepsChangeCount<MIDDLE_STEP_COUNT))
                    {
                        Step_Count_data.StepsChangeTimeBuffer=1;
                        Step_Count_data.StepsChangeCount=1;
                    }
                    if((Step_Count_data.ChangeTime>=TWO_STEP_BETWEEN_MAX_TIME)||((Step_Count_data.ChangeTime<=TWO_STEP_BETWEEN_MIN_TIME)&&(Step_Count_data.StepsChangeCount>1)&&(Step_Count_data.StepsChangeCount!=Step_Count_data.StepsChangePreCount)))/*1.6������������������Ϊ�Ǽ���,С��0.2�룬Ҳ��Ϊ�Ǽ���*/
                    {
                        Step_Count_data.StepsChangeCount=0;
                        Step_Count_data.ChangeTime=0; 
                        Step_Count_data.StepsChangePreCount=Step_Count_data.StepsChangeCount;
                        Step_Count_data.LastChangeTime=0;/*�������ͻ�������*/
                        Step_Count_data.StepsChangeTimeBuffer=0; /*�����ʱǰ�����仯������*/
                    }
                    else if(Step_Count_data.StepsChangeCount!=Step_Count_data.StepsChangePreCount)
                    {
                        if(Step_Count_data.StepsChangeTimeBuffer<HOW_MANY_SECOND_BUFFER)/*С��3��Ĵ���*/
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
                        else if(Step_Count_data.StepsChangeTimeBuffer<0xFF)/*����3��Ĵ���*/
                        {
                            if(Step_Count_data.StepsChangeCount>=HOW_MANY_STEP_BUFFER)/*ǰ3������Ҫ����6��*/
                            {
                                acc_steps+=Step_Count_data.StepsChangeCount;
                                Step_Count_data.StepsChangeTimeBuffer=0xFF;
                            }
                            else /*��Ϊ�������õĶ���*/
                            {
                                Step_Count_data.StepsChangeCount=0;
                                Step_Count_data.ChangeTime=0; 
                                Step_Count_data.StepsChangePreCount=Step_Count_data.StepsChangeCount;
                                Step_Count_data.LastChangeTime=0;/*�������ͻ�������*/
                                Step_Count_data.StepsChangeTimeBuffer=0; /*�����ʱǰ�����仯������*/  
                            }
                        }
                        else 
                        {
                            acc_steps++;
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
    static u32 step_count = 0;

    StepCountProce();
    if(step_count != acc_steps)
    {
        step_count = acc_steps;
        steps_cb(REFRESH_STEPS, NULL);
    }    
    timer_event(280, step_sample_handler);
}
s16 step_sample_init(adapter_callback cb)
{
	timer_event(280, step_sample_handler); 

    Step_Count_data.Pro_Step=PRO_STEP_START;  
    acc_steps = 0;
    steps_cb = cb;
	return 0;
}
u32 step_get(void)
{
    return acc_steps;
}
void step_clear(void)
{
    acc_steps = 0;
}
#if USE_CMD_TEST_STEP_COUNT
void step_test(u32 steps)
{
    acc_steps += steps;
}
#endif
