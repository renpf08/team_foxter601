#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../adapter.h"

s16 button_A_down_cb_handler(void *args);
s16 button_A_up_cb_handler(void *args);

typedef struct {
	u8 down_time;
	u8 up_time;

}bubtton_A_t;
bubtton_A_t button_a_state;

s16 button_A_down_cb_handler(void *args)
{
	button_state_t* button_state = (button_state_t*)args;
	button_a_state.down_time = clock_get()->second;

	button_state->press_state &= ~(KEY_A_SHORT_PRESS_POS);
	button_state->press_state &= ~(KEY_A_LONG_PRESS_POS);
	button_state->press_down |= (1<<KEY_A_POSITION);

	return 0;
}

s16 button_A_up_cb_handler(void *args)
{
	button_state_t* button_state = (button_state_t*)args;
	button_a_state.up_time = clock_get()->second;

	u8 button_interval = 
		(button_a_state.up_time>button_a_state.down_time)?
		(button_a_state.up_time-button_a_state.down_time):
		(button_a_state.down_time>button_a_state.up_time);

	if(button_interval >= KEY_LONG_PRESS_INTERVAL)
	{
		button_state->press_state &= ~(KEY_A_SHORT_PRESS_POS);
		button_state->press_state |= KEY_A_LONG_PRESS_POS;
	}
	else
	{
		button_state->press_state |= KEY_A_SHORT_PRESS_POS;
		button_state->press_state &= ~(KEY_A_LONG_PRESS_POS);
	}
	button_state->press_down &= ~(1<<KEY_A_POSITION);

	return 0;
}
