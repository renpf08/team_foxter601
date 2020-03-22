#ifndef COMMON_H
#define COMMON_H

#include "typedef.h"

#define PIO_DIR_OUTPUT  TRUE        /* PIO direction configured as output */
#define PIO_DIR_INPUT   FALSE       /* PIO direction configured as input */

#define BIT_MASK(num) (0x01 << (num))

typedef struct {
	u8 x_l;
	u8 x_h;
	u8 y_l;
	u8 y_h;
	u8 z_l;
	u8 z_h;
}gsensor_data_t;

typedef enum {
	KEY_A_UNKNOWN,
	KEY_A_UP,
	KEY_A_DOWN,
	KEY_B_UNKNOWN,
	KEY_B_UP,
	KEY_B_DOWN,
	KEY_M_UNKNOWN,
	KEY_M_UP,
	KEY_M_DOWN
}EVENT_E;

#if 0
typedef enum
{
    BUTTON_STATE_DOWN,       /* Button was pressed */
    BUTTON_STATE_UP,         /* Button was released */
    BUTTON_STATE_UNKNOWN,    /* Button state is unknown */
} BUTTON_STATE_E;
#endif

typedef struct {
	u8 group;
	u8 num;
}pin_t;

typedef struct {
	pin_t tx;
	pin_t rx;
}uart_cfg_t;

typedef struct {
	pin_t clk;
	pin_t mosi;
	pin_t miso;
	pin_t cs;
	pin_t int1;
	pin_t int2;
}gsensor_cfg_t;

typedef struct {
	pin_t scl;
	pin_t sda;
	pin_t int;
}magnetometer_cfg_t;

typedef struct {
	uart_cfg_t uart_cfg;
	pin_t      keya_cfg;
	pin_t      vibrator_cfg;
	gsensor_cfg_t gsensor_cfg;
	magnetometer_cfg_t magnetometer_cfg;
}cfg_t;

typedef s16 (*event_callback)(EVENT_E ev);

typedef s16 (*init)(cfg_t *args, event_callback cb);
typedef s16 (*uninit)(void);
typedef s16 (*read)(void *args);
typedef s16 (*write)(u8 *buf, u16 num);

typedef s16 (*on)(void *args);
typedef s16 (*off)(void *args);

typedef s16 (*fread)(u16 *buffer, u16 length, u16 offset);
typedef s16 (*fwrite)(u16 *buffer, u16 length, u16 offset);

typedef void(*timer_cb)(u16 id);
typedef s16 (*timer_start_func)(u16 ms, timer_cb cb);

#endif
