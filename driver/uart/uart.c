#include "user_config.h"
#if USE_UART_PRINT
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
#define BIT_RATE_115200     2
#define BIT_RATE_9600       33
#define BIT_RATE            BIT_RATE_115200

#define UART_TIMER_DELAY    33

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
    
#define QUEUE_MAX   QUEUE_BUFFER

typedef struct {
	pin_t rx;
	pin_t tx;
	volatile u8 data_bit;
	u8 stop;
	u8 parity;
	volatile u8 bit_send_index;
	volatile u16 bitrate;
	volatile UART_STATE_E state;
	volatile u8 size;
	u8 end_flag;
	volatile u8 ring_buffer[QUEUE_MAX];
    u16 ring_buffer_size;
    volatile u16 ring_buffer_head;
    volatile u16 ring_buffer_tail;
    volatile bool ring_buffer_poll;
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
	.ring_buffer[0] = 0,
	.ring_buffer_size = QUEUE_MAX,
	.size = 0,
	.end_flag = 0,
	.ring_buffer_head = 0,
	.ring_buffer_tail = 0,
	.ring_buffer_poll = FALSE,
};

void uart_byte_enqueue(u8 byte);
bool uart_byte_dequeue(void);
static timer_id csr_uart_timer_create(uint32 timeout, timer_callback_arg handler);
static void uart_timer_cb(u16 id);

void uart_byte_enqueue(u8 byte)
{
    /** queue is full, discard the oldest byte */
    if(uart_config.ring_buffer_head == ((uart_config.ring_buffer_tail+1)%uart_config.ring_buffer_size)) {
        if(uart_config.bit_send_index != 0) { // discard only when there is no byte being processed
            return;
        }
        uart_config.ring_buffer_head = (uart_config.ring_buffer_head+1)%uart_config.ring_buffer_size;
    }
    
    uart_config.ring_buffer[uart_config.ring_buffer_tail] = byte;
    uart_config.ring_buffer_tail = (uart_config.ring_buffer_tail+1)%uart_config.ring_buffer_size;

    if(uart_config.ring_buffer_poll == FALSE) {
    	csr_uart_timer_create(UART_TIMER_DELAY, uart_timer_cb);
        uart_config.ring_buffer_poll = TRUE;
    }
}

bool uart_byte_dequeue(void)
{
    uart_config.ring_buffer_head = (uart_config.ring_buffer_head+1)%uart_config.ring_buffer_size;
    
    /** queue is empty */
    if(uart_config.ring_buffer_head == uart_config.ring_buffer_tail) {
        return FALSE;
    }
    
    return TRUE;
}

#if USE_UART_BLOCK_MODE
static int uart_send_handler(uint32 *timeout)
{
	u8 bit_send = 0;
    u8 done = 0;
    u8 bit_wait = 0;
    u8 send_en = 1;

    UART_TX_LOW(uart_config.tx.num);
    //TimeDelayUSec((u16)*timeout);
    
    uart_config.bit_send_index = 0;
    while(uart_config.bit_send_index < uart_config.data_bit) {
        if(send_en == 1) {
            bit_send = (uart_config.ring_buffer[uart_config.ring_buffer_head] >> uart_config.bit_send_index) & 0x01;
            if(bit_send)
                UART_TX_HIGH(uart_config.tx.num);
            else
                UART_TX_LOW(uart_config.tx.num);
        }
        send_en = 0;
        if(bit_wait++ >= uart_config.bitrate) {
            bit_wait = 0;
            send_en = 1;
            uart_config.bit_send_index++;
        }
    }
    uart_config.bit_send_index = 0;

    UART_TX_HIGH(uart_config.tx.num);
    //TimeDelayUSec((u16)*timeout);

    if(uart_byte_dequeue() == FALSE) {
        done = 1;
    }

    return done;
}
#else
static int uart_send_handler(uint32 *timeout)
{
	u8 bit_send = 0;

    switch(uart_config.state) {
		case UART_IDLE:
			uart_config.state = UART_START;
			break;
	    case UART_START:
			//Put GPIO to logic 0 to indicate the start
			UART_TX_LOW(uart_config.tx.num);
			uart_config.bit_send_index = 0;
			uart_config.state = UART_TRANSFERRING;
			break;
		case UART_TRANSFERRING:
			bit_send = (uart_config.ring_buffer[uart_config.ring_buffer_head] >> uart_config.bit_send_index) & 0x1;
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
        	if(uart_byte_dequeue() == FALSE) {
                uart_config.state = UART_IDLE;
                return 1;
        	} else {
                uart_config.state = UART_START;
            }
			break;
		default :
			break;
    }
	return 0;
}
#endif

static timer_id csr_uart_timer_create(uint32 timeout, timer_callback_arg handler)
{
    const timer_id tId = TimerCreate(timeout, TRUE, handler);
    
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID) {
        //DebugWriteString("\r\nFailed to start timer");
        /* Panic with panic code 0xfe */
        //Panic(0xfe);        
		ReportPanic(__FILE__, __func__, __LINE__, app_timer_create_fail);
    }
	return tId;
}

static void uart_timer_cb(u16 id)
{
	u8 done = 0;
    uint32 timeout = UART_TIMER_DELAY;
    
    done = uart_send_handler(&timeout);
	if(1 != done) {
		csr_uart_timer_create(timeout, uart_timer_cb);
 	} else {
        uart_config.ring_buffer_poll = FALSE;
    }
}

static s16 csr_uart_read(void *args)
{
	return 0;
}

static s16 csr_uart_write(u8 *buf, u16 num)
{
    for(uart_config.size = 0; uart_config.size < num; uart_config.size++) {
        uart_byte_enqueue(buf[uart_config.size]);
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

uart_t csr_uart = {
	.uart_init = csr_uart_init,
	.uart_read = csr_uart_read,
	.uart_write = csr_uart_write,
};
#endif //USE_UART_PRINT