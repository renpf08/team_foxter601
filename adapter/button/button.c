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
}BUTTON_SINGLE_EVENT_E;

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
    BUTTON_STATE_MAX                = 0xFF
}BUTTON_COMBO_EVENT_E;

typedef struct {
	TIME down_time[BUTTON_NUM];
	TIME up_time[BUTTON_NUM];
	u8 combo_event_flag; /* high half-byte to indicate long press event, and low half-byte to short press event */
	u8 press_down_flag;
}button_t;

typedef struct {
    REPORT_E report_value;
    BUTTON_COMBO_EVENT_E combo_event;
}button_combo_event_t;

typedef struct {
    EVENT_E trigger_event;
    BUTTON_INDEX_E button_index;
    BUTTON_SINGLE_EVENT_E single_event;
    u8 short_press_mask;
    u8 long_press_mask;
}button_single_event_t;

static button_combo_event_t button_combo_event[] = {
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
    {REPORT_MAX,            BUTTON_STATE_MAX},
};

static button_single_event_t button_single_event[] = {
    {KEY_A_UP,      BUTTON_A, BUTTON_UP,    (0x01<<BUTTON_A),   (0x10<<BUTTON_A)},
    {KEY_A_DOWN,    BUTTON_A, BUTTON_DOWN,  (0x01<<BUTTON_A),   (0x10<<BUTTON_A)},
    {KEY_B_UP,      BUTTON_B, BUTTON_UP,    (0x01<<BUTTON_B),   (0x10<<BUTTON_B)},
    {KEY_B_DOWN,    BUTTON_B, BUTTON_DOWN,  (0x01<<BUTTON_B),   (0x10<<BUTTON_B)},
    {KEY_M_UP,      BUTTON_M, BUTTON_UP,    (0x01<<BUTTON_M),   (0x10<<BUTTON_M)},
    {KEY_M_DOWN,    BUTTON_M, BUTTON_DOWN,  (0x01<<BUTTON_M),   (0x10<<BUTTON_M)},
    {0xFF, 0, 0,  0, 0},
};

REPORT_E button_combo_event_handler(u8 combo_evt_flag);
s16 button_cb_handler(void *args);

REPORT_E button_combo_event_handler(u8 combo_evt_flag)
{
    u8 i = 0;
    u8 comb_state_value = 0;
    REPORT_E combo_event_report_value = REPORT_MAX;
    
    if(combo_evt_flag & BUTTON_LONG_PRESS_MASK)
    {
        comb_state_value = (combo_evt_flag|(combo_evt_flag<<4))&0xF0;
    }
    else if(combo_evt_flag & BUTTON_SHORT_PRESS_MASK)
    {
        comb_state_value = (combo_evt_flag|(combo_evt_flag>>4))&0x0F;
    }
    
    while(button_combo_event[i].combo_event < BUTTON_STATE_MAX)
    {
        if(button_combo_event[i].combo_event == comb_state_value)
        {
            combo_event_report_value = button_combo_event[i].report_value;
            break;
        }
        i++;
    }

    return combo_event_report_value;
}

s16 button_cb_handler(void *args)
{
    u8 i = 0;
    u8 release = 0;
    REPORT_E combo_event_report_value = REPORT_MAX;
    static button_t button = {.combo_event_flag=0, .press_down_flag=0};
    EVENT_E trig_evt = (EVENT_E)args;

    while(button_single_event[i].trigger_event != 0xFF)
    {
        if(button_single_event[i].trigger_event == trig_evt)
        {
            break;
        }
        i++;
    }

    if(button_single_event[i].trigger_event != 0xFF)
    {
        if(button_single_event[i].single_event == BUTTON_DOWN)
        {
            button.down_time[button_single_event[i].button_index] = TimeGet32()/SECOND;
            button.combo_event_flag &= ~button_single_event[i].short_press_mask;
            button.combo_event_flag &= ~button_single_event[i].long_press_mask;
            button.press_down_flag |= (1<<button_single_event[i].button_index);
            release = 0;
        }
        else if(button_single_event[i].single_event == BUTTON_UP)
        {
        	button.up_time[button_single_event[i].button_index] = TimeGet32()/SECOND;
        	u8 button_interval = 
        		(button.up_time[button_single_event[i].button_index]>button.down_time[button_single_event[i].button_index])?
        		(button.up_time[button_single_event[i].button_index]-button.down_time[button_single_event[i].button_index]):
        		(button.down_time[button_single_event[i].button_index]>button.up_time[button_single_event[i].button_index]);

        	if(button_interval >= BUTTON_LONG_PRESS_INTERVAL) /* button long press */
        	{
                button.combo_event_flag |= button_single_event[i].long_press_mask;
        	}
        	else  /* button short press */
        	{
                button.combo_event_flag |= button_single_event[i].short_press_mask;
        	}
        	button.press_down_flag &= ~(1<<button_single_event[i].button_index);
            release = 1;
        }
    }

    if((release == 1) && (button.press_down_flag == 0))
    {
        combo_event_report_value = button_combo_event_handler(button.combo_event_flag);
        button.combo_event_flag = 0;
    }
    
    return (s16)combo_event_report_value;
}

