#include "../common/common.h"
#include "driver.h"

extern timer_t csr_timer;
extern uart_t csr_uart;
extern battery_t csr_battery;
extern charge_t csr_charge;
extern key_t csr_keya;
extern key_t csr_keym;
extern key_t csr_keyb;
extern flash_t csr_flash;
extern vibrator_t csr_vibrator;
extern gsensor_t csr_gsensor;
extern magnetometer_t csr_magnetometer;
extern motor_t csr_motor_hour;
extern motor_t csr_motor_minute;
extern motor_t csr_motor_activity;
extern motor_t csr_motor_date;
extern motor_t csr_motor_battery_week;
extern motor_t csr_motor_notify;

static driver_t csr_driver = {
	.timer = &csr_timer,
    #if USE_UART_PRINT
	.uart = &csr_uart,
	#endif
	.battery = &csr_battery,
	.charge = &csr_charge,
	.keya = &csr_keya,
	.keyb = &csr_keyb,
	.keym = &csr_keym,
	.flash = &csr_flash,
	.vibrator = &csr_vibrator,
	.gsensor = &csr_gsensor,
	.magnetometer = &csr_magnetometer,
	.motor_hour = &csr_motor_hour,
	.motor_minute = &csr_motor_minute,
	.motor_activity = &csr_motor_activity,
	.motor_date = &csr_motor_date,
	.motor_battery_week = &csr_motor_battery_week,
	.motor_notify = &csr_motor_notify,
};

driver_t *get_driver(void)
{
	return &csr_driver;
}

s16 timer_create(uint32 timeout, timer_callback_arg handler)
{
    const timer_id tId = TimerCreate(timeout, TRUE, handler);
    
	//ReportPanic(__FILE__, __func__, __LINE__, app_timer_create_fail + tId);
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID)
    {    
        #if USE_PANIC_PRINT
		ReportPanic(__FILE__, __func__, __LINE__, app_timer_create_fail);
        //DebugWriteString("\r\nFailed to start timer");
        /* Panic with panic code 0xfe */
        //Panic(0xfe);
        #endif
    }
	return tId;
}
