#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <macros.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "../business.h"
#include "state.h"

#define PAIR_SWITCH_INTERVAL   50

STATE_E *paired_state = NULL;

static void paired_mis_match_cb_handler(u16 id)
{
    *paired_state = PAIR_CODE_MATCHING;
    print((u8*)&"paired mis-match", 16);
    
    pair_code_generate();
}

s16 state_paired_code_matching(REPORT_E cb, void *args)
{
	paired_state = (STATE_E *)args;
    
	//u8 string[13] = {'s', 't', 'a', 't', 'e', '_', 'b', 'l', 'e', '_', 'p', 'e', 'd'};
	//print(string, 13);

    u8* code = cmd_get()->pair_code.code;
    u16 paired_code = (code[0]<<8)|code[1];
    u16 gen_code = pair_code_get()->pair_code;
    print_str_hex((u8*)&"recv paired code=0x", paired_code);

    *paired_state = STATE_MAX;
    if(paired_code == gen_code) {
        *paired_state = CLOCK;
        print((u8*)&"paried matched", 14);
    } else {
        timer_event(PAIR_SWITCH_INTERVAL, paired_mis_match_cb_handler);
    }

	return 0;
}

