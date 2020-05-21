#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <macros.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

#define PAIR_SWITCH_INTERVAL   50

STATE_E *pairing_state = NULL;

static void pairing_code_disp_cb_handler(u16 id)
{
    *pairing_state = PAIR_CODE_MATCHING;
    print((u8*)&"pairing bgn", 11);
}
static void pairing_mis_match_cb_handler(u16 id)
{
    *pairing_state = PAIR_CODE_GENERATE;
    print((u8*)&"pairing mis-match", 17);
}
s16 state_pairing_code_generate(REPORT_E cb, void *args)
{
	pairing_state = (STATE_E *)args;
    
	//u8 string[13] = {'s', 't', 'a', 't', 'e', '_', 'b', 'l', 'e', '_', 'p', 'n', 'g'};
	//print(string, 13);

    u8* code = cmd_get()->pair_code.code;
    u16 pair_code = (code[0]<<8)|code[1];

    u8 test_table[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    u8 test_code[17] = {"pairing code=xxxx"};

    test_code[13] = test_table[(pair_code>>12)&0x000F];
    test_code[14] = test_table[(pair_code>>8)&0x000F];
    test_code[15] = test_table[(pair_code>>4)&0x000F];
    test_code[16] = test_table[pair_code&0x000F];
    print(test_code, 17);
    
    *pairing_state = STATE_MAX;
    if(pair_code == 0xFFFF) {
        timer_event(PAIR_SWITCH_INTERVAL, pairing_code_disp_cb_handler);
    } else if(pair_code == 0x1234) {
        *pairing_state = CLOCK;
        print((u8*)&"pairing match", 13);
    } else {
        timer_event(PAIR_SWITCH_INTERVAL, pairing_mis_match_cb_handler);
    }

	return 0;
}

