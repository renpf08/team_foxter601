#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "config.h"
#include "adapter.h"

//callback handler
extern s16 button_A_down_cb_handler(void *args);
extern s16 button_A_up_cb_handler(void *args);
extern s16 button_M_down_cb_handler(void *args);
extern s16 button_M_up_cb_handler(void *args);
extern s16 button_B_down_cb_handler(void *args);
extern s16 button_B_up_cb_handler(void *args);
extern s16 mag_cb_handler(void *args);

//module init
extern s16 clock_init(void);

s16 csr_event_callback(EVENT_E ev);
void driver_uninit(void);

static driver_callback_handler driver_cb[] = {
	[KEY_A_UP] =  button_A_up_cb_handler,
	[KEY_A_DOWN] = button_A_down_cb_handler,
	[KEY_B_UP] = button_B_up_cb_handler,
	[KEY_B_DOWN] = button_B_down_cb_handler,
	[KEY_M_UP] = button_M_up_cb_handler,
	[KEY_M_DOWN] = button_M_down_cb_handler,
	[MAGNETOMETER_READY] = mag_cb_handler,
};

static button_state_t button_state = {0, 0};

static button_report_t button_report[] = {
    {KEY_A_LONG_PRESS, KEYS_A_LONG_PRESS_VALUE, 16},
    {KEY_A_SHORT_PRESS, KEYS_A_SHORT_PRESS_VALUE, 17},
    {KEY_B_LONG_PRESS, KEYS_B_LONG_PRESS_VALUE, 16},
    {KEY_B_SHORT_PRESS, KEYS_B_SHORT_PRESS_VALUE, 17},
    {KEY_M_LONG_PRESS, KEYS_M_LONG_PRESS_VALUE, 16},
    {KEY_M_SHORT_PRESS, KEYS_M_SHORT_PRESS_VALUE, 17},
    {KEY_A_B_LONG_PRESS, KEYS_A_B_LONG_PRESS_VALUE, 18},
    {KEY_A_B_SHORT_PRESS, KEYS_A_B_SHORT_PRESS_VALUE, 19},
    {KEY_A_M_LONG_PRESS, KEYS_A_M_LONG_PRESS_VALUE, 18},
    {KEY_A_M_SHORT_PRESS, KEYS_A_M_SHORT_PRESS_VALUE, 19},
    {KEY_B_M_LONG_PRESS, KEYS_B_M_LONG_PRESS_VALUE, 18},
    {KEY_B_M_SHORT_PRESS, KEYS_B_M_SHORT_PRESS_VALUE, 19},
    {KEY_A_B_M_LONG_PRESS, KEYS_A_B_M_LONG_PRESS_VALUE, 20},
    {KEY_A_B_M_SHORT_PRESS, KEYS_A_B_M_SHORT_PRESS_VALUE, 21},
    {REPORT_MAX, STATE_MAX, 10},
};

typedef struct {
	driver_t *drv;
	adapter_callback cb;
}adapter_t;

static adapter_t adapter = {
	.drv = NULL,
	.cb = NULL,
};

#define STRINGIFY(val) #val
#define M_VALUE_TO_STR(name) [name] = STRINGIFY(name)
static const char * keys_state_str[] =
{
    M_VALUE_TO_STR(KEY_A_LONG_PRESS),
    M_VALUE_TO_STR(KEY_A_SHORT_PRESS),
    M_VALUE_TO_STR(KEY_B_LONG_PRESS),
    M_VALUE_TO_STR(KEY_B_SHORT_PRESS),
    M_VALUE_TO_STR(KEY_M_LONG_PRESS),
    M_VALUE_TO_STR(KEY_M_SHORT_PRESS),
    M_VALUE_TO_STR(KEY_A_B_LONG_PRESS),
    M_VALUE_TO_STR(KEY_A_B_SHORT_PRESS),
    M_VALUE_TO_STR(KEY_A_M_LONG_PRESS),
    M_VALUE_TO_STR(KEY_A_M_SHORT_PRESS),
    M_VALUE_TO_STR(KEY_B_M_LONG_PRESS),
    M_VALUE_TO_STR(KEY_B_M_SHORT_PRESS),
    M_VALUE_TO_STR(KEY_A_B_M_LONG_PRESS),
    M_VALUE_TO_STR(KEY_A_B_M_SHORT_PRESS),
    M_VALUE_TO_STR(REPORT_MAX)
};

#include "serial_service.h" /* just for test purpose */
REPORT_E csr_key_event_rec(u8 key_state);
REPORT_E csr_key_event_rec(u8 key_state)
{
    u8 i = 0;
    u8 press_keys = 0;
    REPORT_E key_report = REPORT_MAX;
    
    if(key_state & KEY_LONG_PRESS_MASK)
    {
        press_keys = (key_state&0xF0)|((key_state<<4)&0xF0);
    }
    else if(key_state & KEY_SHORT_PRESS_MASK)
    {
        press_keys = (key_state&0x0F)|((key_state>>4)&0x0F);
    }
    
    while(button_report[i].state_value != STATE_MAX)
    {
        if(button_report[i].state_value == press_keys)
        {
            key_report = button_report[i].report_value;
            break;
        }
        i++;
    }
    
    u8* key_str = (u8*)&keys_state_str[key_report];
    SerialSendNotification((u8*)key_str[0], button_report[i].report_str_len); /* just for test purpose */
    
    return key_report;
}

s16 csr_event_callback(EVENT_E ev)
{
    if(ev >= EVENT_MAX) {
        return -1;
    }else if(ev < MAGNETOMETER_READY){
    	adapter.drv->uart->uart_write((u8 *)&ev, 1);
        driver_cb[ev]((void*)&button_state);
        if(button_state.press_down == 0) {  /* All button release */
            csr_key_event_rec(button_state.press_state);
            button_state.press_state = 0;
        }
    }
	
	return 0;
}

static s16 driver_init(void)
{
	u8 test[25] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};

	adapter.drv = get_driver();
	//timer init
	adapter.drv->timer->timer_init(NULL, NULL);
	adapter.drv->battery->battery_init(NULL, NULL);
	adapter.drv->keya->key_init(&args, csr_event_callback);
	adapter.drv->keym->key_init(&args, csr_event_callback);
	adapter.drv->keyb->key_init(&args, csr_event_callback);	
	adapter.drv->flash->flash_init(NULL, NULL);

	//uart init and test
	adapter.drv->uart->uart_init(&args, NULL);
	adapter.drv->uart->uart_write(test, 23);

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

void driver_uninit(void)
{
	//timer uninit
	adapter.drv->timer->timer_uninit();
	adapter.drv->battery->battery_uninit();
	adapter.drv->keya->key_uninit();
	adapter.drv->flash->flash_uninit();

	//uart uninit
	adapter.drv->uart->uart_uninit();

	//vibrator uninit	
	adapter.drv->vibrator->vibrator_off(NULL);
	adapter.drv->vibrator->vibrator_uninit();

	//gsensor uninit
	adapter.drv->gsensor->gsensor_uninit();

	//magnetometer uninit
	adapter.drv->magnetometer->magnetometer_uninit();

	//motor uninit
	adapter.drv->motor_hour->motor_uninit();
	adapter.drv->motor_minute->motor_uninit();
	adapter.drv->motor_activity->motor_uninit();
	adapter.drv->motor_date->motor_uninit();
	adapter.drv->motor_battery_week->motor_uninit();
	adapter.drv->motor_notify->motor_uninit();
	
	adapter.drv = NULL;
}

s16 adapter_init(adapter_callback cb)
{
	//driver init
	driver_init();
	adapter.cb = cb;

	//module init
	clock_init();

	return 0;
}

s16 adapter_uninit()
{
	driver_uninit();
	adapter.cb = NULL;
	return 0;
}

#if 0
u8 i = 0, j = 0;
for(i = 0; i < 10; i++) {
	driver->motor_hour->motor_positive_first_half(NULL);
	TimeDelayUSec(5 * MILLISECOND);
	driver->motor_hour->motor_stop(NULL);
	driver->motor_hour->motor_positive_second_half(NULL);
	TimeDelayUSec(5 * MILLISECOND);
	driver->motor_hour->motor_stop(NULL);
	
	for(j = 0; j < 20; j++) {
		TimeDelayUSec(50 * MILLISECOND);
	}
}

for(i = 0; i < 10; i++) {
	driver->motor_hour->motor_negtive_first_half(NULL);
	TimeDelayUSec(5 * MILLISECOND);
	driver->motor_hour->motor_stop(NULL);
	driver->motor_hour->motor_negtive_second_half(NULL);
	TimeDelayUSec(5 * MILLISECOND);
	driver->motor_hour->motor_stop(NULL);
	
	for(j = 0; j < 20; j++) {
		TimeDelayUSec(50 * MILLISECOND);
	}
}

driver->timer->timer_start(2000, csr_timer_cb);
m_printf("battery voltage[%d]\r\n",driver->battery->battery_voltage_read(NULL));
#endif
