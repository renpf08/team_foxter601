#include <debug.h>          /* Simple host interface to the UART driver */
#include <pio.h>            /* Programmable I/O configuration and control */
#include "../adapter.h"

s16 button_B_down_cb_handler(void *args);
s16 button_B_up_cb_handler(void *args);

typedef struct {
	u8 down_time;
	u8 up_time;

}bubtton_B_t;
bubtton_B_t button_b_state;

s16 button_B_down_cb_handler(void *args)
{
	button_state_t* button_state = (button_state_t*)args;
	button_b_state.down_time = clock_get()->second;

	button_state->press_state &= ~(KEY_B_SHORT_PRESS_POS);
	button_state->press_state &= ~(KEY_B_LONG_PRESS_POS);
	button_state->press_down |= (1<<KEY_B_POSITION);

	return 0;
}

s16 button_B_up_cb_handler(void *args)
{
	button_state_t* button_state = (button_state_t*)args;
	button_b_state.up_time = clock_get()->second;

	u8 button_interval = 
		(button_b_state.up_time>button_b_state.down_time)?
		(button_b_state.up_time-button_b_state.down_time):
		(button_b_state.down_time>button_b_state.up_time);

	if(button_interval >= KEY_LONG_PRESS_INTERVAL)
	{
		button_state->press_state &= ~(KEY_B_SHORT_PRESS_POS);
		button_state->press_state |= KEY_B_LONG_PRESS_POS;
	}
	else
	{
		button_state->press_state |= KEY_B_SHORT_PRESS_POS;
		button_state->press_state &= ~(KEY_B_LONG_PRESS_POS);
	}
	button_state->press_down &= ~(1<<KEY_B_POSITION);

	return 0;
}
