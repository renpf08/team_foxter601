#include "../common/common.h"
#include "driver.h"

extern timer_t csr_timer;
extern uart_t csr_uart;
extern battery_t csr_battery;
extern key_t csr_keya;
extern key_t csr_keym;
extern key_t csr_keyb;
extern flash_t csr_flash;
extern vibrator_t csr_vibrator;
extern gsensor_t csr_gsensor;
extern magnetometer_t csr_magnetometer;
extern motor_ctrl_t csr_motor_ctrl;

static driver_t csr_driver = {
	.timer = &csr_timer,
    #if USE_UART_PRINT
	.uart = &csr_uart,
	#endif
	.battery = &csr_battery,
	.keya = &csr_keya,
	.keyb = &csr_keyb,
	.keym = &csr_keym,
	.flash = &csr_flash,
	.vibrator = &csr_vibrator,
	.gsensor = &csr_gsensor,
	.magnetometer = &csr_magnetometer,
	.motor_ctrl = &csr_motor_ctrl,
};

driver_t *get_driver(void)
{
	return &csr_driver;
}

void timer_create(uint32 timeout, timer_callback_arg handler)
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
}
