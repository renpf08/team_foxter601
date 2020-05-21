#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <macros.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "state.h"

#define PAIR_SWITCH_INTERVAL   50

STATE_E *paired_state = NULL;

static void paired_mis_match_cb_handler(u16 id)
{
    *paired_state = PAIR_CODE_MATCHING;
    print((u8*)&"paired mis-match", 16);
}

s16 state_paired_code_matching(REPORT_E cb, void *args)
{
	paired_state = (STATE_E *)args;
    
	//u8 string[13] = {'s', 't', 'a', 't', 'e', '_', 'b', 'l', 'e', '_', 'p', 'e', 'd'};
	//print(string, 13);

    u8 test_table[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    u8 test_code[16] = {"paired code=xxxx"};

    u8* code = cmd_get()->pair_code.code;
    u16 pair_code = (code[0]<<8)|code[1];

    test_code[12] = test_table[(pair_code>>12)&0x000F];
    test_code[13] = test_table[(pair_code>>8)&0x000F];
    test_code[14] = test_table[(pair_code>>4)&0x000F];
    test_code[15] = test_table[pair_code&0x000F];
    print(test_code, 16);

    *paired_state = STATE_MAX;
    if(pair_code == 0x1234) {
        *paired_state = CLOCK;
        print((u8*)&"paried matched", 14);
    } else {
        timer_event(PAIR_SWITCH_INTERVAL, paired_mis_match_cb_handler);
    }

	return 0;
}

