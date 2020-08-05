#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../adapter.h"

#define BATTERY_FULL             4200
#define BATTERY_NORMAL_THRESHOLD 3000
#define BATTERY_LOW_THRESHOLD    2700

#define PERCENT_TABLE_NUM 10
#define PERCENT_VOL(perc) (BATTERY_LOW_THRESHOLD + (BATTERY_FULL - BATTERY_LOW_THRESHOLD) / 100.0f * (perc))

static void bat_cb_handler(u16 id);

typedef struct {
	u16 voltage;
	u8  percent;
}percent_table_t;

static percent_table_t percent_table[PERCENT_TABLE_NUM] = {
	{PERCENT_VOL(90), BAT_PECENT_90},
	{PERCENT_VOL(80), BAT_PECENT_80},
	{PERCENT_VOL(70), BAT_PECENT_70},
	{PERCENT_VOL(60), BAT_PECENT_60},
	{PERCENT_VOL(50), BAT_PECENT_50},
	{PERCENT_VOL(40), BAT_PECENT_40},
	{PERCENT_VOL(30), BAT_PECENT_30},
	{PERCENT_VOL(20), BAT_PECENT_20},
	{PERCENT_VOL(10), BAT_PECENT_10},
	{PERCENT_VOL(0), BAT_PECENT_0},	
};

typedef struct {
	driver_t *drv;
	adapter_callback cb;
	u16 val;
	u8 percent;
	u8 cur_status;
	u8 old_status;
}bat_cfg_t;

static bat_cfg_t bat_cfg = {
	.drv = NULL,
	.cb = NULL,
	.val = 0,
	.percent = 0,
	.cur_status = BATTERY_NORMAL,
	.old_status = BATTERY_NORMAL,
};

u8 battery_percent_read(void)
{
	u8 i = 0;

	if(bat_cfg.val >= BATTERY_FULL) {
		return BAT_PECENT_100;
	}else if(bat_cfg.val <= BATTERY_LOW_THRESHOLD) {
		return BAT_PECENT_0;
	}else {
		for(i = 0; i < PERCENT_TABLE_NUM; i++) {
			if(bat_cfg.val >= percent_table[i].voltage) {
				return percent_table[i].percent;
			}
		}
	}
	return BAT_PECENT_0;
}

static void bat_cb_handler(u16 id)
{
	/*start another loop*/
	bat_cfg.drv->timer->timer_start(3000, bat_cb_handler);

	/*get current voltage*/
	bat_cfg.val = (u16)bat_cfg.drv->battery->battery_voltage_read(NULL);
	if(bat_cfg.val > BATTERY_NORMAL_THRESHOLD) {
		bat_cfg.cur_status = BATTERY_NORMAL;
	}else if(bat_cfg.val < BATTERY_LOW_THRESHOLD) {
		bat_cfg.cur_status = BATTERY_LOW;
	}

	/*callback now*/
	if(bat_cfg.cur_status != bat_cfg.old_status) {
		//bat_cfg.cb(bat_cfg.cur_status, NULL);
	}

	/*current status copy to the old status*/
	bat_cfg.old_status = bat_cfg.cur_status;
}

s16 battery_init(adapter_callback cb)
{
	bat_cfg.drv = get_driver();
	bat_cfg.cb = cb;
	bat_cfg.cb(BATTERY_NORMAL, NULL);
    bat_cb_handler(0); // confirm to read valid voltage before motor set
	//bat_cfg.drv->timer->timer_start(1, bat_cb_handler);
	return 0;
}
