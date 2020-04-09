#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <time.h>
#include "../adapter.h"

#define BUTTON_A    0
#define BUTTON_B    1
#define BUTTON_M    2
#define BUTTON_MAX  4
#define BUTTON_NUM  3
#define KEY_LONG_PRESS_INTERVAL 2 /* unit: second */
#define KEY_SHORT_PRESS_MASK    0x0F
#define KEY_LONG_PRESS_MASK     0xF0

s16 button_down_handler(EVENT_E ev, u8 idx);
s16 button_up_handler(EVENT_E ev, u8 idx);
s16 button_cb_handler(void *args);

typedef struct {
	TIME down_time[BUTTON_NUM];
	TIME up_time[BUTTON_NUM];
	u8 press_state;
	u8 press_down;

}bubtton_t;
bubtton_t button_state = {.press_state=0, .press_down=0};

typedef enum {
    KEYS_A_LONG_PRESS_VALUE         = (0x10<<BUTTON_A),
    KEYS_B_LONG_PRESS_VALUE         = (0x10<<BUTTON_B),
    KEYS_M_LONG_PRESS_VALUE         = (0x10<<BUTTON_M),
    KEYS_A_B_LONG_PRESS_VALUE       = ((0x10<<BUTTON_A)|(0x10<<BUTTON_B)),
    KEYS_A_M_LONG_PRESS_VALUE       = ((0x10<<BUTTON_A)|(0x10<<BUTTON_M)),
    KEYS_B_M_LONG_PRESS_VALUE       = ((0x10<<BUTTON_B)|(0x10<<BUTTON_M)),
    KEYS_A_B_M_LONG_PRESS_VALUE     = ((0x10<<BUTTON_A)|(0x10<<BUTTON_B)|(0x10<<BUTTON_M)),
    KEYS_A_SHORT_PRESS_VALUE        = (0x01<<BUTTON_A),
    KEYS_B_SHORT_PRESS_VALUE        = (0x01<<BUTTON_B),
    KEYS_M_SHORT_PRESS_VALUE        = (0x01<<BUTTON_M),
    KEYS_A_B_SHORT_PRESS_VALUE      = ((0x01<<BUTTON_A)|(0x01<<BUTTON_B)),
    KEYS_A_M_SHORT_PRESS_VALUE      = ((0x01<<BUTTON_A)|(0x01<<BUTTON_M)),
    KEYS_B_M_SHORT_PRESS_VALUE      = ((0x01<<BUTTON_B)|(0x01<<BUTTON_M)),
    KEYS_A_B_M_SHORT_PRESS_VALUE    = ((0x01<<BUTTON_A)|(0x01<<BUTTON_B)|(0x01<<BUTTON_M)),
    STATE_MAX                       = 0xFF
}KEY_STATE_E;

typedef struct {
    REPORT_E report_value;
    KEY_STATE_E state_value;
}button_report_t;

static button_report_t button_report[] = {
    {KEY_A_LONG_PRESS, KEYS_A_LONG_PRESS_VALUE},
    {KEY_A_SHORT_PRESS, KEYS_A_SHORT_PRESS_VALUE},
    {KEY_B_LONG_PRESS, KEYS_B_LONG_PRESS_VALUE},
    {KEY_B_SHORT_PRESS, KEYS_B_SHORT_PRESS_VALUE},
    {KEY_M_LONG_PRESS, KEYS_M_LONG_PRESS_VALUE},
    {KEY_M_SHORT_PRESS, KEYS_M_SHORT_PRESS_VALUE},
    {KEY_A_B_LONG_PRESS, KEYS_A_B_LONG_PRESS_VALUE},
    {KEY_A_B_SHORT_PRESS, KEYS_A_B_SHORT_PRESS_VALUE},
    {KEY_A_M_LONG_PRESS, KEYS_A_M_LONG_PRESS_VALUE},
    {KEY_A_M_SHORT_PRESS, KEYS_A_M_SHORT_PRESS_VALUE},
    {KEY_B_M_LONG_PRESS, KEYS_B_M_LONG_PRESS_VALUE},
    {KEY_B_M_SHORT_PRESS, KEYS_B_M_SHORT_PRESS_VALUE},
    {KEY_A_B_M_LONG_PRESS, KEYS_A_B_M_LONG_PRESS_VALUE},
    {KEY_A_B_M_SHORT_PRESS, KEYS_A_B_M_SHORT_PRESS_VALUE},
    {REPORT_MAX, STATE_MAX},
};

REPORT_E csr_key_event_recognize(u8 key_state);
REPORT_E csr_key_event_recognize(u8 key_state)
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

    return key_report;
}

s16 button_down_handler(EVENT_E ev, u8 idx)
{
    button_state.down_time[idx] = TimeGet32()/SECOND;

    button_state.press_state &= ~(0x01<<ev); /* clear short press state */
    button_state.press_state &= ~((0x10<<ev)<<4); /* clear long press state */
    button_state.press_down |= (1<<ev); /* set button down flag */

    return 0;
}

s16 button_up_handler(EVENT_E ev, u8 idx)
{
	button_state.up_time[idx] = TimeGet32()/SECOND;

	u8 button_interval = 
		(button_state.up_time[idx]>button_state.down_time[idx])?
		(button_state.up_time[idx]-button_state.down_time[idx]):
		(button_state.down_time[idx]>button_state.up_time[idx]);

	if(button_interval >= KEY_LONG_PRESS_INTERVAL) /* button long press */
	{
		//button_state.press_state &= ~(0x01<<ev);
		button_state.press_state |= (0x01<<ev);
	}
	else  /* button short press */
	{
		button_state.press_state |= ((0x01<<ev)>>4);
		//button_state.press_state &= ~(0x10<<ev);
	}
	button_state.press_down &= ~((0x01<<ev)>>4);

    return 1;
}

s16 button_cb_handler(void *args)
{
    EVENT_E button_event = (EVENT_E)args;
    u8 button_release = 0;
    u8 button_idx = 0;
    
    if((button_event==KEY_A_DOWN)||(button_event==KEY_A_UP)) button_idx = BUTTON_A;
    if((button_event==KEY_B_DOWN)||(button_event==KEY_B_UP)) button_idx = BUTTON_B;
    if((button_event==KEY_M_DOWN)||(button_event==KEY_M_UP)) button_idx = BUTTON_M;

    if((button_event == KEY_A_DOWN) || (button_event == KEY_B_DOWN) || (button_event == KEY_M_DOWN))
    {
        button_release = button_down_handler(button_event, button_idx);
    }
    else if((button_event == KEY_A_UP) || (button_event == KEY_B_UP) || (button_event == KEY_M_UP))
    {
        button_release = button_up_handler(button_event, button_idx);
    }

    if((button_release == 1) && (button_state.press_down == 0))
    {
        csr_key_event_recognize(button_state.press_state);
        button_state.press_state = 0;
    }
    
    return 0;
}

