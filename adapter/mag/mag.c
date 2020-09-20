#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h> 
#include "../adapter.h"

#define USE_MAG_SAMPLE_SMOOTH   0

s16 mag_cb_handler(void *args);
u8 calculate_angle(int16 x_val,int16 y_val,int16 z_val);
#if USE_MAG_SAMPLE_SMOOTH
void mag_get_measure_val(s16 temp_val_x, s16 temp_val_y, s16 temp_val_z);
#else
void mag_get_measure_val(void);
#endif
s16 mag_sample_init(void);

typedef struct {
	u8 angle_value;
	u8 measure_flag;
    int16 mag_x_max;
    int16 mag_y_max;
    int16 mag_z_max;
    int16 mag_x_min;
    int16 mag_y_min;
    int16 mag_z_min;
    int16 mag_x_offset;
    int16 mag_y_offset;
    int16 mag_z_offset;
	u16 mag_x_y_z_val;/*jim magnetism*/
}mag_t;

mag_t mag;
#if USE_MAG_SAMPLE_SMOOTH
static mag_data_t data_smooth[5];
#endif


s16 mag_cb_handler(void *args)
{
    /*mag_get_measure_val();

    static u8 angle = 0;
    if(angle != mag.angle_value)
    {
        send_ble((u8*)&mag.angle_value, 1);
        printf("angle: %d\r\n", mag.angle_value);
    }
    angle = mag.angle_value;*/

	return 0;
}

u8 calculate_angle(int16 x_val,int16 y_val,int16 z_val)
{
	u16 tan_table[45]={2,5,9,12,16,19,23,27,31,34,38,42,47,51,55,60,65,70,75,81,87,93,100,107,115,123,133,143,154,166,180,196,214,236,261,290,327,373,433,514,631,814,1143,1908,5729};/*jim magnetism*/
    u8 temp_angle=0,i=0;
    u16 x_temp=0xFF,y_temp=0xFF;
	/*由X,Y,Z三轴的磁力值算出角度*/
	/*X正向北，Y正向东*/
	if((z_val<30)&&(z_val>(-30)))/*Z轴受影响小*/
	{
		if(x_val<0)  /*求绝对值和相象*/
		{
			x_temp=0-x_val;
			if(y_val<0)
			{
				y_temp=0-y_val;
				temp_angle=3;
			}
			else
			{
				y_temp=y_val;
				temp_angle=2;
			}
		}
		else
		{
			x_temp=x_val;
			if(y_val<0)
			{
				y_temp=0-y_val;
				temp_angle=4;
			}
			else
			{
				y_temp=y_val;
				temp_angle=1;
			}
		}
        if(x_temp==0)
		{
			x_temp=45; /*所有角度除于２，一周１８０*/
		}
		else
		{
            y_temp=y_temp*mag.mag_x_y_z_val;/*计算TAN值jim magnetism*/
            y_temp=y_temp/x_temp;
			for(i=0;i<45;i++)   /*查表得角度值*/
				if(y_temp<tan_table[i])
					break;
			x_temp=i;
		}
		if(temp_angle==1)  /*第一相象*/
		{
			 x_temp=180-x_temp;/**/
		}
		else if(temp_angle==2)  /*第二相象*/
		{
			x_temp=90+x_temp;
		}
		else if(temp_angle==3)  /*第三相象*/
		{
			x_temp=90-x_temp;
		}
		else if(temp_angle==4)  /*第四相象*/
		{
           
		}
		else
			x_temp=0xFF;
	}
	return x_temp;
}
#if USE_MAG_SAMPLE_SMOOTH
void mag_get_measure_val(s16 temp_val_x, s16 temp_val_y, s16 temp_val_z)
#else
void mag_get_measure_val(void)
#endif
{
    u8 temp_flag=0;/*X_Temp,Y_Temp,Z_Temp;*/
    u16 Temp=0,Temp1=0;
    #if !USE_MAG_SAMPLE_SMOOTH
    s16 temp_val_x=0x00,temp_val_y=0x00,temp_val_z=0x00;
	mag_data_t data;
    temp_flag = get_driver()->magnetometer->magnetometer_read((void*)&data);
	//temp_flag=MAG3110_I2c_Read(OUT_X_MSB,data_temp,6);
    if(temp_flag != 0x00) {/*获取数据不正确*/
        return;
    }

    temp_val_x=data.mag_xh;
    temp_val_x<<=8;
    temp_val_x|=data.mag_xl;

    temp_val_y=data.mag_yh;
    temp_val_y<<=8;
    temp_val_y|=data.mag_yl;

    temp_val_z=data.mag_zh;
    temp_val_z<<=8;
    temp_val_z|=data.mag_zl;
    #endif

  //if(mag.measure_flag==4)
  {
    if(temp_val_x>mag.mag_x_max) mag.mag_x_max=temp_val_x;
    else if(temp_val_x<mag.mag_x_min)mag.mag_x_min=temp_val_x;
    
    if(temp_val_y>mag.mag_y_max) mag.mag_y_max=temp_val_y; 
    else if(temp_val_y<mag.mag_y_min)mag.mag_y_min=temp_val_y;

    if(temp_val_z>mag.mag_z_max) mag.mag_z_max=temp_val_z;
    else if(temp_val_z<mag.mag_z_min) mag.mag_z_min=temp_val_z;
    
    mag.mag_x_offset=(mag.mag_x_max+mag.mag_x_min)/2;
    mag.mag_y_offset=(mag.mag_y_max+mag.mag_y_min)/2;
    mag.mag_z_offset=(mag.mag_z_max+mag.mag_z_min)/2;
	if((mag.mag_x_max>mag.mag_x_offset)&&(mag.mag_y_max>mag.mag_y_offset))/*jim magnetism*/
	 {
		Temp=mag.mag_x_max-mag.mag_x_offset;
		Temp1=mag.mag_y_max-mag.mag_y_offset;
		mag.mag_x_y_z_val=Temp*100/Temp1; 
	 }

  } 
	/*由X和Y磁感值算出角度*/

    temp_val_x=temp_val_x-mag.mag_x_offset;
    temp_val_y=temp_val_y-mag.mag_y_offset;
    temp_val_z=temp_val_z-mag.mag_z_offset;
             
	temp_flag=calculate_angle(temp_val_x,temp_val_y,0);
	if(temp_flag!=0xFF)
	{
        /*temp_val_x=mag.angle_value+temp_flag;
		mag.angle_value=temp_val_x/2;*/
	    Temp=mag.angle_value;
	    
        temp_val_x=mag.angle_value+temp_flag;/**/

		if((Temp>=(temp_flag+90))||((Temp+90)<=temp_flag))//jim modify 20160413
		    mag.angle_value=(temp_val_x/2+90)%180;	
		else
			mag.angle_value=temp_val_x/2;
	}
}

#if USE_MAG_SAMPLE_SMOOTH
static s16 mag_sample_bubble(s16 *value)
{
    u8 i = 0;
    u8 j = 0;
    s16 tmp_val = 0;

    for(i = 0; i < 4; i++) {
        for(j = i; j < 5; j++) {
            if(value[i] > value[j]) {
                tmp_val = value[i];
                value[i] = value[j];
                value[j] = tmp_val;
            }
        }
    }
    for(i = 1; i < 4; i++) {
        tmp_val += value[i];
    }
    tmp_val /= 3;
    return tmp_val;
}
static void mag_sample_smooth_handler(u16 id)
{
    static s16 temp_val_x[5]={0,0,0,0,0};
    static s16 temp_val_y[5]={0,0,0,0,0};
    static s16 temp_val_z[5]={0,0,0,0,0};
    s16 val_x = 0;
    s16 val_y = 0;
    s16 val_z = 0;
    static u8 sample_cnt = 0;
	mag_data_t data;
    u8 i = 0;

    if(get_driver()->magnetometer->magnetometer_read((void*)&data) == 0x00) {/*获取数据正确*/
        MemCopy(&data_smooth[sample_cnt], &data, sizeof(mag_data_t));
        sample_cnt++;
    }
    if(sample_cnt >= 5) {
        MemSet(&data, 0, sizeof(mag_data_t));
        for(i = 0; i < 5; i++) {
            data.mag_xh += data_smooth[i].mag_xh;
            data.mag_xl += data_smooth[i].mag_xl;
            data.mag_yh += data_smooth[i].mag_yh;
            data.mag_yl += data_smooth[i].mag_yl;
            data.mag_zh += data_smooth[i].mag_yh;
            data.mag_zl += data_smooth[i].mag_yl;
            
            temp_val_x[i]=data.mag_xh;
            temp_val_x[i]<<=8;
            temp_val_x[i]|=data.mag_xl;

            temp_val_y[i]=data.mag_yh;
            temp_val_y[i]<<=8;
            temp_val_y[i]|=data.mag_yl;

            temp_val_z[i]=data.mag_zh;
            temp_val_z[i]<<=8;
            temp_val_z[i]|=data.mag_zl;
        }
        val_x = mag_sample_bubble(temp_val_x);
        val_y = mag_sample_bubble(temp_val_y);
        val_z = mag_sample_bubble(temp_val_z);
        sample_cnt = 0;
        mag_get_measure_val(val_x, val_y, val_z);
        return;
    }
    timer_event(50, mag_sample_smooth_handler);
}
#endif
static void mag_sample_handler(u16 id)
{
    #if USE_MAG_SAMPLE_SMOOTH
    timer_event(1, mag_sample_smooth_handler);
    #else
    mag_get_measure_val();
    #endif

    #if 0
    u8 ble_log[5] = {CMD_TEST_SEND, BLE_LOG_MAG_SAMPLE, 0};
    static u8 angle = 0;
    volatile u8 cur = mag.angle_value;
    if((angle != mag.angle_value) && (key_m_ctrl.compass_adj_state == 0) && (key_m_ctrl.compass_state == 0))
    {
        ble_log[2] = mag.angle_value;
        ble_log[3] = cur/100;
        cur = cur%100;
        ble_log[4] = (cur/10<<4)&0xF0;
        cur = cur%10;
        ble_log[4] |= cur&0x0F;
        BLE_SEND_LOG(ble_log, 5);
    }
    angle = mag.angle_value;
    #endif
    
	timer_event(280, mag_sample_handler);
}

s16 mag_sample_init(void)
{
	timer_event(280, mag_sample_handler);  
	return 0;
}

u8 angle_get(void)
{
    return mag.angle_value;
}
