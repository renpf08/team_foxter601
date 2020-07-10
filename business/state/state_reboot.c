#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <mem.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"
#include <csr_ota.h>

s16 state_reboot(REPORT_E cb, void *args)
{
    APP_Move_Bonded(4);
    OtaReset();
	return 0;
}
