#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include <time.h>
#include "../adapter.h"

#define BUTTON_NUM                  3
#define BUTTON_LONG_PRESS_INTERVAL  2 /* unit: second */
#define BUTTON_SHORT_PRESS_MASK     0x0F
#define BUTTON_LONG_PRESS_MASK      0xF0

typedef enum {
    BUTTON_A,
    BUTTON_B,
    BUTTON_M,
    MAX,
}BUTTON_INDEX_E;

typedef enum {
    BUTTON_UP,
    BUTTON_DOWN,
}BUTTON_EVENT_E;

typedef enum {
    BUTTON_A_LONG_PRESS_VALUE       = (0x10<<BUTTON_A),
    BUTTON_B_LONG_PRESS_VALUE       = (0x10<<BUTTON_B),
    BUTTON_M_LONG_PRESS_VALUE       = (0x10<<BUTTON_M),
    BUTTON_A_B_LONG_PRESS_VALUE     = ((0x10<<BUTTON_A)|(0x10<<BUTTON_B)),
    BUTTON_A_M_LONG_PRESS_VALUE     = ((0x10<<BUTTON_A)|(0x10<<BUTTON_M)),
    BUTTON_B_M_LONG_PRESS_VALUE     = ((0x10<<BUTTON_B)|(0x10<<BUTTON_M)),
    BUTTON_A_B_M_LONG_PRESS_VALUE   = ((0x10<<BUTTON_A)|(0x10<<BUTTON_B)|(0x10<<BUTTON_M)),
    BUTTON_A_SHORT_PRESS_VALUE      = (0x01<<BUTTON_A),
    BUTTON_B_SHORT_PRESS_VALUE      = (0x01<<BUTTON_B),
    BUTTON_M_SHORT_PRESS_VALUE      = (0x01<<BUTTON_M),
    BUTTON_A_B_SHORT_PRESS_VALUE    = ((0x01<<BUTTON_A)|(0x01<<BUTTON_B)),
    BUTTON_A_M_SHORT_PRESS_VALUE    = ((0x01<<BUTTON_A)|(0x01<<BUTTON_M)),
    BUTTON_B_M_SHORT_PRESS_VALUE    = ((0x01<<BUTTON_B)|(0x01<<BUTTON_M)),
    BUTTON_A_B_M_SHORT_PRESS_VALUE  = ((0x01<<BUTTON_A)|(0x01<<BUTTON_B)|(0x01<<BUTTON_M)),
    STATE_MAX                       = 0xFF
}BUTTON_STATE_E;

typedef struct {
	TIME down_time[BUTTON_NUM];
	TIME up_time[BUTTON_NUM];
	u8 press_state;
	u8 press_down;
}button_t;

typedef struct {
    REPORT_E report_value;
    BUTTON_STATE_E state_value;
}button_state_t;

typedef struct {
    EVENT_E ev;
    BUTTON_INDEX_E idx;
    BUTTON_STATE_E sta;
    u8 short_press_mask;
    u8 long_press_mask;
}button_event_t;

static button_state_t button_state[] = {
    {KEY_A_LONG_PRESS,      BUTTON_A_LONG_PRESS_VALUE},
    {KEY_A_SHORT_PRESS,     BUTTON_A_SHORT_PRESS_VALUE},
    {KEY_B_LONG_PRESS,      BUTTON_B_LONG_PRESS_VALUE},
    {KEY_B_SHORT_PRESS,     BUTTON_B_SHORT_PRESS_VALUE},
    {KEY_M_LONG_PRESS,      BUTTON_M_LONG_PRESS_VALUE},
    {KEY_M_SHORT_PRESS,     BUTTON_M_SHORT_PRESS_VALUE},
    {KEY_A_B_LONG_PRESS,    BUTTON_A_B_LONG_PRESS_VALUE},
    {KEY_A_B_SHORT_PRESS,   BUTTON_A_B_SHORT_PRESS_VALUE},
    {KEY_A_M_LONG_PRESS,    BUTTON_A_M_LONG_PRESS_VALUE},
    {KEY_A_M_SHORT_PRESS,   BUTTON_A_M_SHORT_PRESS_VALUE},
    {KEY_B_M_LONG_PRESS,    BUTTON_B_M_LONG_PRESS_VALUE},
    {KEY_B_M_SHORT_PRESS,   BUTTON_B_M_SHORT_PRESS_VALUE},
    {KEY_A_B_M_LONG_PRESS,  BUTTON_A_B_M_LONG_PRESS_VALUE},
    {KEY_A_B_M_SHORT_PRESS, BUTTON_A_B_M_SHORT_PRESS_VALUE},
    {REPORT_MAX,            0},
};

static button_event_t button_event[] = {
    {KEY_A_UP,      BUTTON_A, BUTTON_UP,    (0x01<<BUTTON_A),   (0x10<<BUTTON_A)},
    {KEY_A_DOWN,    BUTTON_A, BUTTON_DOWN,  (0x01<<BUTTON_A),   (0x10<<BUTTON_A)},
    {KEY_B_UP,      BUTTON_B, BUTTON_UP,    (0x01<<BUTTON_B),   (0x10<<BUTTON_B)},
    {KEY_B_DOWN,    BUTTON_B, BUTTON_DOWN,  (0x01<<BUTTON_B),   (0x10<<BUTTON_B)},
    {KEY_M_UP,      BUTTON_M, BUTTON_UP,    (0x01<<BUTTON_M),   (0x10<<BUTTON_M)},
    {KEY_M_DOWN,    BUTTON_M, BUTTON_DOWN,  (0x01<<BUTTON_M),   (0x10<<BUTTON_M)},
    {0xFF, 0, 0,  0, 0},
};

REPORT_E button_event_handler(u8 button_press_state);
s16 button_cb_handler(void *args);

REPORT_E button_event_handler(u8 button_press_state)
{
    u8 i = 0;
    u8 comb_state_value = 0;
    REPORT_E comb_report_value = REPORT_MAX;
    
    if(button_press_state & BUTTON_LONG_PRESS_MASK)
    {
        comb_state_value = (button_press_state|(button_press_state<<4))&0xF0;
    }
    else if(button_press_state & BUTTON_SHORT_PRESS_MASK)
    {
        comb_state_value = (button_press_state|(button_press_state>>4))&0x0F;
    }
    
    while(button_state[i].state_value != STATE_MAX)
    {
        if(button_state[i].state_value == comb_state_value)
        {
            comb_report_value = button_state[i].report_value;
            break;
        }
        i++;
    }

    return comb_report_value;
}

s16 button_cb_handler(void *args)
{
    u8 i = 0;
    u8 release = 0;
    REPORT_E report_value = REPORT_MAX;
    static button_t button = {.press_state=0, .press_down=0};
    EVENT_E ev = (EVENT_E)args;

    while(button_event[i].ev != 0xFF)
    {
        if(button_event[i].ev == ev)
        {
            break;
        }
        i++;
    }

    if(button_event[i].ev != 0xFF)
    {
        if(button_event[i].sta == BUTTON_DOWN)
        {
            button.down_time[button_event[i].idx] = TimeGet32()/SECOND;
            button.press_state &= ~button_event[i].short_press_mask;
            button.press_state &= ~button_event[i].long_press_mask;
            button.press_down |= (1<<button_event[i].idx);
            release = 0;
        }
        else if(button_event[i].sta == BUTTON_UP)
        {
        	button.up_time[button_event[i].idx] = TimeGet32()/SECOND;
        	u8 button_interval = 
        		(button.up_time[button_event[i].idx]>button.down_time[button_event[i].idx])?
        		(button.up_time[button_event[i].idx]-button.down_time[button_event[i].idx]):
        		(button.down_time[button_event[i].idx]>button.up_time[button_event[i].idx]);

        	if(button_interval >= BUTTON_LONG_PRESS_INTERVAL) /* button long press */
        	{
                button.press_state |= button_event[i].long_press_mask;
        	}
        	else  /* button short press */
        	{
                button.press_state |= button_event[i].short_press_mask;
        	}
        	button.press_down &= ~(1<<button_event[i].idx);
            release = 1;
        }
    }

    if((release == 1) && (button.press_down == 0))
    {
        report_value = button_event_handler(button.press_state);
        button.press_state = 0;
    }
    
    return (s16)report_value;
}

