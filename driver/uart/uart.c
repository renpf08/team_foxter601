#include <gatt.h>
#include <timer.h>
#include <panic.h>
#include <mem.h>
#include <uart.h>
#include <pio.h>            /* Programmable I/O configuration and control */
#include <timer.h>          /* Chip timer functions */
#include "user_config.h"
#include "../driver.h"

#define USE_UART_BLOCK_MODE 0

#define BIT_RATE_9600   99
#define BIT_RATE_115200 1
#define BIT_RATE        BIT_RATE_115200
#define TIMEOUT 33

#define UART_TX_HIGH(num) PioSet((num), 1UL)
#define UART_TX_LOW(num)  PioSet((num), 0UL)

typedef enum {
	UART_IDLE,
	UART_START,
	UART_TRANSFERRING,
	UART_PARITY,
	UART_STOP,
}UART_STATE_E;

/// parity mode
typedef enum {
    GPIO_UART_PARITY_MODE_NONE = 0,
    GPIO_UART_PARITY_MODE_ODD,
    GPIO_UART_PARITY_MODE_EVEN
}UART_PARITY_MODE;
    
#define QUEUE_MAX   64
u8 ring_buffer[64];

typedef struct {
	pin_t rx;
	pin_t tx;
	volatile u8 data_bit;
	u8 stop;
	u8 parity;
	volatile u8 bit_send_index;
	u16 bitrate;
	volatile UART_STATE_E state;
	volatile u8 *buf_ptr;
	volatile u8 size;
	u8 end_flag;
    volatile u8 head;
    volatile u8 tail;
    volatile bool begin;
}uart_config_t;

static uart_config_t uart_config = {
	.rx = {
			.group = 0,
			.num = 0,
		},
	.tx = {
			.group = 0,
			.num = 0,
		},
	.data_bit = 8,
	.stop = 1,
	.parity = 0,
	.bit_send_index = 0,
	.bitrate = BIT_RATE,
	.state = UART_IDLE,
	.buf_ptr = ring_buffer,
	.size = 0,
	.end_flag = 0,
	.head = 0,
	.tail = 0,
	.begin = FALSE,
};

void uart_byte_enqueue(u8 byte);
bool uart_byte_dequeue(void);
static timer_id csr_uart_timer_create(uint32 timeout, timer_callback_arg handler);
static void uart_timer_cb(u16 id);

void uart_byte_enqueue(u8 byte)
{
    /** queue is full, discard the oldest byte */
    if(uart_config.head == ((uart_config.tail+1)%QUEUE_MAX))
    {
        uart_config.head = (uart_config.head+1)%QUEUE_MAX;
    }
    
    uart_config.buf_ptr[uart_config.tail] = byte;
    uart_config.tail = (uart_config.tail+1)%QUEUE_MAX;

    //uart_config.size = (uart_config.tail - uart_config.head);

    if(uart_config.begin == FALSE)
    {
    	//timer start and regitster callback
    	csr_uart_timer_create(TIMEOUT, uart_timer_cb);
        uart_config.begin = TRUE;
    }
}

bool uart_byte_dequeue(void)
{
    uart_config.head = (uart_config.head+1)%QUEUE_MAX;
    //uart_config.size = (uart_config.tail - uart_config.head);
    
    /** queue is empty */
    if(uart_config.head == uart_config.tail)
    {
        //uart_config.size = 0;
        return FALSE;
    }
    
    return TRUE;
}

static int uart_send_handler(void)
{
	u8 bit_send = 0;

    switch(uart_config.state) {
		case UART_IDLE:
			uart_config.state = UART_START;
			break;
	    case UART_START:
			//Put GPIO to logic 0 to indicate the start
			UART_TX_LOW(uart_config.tx.num);
            #ifdef RELEASE_MODE
            TimeDelayUSec(10);
            #endif
			uart_config.bit_send_index = 0;
			uart_config.state = UART_TRANSFERRING;
			break;
		case UART_TRANSFERRING:
			bit_send = (uart_config.buf_ptr[uart_config.head] >> uart_config.bit_send_index) & 0x1;
			//Put GPIO to the logic level according to the bit value. 0 for low, and 1 for high;
			if(bit_send) {
				UART_TX_HIGH(uart_config.tx.num);
			} else {
				UART_TX_LOW(uart_config.tx.num);
			}

			uart_config.bit_send_index++;
			if(uart_config.bit_send_index == 8) {
				uart_config.state = UART_STOP;
			}
			break;
		case UART_PARITY:
			break;
		case UART_STOP:
			//Put GPIO to logic 1 to stop the transferring
			UART_TX_HIGH(uart_config.tx.num);
            #ifdef RELEASE_MODE
            TimeDelayUSec(10);
            #endif
        	if(uart_byte_dequeue() == FALSE)
        	{
                uart_config.state = UART_IDLE;
                uart_config.begin = FALSE;
                return 1;
        	}
            else
            {
                uart_config.state = UART_START;
            }
			break;
		default :
			break;
    }
	return 0;
}

static timer_id csr_uart_timer_create(uint32 timeout, timer_callback_arg handler)
{
    const timer_id tId = TimerCreate(timeout, TRUE, handler);
    
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID)
    {
        //DebugWriteString("\r\nFailed to start timer");
        /* Panic with panic code 0xfe */
        Panic(0xfe);
    }
	return tId;
}

static void uart_timer_cb(u16 id)
{
	u8 done = 0;
	done = uart_send_handler();
	if(1 != done) {
		csr_uart_timer_create(TIMEOUT, uart_timer_cb);
	}
}

static s16 csr_uart_read(void *args)
{
	return 0;
}

static s16 csr_uart_write(u8 *buf, u16 num)
{
	//uart state change to START
	uart_config.buf_ptr = buf;
	uart_config.size = num;
	uart_config.state = UART_IDLE;
	uart_config.bit_send_index = 0;

    for(num = 0; num < uart_config.size; num++)
    {
        uart_byte_enqueue(buf[num]);
    }
    
	return 0;
}

static s16 csr_uart_init(cfg_t *args, event_callback cb)
{
	uart_config.tx.group = args->uart_cfg.tx.group;
	uart_config.tx.num = args->uart_cfg.tx.num;
	uart_config.rx.group = args->uart_cfg.rx.group;
	uart_config.rx.num = args->uart_cfg.rx.num;

	//tx gpio init and set to high
    PioSetModes(1UL << uart_config.tx.num, pio_mode_user);
	PioSetDir(uart_config.tx.num, PIO_DIR_OUTPUT);
	PioSetPullModes(1UL << uart_config.tx.num, pio_mode_strong_pull_up);
	UART_TX_HIGH(uart_config.tx.num);
	uart_config.end_flag = 0;
	return 0;
}

static s16 csr_uart_uninit(void)
{
	UART_TX_HIGH(uart_config.tx.num);
	uart_config.tx.group = 0;
	uart_config.tx.num = 0;
	uart_config.rx.group = 0;
	uart_config.rx.num = 0;
	return 0;
}

uart_t csr_uart = {
	.uart_init = csr_uart_init,
	.uart_read = csr_uart_read,
	.uart_write = csr_uart_write,
	.uart_uninit = csr_uart_uninit,
};
