#include <debug.h>          /* Simple host interface to the UART driver */
#include "../adapter.h"

s16 step_count_init(void);

static void step_count_algo(void)
{
    s16 xyz = 0;
    
    //xyz = get_driver()->gsensor->gsensor_read();
    printf("cnt = %d\r\n", xyz);
}

static void step_count_cb_handler(u16 id)
{
	get_driver()->timer->timer_start(1000, step_count_cb_handler);
	step_count_algo();
}

s16 step_count_init(void)
{
	get_driver()->timer->timer_start(1000, step_count_cb_handler);
	return 0;
}

