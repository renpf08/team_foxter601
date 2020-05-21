#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <macros.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

s16 state_ble_disconnect(REPORT_E cb, void *args)
{
	//u8 string[13] = {'s', 't', 'a', 't', 'e', '_', 'b', 'l', 'e', '_', 'c', 'o', 'n'};
	//print(string, 13);
	
    print((u8*)&"discon", 6);

	return 0;
}


