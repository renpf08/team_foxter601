#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <time.h>
#include <mem.h>
#include "config.h"
#include "adapter.h"

const u8 date[] = {DAY_0,
	DAY_1, DAY_2, DAY_3, DAY_4, DAY_5,
	DAY_6, DAY_7, DAY_8, DAY_9, DAY_10,
	DAY_11, DAY_12, DAY_13, DAY_14, DAY_15,
	DAY_16, DAY_17, DAY_18, DAY_19, DAY_20,
	DAY_21, DAY_22, DAY_23, DAY_24, DAY_25,
	DAY_26, DAY_27, DAY_28, DAY_29, DAY_30,
	DAY_31};

//callback handler
extern s16 mag_cb_handler(void *args);
extern s16 button_cb_handler(void *args);

//module init
extern s16 clock_init(adapter_callback cb);
extern s16 ancs_init(adapter_callback cb);
extern s16 cmd_init(adapter_callback cb);

extern s16 step_sample_init(adapter_callback cb);
extern s16 mag_sample_init(void);
extern s16 ble_switch_init(adapter_callback cb);

s16 csr_event_callback(EVENT_E ev);
void driver_uninit(void);

typedef struct {
	driver_t *drv;
	adapter_callback cb;
}adapter_t;

static adapter_t adapter = {
	.drv = NULL,
	.cb = NULL,
};

u8 day_table[] = {DAY_0,
DAY_1, DAY_2, DAY_3, DAY_4, DAY_5,
DAY_6, DAY_7, DAY_8, DAY_9, DAY_10,
DAY_11, DAY_12, DAY_13, DAY_14, DAY_15,
DAY_16, DAY_17, DAY_18, DAY_19, DAY_20,
DAY_21, DAY_22, DAY_23, DAY_24, DAY_25,
DAY_26, DAY_27, DAY_28, DAY_29, DAY_30,
DAY_31};

s16 csr_event_callback(EVENT_E ev)
{
	if(ev >= EVENT_MAX) {
		return -1;
	}else if(ev < MAGNETOMETER_READY){
		//print((u8 *)&ev, 1);
		u16 combo_event = button_cb_handler((void*)ev);
        if(combo_event < REPORT_MAX) {     // sure the button released
        	adapter.cb(combo_event, NULL);
    	    //adapter.drv->uart->uart_write((u8 *)&combo_event, 1);
        }
	} else if(ev == MAGNETOMETER_READY) {
	    mag_cb_handler((void*)ev);
	}
	
	return 0;
}

static s16 driver_init(void)
{
	//u8 test[25] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};

	adapter.drv = get_driver();
	//timer init
	adapter.drv->timer->timer_init(NULL, NULL);
	adapter.drv->battery->battery_init(NULL, NULL);
	adapter.drv->keya->key_init(&args, csr_event_callback);
	adapter.drv->keym->key_init(&args, csr_event_callback);
	adapter.drv->keyb->key_init(&args, csr_event_callback);	
	adapter.drv->flash->flash_init(NULL, NULL);

    #if USE_UART_PRINT
	//uart init and test
	adapter.drv->uart->uart_init(&args, NULL);
	//adapter.drv->uart->uart_write(test, 23);
	#endif

	//vibrator init
	adapter.drv->vibrator->vibrator_init(&args, NULL);
	adapter.drv->vibrator->vibrator_off(NULL);

	//gsensor init
	adapter.drv->gsensor->gsensor_init(&args, NULL);

	//magnetometer init
	adapter.drv->magnetometer->magnetometer_init(&args, csr_event_callback);

	//motor init and off
	adapter.drv->motor_hour->motor_init(&args, NULL);
	adapter.drv->motor_hour->motor_stop(NULL);
	adapter.drv->motor_minute->motor_init(&args, NULL);
	adapter.drv->motor_minute->motor_stop(NULL);
	adapter.drv->motor_activity->motor_init(&args, NULL);
	adapter.drv->motor_activity->motor_stop(NULL);
	adapter.drv->motor_date->motor_init(&args, NULL);
	adapter.drv->motor_date->motor_stop(NULL);
	adapter.drv->motor_battery_week->motor_init(&args, NULL);
	adapter.drv->motor_battery_week->motor_stop(NULL);
	adapter.drv->motor_notify->motor_init(&args, NULL);
	adapter.drv->motor_notify->motor_stop(NULL);

	return 0;
}

s16 adapter_init(adapter_callback cb)
{
	//driver init
	driver_init();
	adapter.cb = cb;

	//module init
	clock_init(cb);
	ancs_init(cb);
    cmd_init(cb);
	motor_manager_init();
	battery_init(cb);
    step_sample_init(cb);
    mag_sample_init();
    ble_switch_init(cb);
    nvm_storage_init(cb);
	return 0;
}

#if USE_UART_PRINT
void print(u8 *buf, u16 num)
{
	u8 rn[2] = {0x0d, 0x0a};
	if(NULL != adapter.drv) {
		adapter.drv->uart->uart_write(buf, num);
		adapter.drv->uart->uart_write(rn, 2);
	}
}
#endif

#if 0
void print_str_hex(u8 *buf, u16 hex_num)
{
    u8 hex_table[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    u8 i = 0;
    u8 len = StrLen((s8*)buf);
    u8 sbuf[32] = {0};

    for(i = 0; i < len; i++) {
        sbuf[i] = buf[i];
    }
    
    sbuf[i++] = hex_table[(hex_num>>12)&0x000F];
    sbuf[i++] = hex_table[(hex_num>>8)&0x000F];
    sbuf[i++] = hex_table[(hex_num>>4)&0x000F];
    sbuf[i++] = hex_table[hex_num&0x000F];

    print(sbuf, i);
}

void print_str_dec(u8 *buf, u16 dec_num)
{
    u8 dec_table[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    u8 i = 0;
    u16 tmp_num = dec_num;
    u32 mul = 1;
    u8 len = StrLen((s8*)buf);
    u8 sbuf[32] = {0};

    for(i = 0; i < len; i++) {
        sbuf[i] = buf[i];
    }
    do {
        mul *= 10;
        tmp_num /= 10;
    } while(tmp_num != 0);
    if(mul > dec_num) mul /= 10;
    while(mul != 0) {
        sbuf[i++] = dec_table[dec_num/mul];
        dec_num %= mul;
        mul /= 10;
    }

    print(sbuf, i);
}

void print_date_time(u8 *buf, clock_t *datm)
{
    u8 dec_table[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    u8 i = 0;
    u8 len = StrLen((s8*)buf);
    u8 sbuf[32] = {0};
    u16 year = datm->year;

    for(i = 0; i < len; i++) {
        sbuf[i] = buf[i];
    }
    sbuf[i++] = dec_table[year/1000]; year %= 1000;
    sbuf[i++] = dec_table[year/100]; year %= 100;
    sbuf[i++] = dec_table[year/10]; year %= 10;
    sbuf[i++] = dec_table[year];
    sbuf[i++] = '/';
    sbuf[i++] = dec_table[datm->month/10];
    sbuf[i++] = dec_table[datm->month%10];
    sbuf[i++] = '/';
    sbuf[i++] = dec_table[datm->day/10];
    sbuf[i++] = dec_table[datm->day%10];
    sbuf[i++] = ' ';
    sbuf[i++] = dec_table[datm->hour/10];
    sbuf[i++] = dec_table[datm->hour%10];
    sbuf[i++] = ':';
    sbuf[i++] = dec_table[datm->minute/10];
    sbuf[i++] = dec_table[datm->minute%10];
    sbuf[i++] = ':';
    sbuf[i++] = dec_table[datm->second/10];
    sbuf[i++] = dec_table[datm->second%10];
    sbuf[i++] = ' ';
    sbuf[i++] = 'w';
    sbuf[i++] = dec_table[datm->week];

    print(sbuf, i);
}

u8 bcd_to_hex(u8 bcd_data)
{
  u8 x = (bcd_data & 0xF0) >> 4;

    if (x > 9 || (bcd_data & 0x0F) > 9) {
        return 0;
    }
    else {
        return (x << 3) + (x << 1) + (bcd_data & 0x0F);
    }
}
u32 hex_to_bcd(u8 hex_data)
{
    u32 bcd_data;
    u8 temp;
    temp=hex_data%100;
    bcd_data=((unsigned int)hex_data)/100<<8;
    bcd_data=bcd_data|temp/10<<4;
    bcd_data=bcd_data|temp%10;
    return bcd_data;
}
#endif


void timer_event(u16 ms, timer_cb cb)
{
	adapter.drv->timer->timer_start(ms, cb);
}
