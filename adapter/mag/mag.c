#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../adapter.h"

s16 mag_cb_handler(void *args);
u8 calculate_angle(int16 x_val,int16 y_val,int16 z_val);
void mag_get_measure_val(void);
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

void mag_get_measure_val(void)
{
    u8 temp_flag=0;/*X_Temp,Y_Temp,Z_Temp;*/
    s16 temp_val_x=0x00,temp_val_y=0x00,temp_val_z=0x00;
    u16 Temp=0,Temp1=0;
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
static void mag_sample_handler(u16 id)
{
    mag_get_measure_val();

    #if 0
    static u8 angle = 0;
    u8 value[2] = {0, 0};
    u8 cur = mag.angle_value;
    if(angle != mag.angle_value)
    {
        value[0] = cur/100;
        cur = cur%100;
        value[1] = (cur/10<<4)&0xF0;
        cur = cur%10;
        value[1] |= cur&0x0F;
        send_ble(value, 2);
        printf("angle: %d\r\n", mag.angle_value);
    }
    angle = mag.angle_value;
    #endif
    
	get_driver()->timer->timer_start(280, mag_sample_handler);
}
s16 mag_sample_init(void)
{
	get_driver()->timer->timer_start(280, mag_sample_handler);  
	return 0;
}
u8 angle_get(void)
{
    return mag.angle_value;
}
