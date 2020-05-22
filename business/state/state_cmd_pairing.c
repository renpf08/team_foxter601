#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <macros.h>
#include <random.h>

#include "../../common/common.h"
#include "../../adapter/adapter.h"
#include "../business.h"
#include "state.h"

#define PAIR_SWITCH_INTERVAL   50
STATE_E *pairing_state = NULL;
pair_code_t pair_code = {0, 0, 0};

void pair_code_generate(void)
{
    u16 old_pair_code = 0;
    u8 bcd_hour = 0;
    u8 bcd_minute = 0;
    
    while(1) {
        old_pair_code = pair_code.pair_code;
        pair_code.pair_code = Random16();
        while((pair_code.pair_code == 0) || (pair_code.pair_code == 0xFFFF)) {
            pair_code.pair_code = Random16();
        }
        pair_code.hour = (pair_code.pair_code>>8)&0x00FF;
        pair_code.minute = pair_code.pair_code&0x00FF;
        while(pair_code.hour >= 12) {
            pair_code.hour %= 12;
        }
        while(pair_code.minute >= 12) {
            pair_code.minute %= 12;
        }
        pair_code.minute *= 5;
        bcd_hour = hex_to_bcd(pair_code.hour);
        bcd_minute = hex_to_bcd(pair_code.minute);
        pair_code.pair_code = (bcd_hour<<8)|bcd_minute;
        if(pair_code.pair_code != old_pair_code) {
            break;
        }
    }
	motor_hour_to_position(pair_code.hour);
	motor_minute_to_position(pair_code.minute);
    print_str_hex((u8*)&"gen pair code=0x", pair_code.pair_code);
    //print_str_dec((u8*)&"hour=", pair_code.hour);
    //print_str_dec((u8*)&"minute=", pair_code.minute);
}
static void pairing_code_disp_cb_handler(u16 id)
{
    pair_code_generate();
    *pairing_state = PAIR_CODE_MATCHING;
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
    u16 pairing_code = (code[0]<<8)|code[1];
    print_str_hex((u8*)&"recv pairing code=0x", pairing_code);

    *pairing_state = STATE_MAX;
    if(pairing_code == 0xFFFF) {
        timer_event(PAIR_SWITCH_INTERVAL, pairing_code_disp_cb_handler);
    } else if(pairing_code == pair_code.pair_code) {
        *pairing_state = CLOCK;
        print((u8*)&"pairing match", 13);
    } else {
        timer_event(PAIR_SWITCH_INTERVAL, pairing_mis_match_cb_handler);
    }

	return 0;
}
pair_code_t *pair_code_get(void)
{
    return &pair_code;
}

