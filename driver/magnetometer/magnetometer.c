#include <types.h>
#include <pio.h>
#include <pio_ctrlr.h>
#include <spi.h>
#include <i2c.h>
#include <timer.h>
#include "../driver.h"
#include "magnetometer.h"

#define SDA_MODE_OUT(sda_pin)   PioSetDir(sda_pin, PIO_DIR_OUTPUT)
#define SDA_MODE_IN(sda_pin)   	PioSetDir(sda_pin, PIO_DIR_INPUT)
#define SDA_HIGH(sda_pin)     	PioSet(sda_pin, 1UL)
#define SDA_LOW(sda_pin)		PioSet(sda_pin, 0UL)
#define SDA_GET(sda_pin)      	PioGet(sda_pin)

#define SCL_MODE_OUT(scl_pin)   PioSetDir(scl_pin, PIO_DIR_OUTPUT)
#define SCL_MODE_IN(scl_pin)   	PioSetDir(scl_pin, PIO_DIR_INPUT)
#define SCL_HIGH(scl_pin)     	PioSet(scl_pin, 1UL)
#define SCL_LOW(scl_pin)		PioSet(scl_pin, 0UL)

#define INT_GET(int_pin)        PioGet(int_pin)

#define I2C_DELAY_US(us)		TimeDelayUSec(us)

typedef struct {
	magnetometer_cfg_t cfg;
	event_callback magnetometer_cb;
}csr_magnetometer_t;

static csr_magnetometer_t csr_mag3110 = {
	.cfg = {
		.scl = {
			.group = 0,
			.num = 0,
		},
		.sda = {
			.group = 0,
			.num = 0,
		},
		.int1 = {
			.group = 0,
			.num = 0,
		},
	},
	.magnetometer_cb = NULL,
};

//i2c protocol
static void i2c_start(void)
{
    SDA_MODE_OUT(csr_mag3110.cfg.sda.num);
	SDA_HIGH(csr_mag3110.cfg.sda.num);
    I2C_DELAY_US(2);

	SCL_HIGH(csr_mag3110.cfg.scl.num);
    I2C_DELAY_US(2);

	SDA_LOW(csr_mag3110.cfg.sda.num);
    I2C_DELAY_US(4); 
}

static void write_byte(u8 data)
{
	u8 cnt = 0;
	u8 temp = data;

	for(cnt = 0; cnt < 8; cnt++) {
		SCL_LOW(csr_mag3110.cfg.scl.num);
		I2C_DELAY_US(2);
	
		SDA_MODE_OUT(csr_mag3110.cfg.sda.num);
		if(temp & 0x80) {
			SDA_HIGH(csr_mag3110.cfg.sda.num);
		} else {
			SDA_LOW(csr_mag3110.cfg.sda.num);
		}
		I2C_DELAY_US(2);

		SCL_HIGH(csr_mag3110.cfg.scl.num);
		I2C_DELAY_US(2);

		temp <<= 1;
	}
	SCL_LOW(csr_mag3110.cfg.scl.num);
}

static bool check_ack(void)
{
	bool temp;

	SCL_LOW(csr_mag3110.cfg.scl.num);
	SDA_MODE_IN(csr_mag3110.cfg.sda.num);
	SDA_HIGH(csr_mag3110.cfg.sda.num);

	I2C_DELAY_US(2);
	SCL_HIGH(csr_mag3110.cfg.scl.num);

	temp = SDA_GET(csr_mag3110.cfg.sda.num);
	I2C_DELAY_US(2);

	SCL_LOW(csr_mag3110.cfg.scl.num);
	I2C_DELAY_US(2);
	return temp;
}

static void i2c_stop(void) 
{ 
	SCL_LOW(csr_mag3110.cfg.scl.num);
	SDA_MODE_OUT(csr_mag3110.cfg.sda.num);
	SDA_LOW(csr_mag3110.cfg.sda.num);
    I2C_DELAY_US(2);

	SCL_HIGH(csr_mag3110.cfg.scl.num);
    I2C_DELAY_US(2);

	SDA_HIGH(csr_mag3110.cfg.sda.num);
    I2C_DELAY_US(4); 
} 

static u8 read_byte(void)
{
	bool bit = false;
	u8 temp = 0;
	u8 cnt = 0;

	for(cnt = 0; cnt < 8; cnt++) {
		temp <<= 1;
		SCL_LOW(csr_mag3110.cfg.scl.num);
		SDA_MODE_IN(csr_mag3110.cfg.sda.num);
		I2C_DELAY_US(2);

		SCL_HIGH(csr_mag3110.cfg.scl.num);
		I2C_DELAY_US(2);

		bit = SDA_GET(csr_mag3110.cfg.sda.num);

		if(true == bit) {
			temp |= 0x01;
		}else {
			temp &= 0xfe;
		}
	}
	SCL_LOW(csr_mag3110.cfg.scl.num);
	return temp;
}

static void send_ack(void) 
{
	SCL_LOW(csr_mag3110.cfg.scl.num);
	SDA_MODE_OUT(csr_mag3110.cfg.sda.num);
	SDA_LOW(csr_mag3110.cfg.sda.num);
    I2C_DELAY_US(2);
	
	SCL_HIGH(csr_mag3110.cfg.scl.num);
	I2C_DELAY_US(2);

	SCL_LOW(csr_mag3110.cfg.scl.num);
	I2C_DELAY_US(2);
} 

static void send_noack(void)
{
	SCL_LOW(csr_mag3110.cfg.scl.num);
	SDA_MODE_OUT(csr_mag3110.cfg.sda.num);
	SDA_HIGH(csr_mag3110.cfg.sda.num);
    I2C_DELAY_US(2);

	SCL_HIGH(csr_mag3110.cfg.scl.num);
	I2C_DELAY_US(2);

	SCL_LOW(csr_mag3110.cfg.scl.num);
	I2C_DELAY_US(2);
}

static void csr_magnetometer_start_i2c(void)
{
	//SDA SCL CONFIGURATION
	PioSetModes(BIT_MASK(csr_mag3110.cfg.sda.num)|BIT_MASK(csr_mag3110.cfg.scl.num), pio_mode_user);	
	PioSetDir(csr_mag3110.cfg.sda.num, TRUE);	
	PioSetDir(csr_mag3110.cfg.scl.num, TRUE);
	PioSetPullModes(BIT_MASK(csr_mag3110.cfg.sda.num) | BIT_MASK(csr_mag3110.cfg.scl.num), pio_mode_strong_pull_up);
	SDA_HIGH(csr_mag3110.cfg.sda.num);
	SCL_HIGH(csr_mag3110.cfg.scl.num);
}

#if 0
static void csr_magnetometer_stop_i2c(void)
{
	//SDA CONFIGURATION
	PioSetModes(BIT_MASK(csr_mag3110.cfg.sda.num)|BIT_MASK(csr_mag3110.cfg.scl.num), pio_mode_user);	
	PioSetDir(csr_mag3110.cfg.sda.num, TRUE);
	PioSetDir(csr_mag3110.cfg.scl.num, TRUE);
	PioSetPullModes(BIT_MASK(csr_mag3110.cfg.sda.num) | BIT_MASK(csr_mag3110.cfg.scl.num), pio_mode_no_pulls);
	SDA_HIGH(csr_mag3110.cfg.sda.num);
	SCL_HIGH(csr_mag3110.cfg.scl.num);
}
#endif

static u8 csr_magnetometer_reg_read(u8 reg, u8 *buf, u8 num)
{
    u8 i = 0;
	
	csr_magnetometer_start_i2c();
    for(i = 0; i < 3; i++) {
        i2c_start();
        write_byte(MAG3110_IIC_ADDRESS_WRITE);
        if(check_ack()) {
			i2c_stop();
            if(i==2) {
                return 0x01;
            }
        } else {/*ACK*/
            break;
        }
    }

	write_byte(reg);
    if(check_ack()) {
        i2c_stop();
        return 0x01;
    }
    
    i2c_start();
    write_byte(MAG3110_IIC_ADDRESS_READ);
    if(check_ack()) {
       i2c_stop();
       return 0x01;
    }
    
    for(i = 0; i < num; i++) {
        buf[i] = read_byte();
        if(i < (num - 1)) {
			send_ack();
        }else {
			send_noack();
        }
    }
	
	i2c_stop();
	return 0x00; 
}/* MAG3110_I2c_Read */

static u8 csr_magnetometer_reg_write(u8 reg, u8 *buf, u8 num)
{
    u8 i = 0;
	u8 ack_flag = 0;
	
	csr_magnetometer_start_i2c();
    for(i = 0; i < 3; i++) {
        i2c_start();
        write_byte(MAG3110_IIC_ADDRESS_WRITE);
        if(check_ack()) {/*NoACK*/
            i2c_stop();
            if(i==2) {
                return 0x01;
            }
        }else {/*ACK*/
            break;
        }
    }
	
    write_byte(reg);
    if(check_ack()) {/*NoACK*/
        i2c_stop();
        return 0x01;
    }
	
    for(i = 0; i < num; i++) {
        write_byte(buf[i]);
        if(check_ack()) {
           ack_flag = 1; 
        }
    }
	
	i2c_stop();
	return ack_flag;
} /* MAG3110_I2c_Write */

static s16 csr_magnetometer_to_idle(void)
{
	u8 temp = 0xA0;
	/*TempVal=0xA4;  OutPut Rate 2.5HZ, Over Sample Ratio 16,Bit2-Fast Read 8bit,Bit1-Normal Opterate, Bit0-Standby*/
	csr_magnetometer_reg_write(CTRL_REG1, &temp, 1);

	SDA_MODE_IN(csr_mag3110.cfg.sda.num);
	SCL_MODE_IN(csr_mag3110.cfg.scl.num);
	PioSetPullModes(BIT_MASK(csr_mag3110.cfg.sda.num) | BIT_MASK(csr_mag3110.cfg.scl.num), pio_mode_no_pulls);	
	return 0;
}

static s16 csr_magnetometer_to_active(void)
{
	u8 temp = 0xA1;
	/*TempVal=0xA5;  OutPut Rate 2.5HZ, Over Sample Ratio 16,Bit2-Fast Read 8bit,Bit1-Normal Opterate, Bit0-Standby*/
	csr_magnetometer_reg_write(CTRL_REG1, &temp, 1);
	return 0;
}

static s16 csr_magnetometer_read(void *args)
{
	mag_data_t *data = (mag_data_t *)args;
    u8 res = 0;
	
	//csr_magnetometer_to_active();
	//while(!INT_GET(csr_mag3110.cfg.int1.num));
	res = csr_magnetometer_reg_read(OUT_X_MSB, (u8 *)data, 6);
	return (s16)res;
}

s16 csr_magnetometer_event_handler(u32 num, u32 status)
{
	if(num & (1UL << csr_mag3110.cfg.int1.num)) {
		if(NULL != csr_mag3110.magnetometer_cb) {
			csr_mag3110.magnetometer_cb(MAGNETOMETER_READY);
		}
	}
	
#if 0
	EVENT_E now_state;
	if(num & (1UL << csr_keya_cfg.pin.num)) {
		if(status & (1 << csr_keya_cfg.pin.num)) {
			now_state = KEY_A_UP;
		} else {
			now_state = KEY_A_DOWN;
		}

		if(csr_keya_cfg.last_state == now_state) {
			return 0;
		} else {
			if(NULL != csr_keya_cfg.keya_cb) {
				csr_keya_cfg.keya_cb(now_state);
			}
			csr_keya_cfg.last_state = now_state;
		}
	}
#endif	
	return 0;
}

static s16 csr_magnetometer_init(cfg_t *args, event_callback cb)
{
    uint8 temp = 0x00;

	//scl
	csr_mag3110.cfg.scl.group = args->magnetometer_cfg.scl.group;
	csr_mag3110.cfg.scl.num = args->magnetometer_cfg.scl.num;

	//sda
	csr_mag3110.cfg.sda.group = args->magnetometer_cfg.sda.group;
	csr_mag3110.cfg.sda.num = args->magnetometer_cfg.sda.num;

	//int1
	csr_mag3110.cfg.int1.group = args->magnetometer_cfg.int1.group;
	csr_mag3110.cfg.int1.num = args->magnetometer_cfg.int1.num;

	csr_mag3110.magnetometer_cb = cb;
	
	PioSetMode(csr_mag3110.cfg.int1.num, pio_mode_user);
	PioSetDir(csr_mag3110.cfg.int1.num, FALSE);
	PioSetPullModes(BIT_MASK(csr_mag3110.cfg.int1.num), pio_mode_strong_pull_up);
	PioSetEventMask(BIT_MASK(csr_mag3110.cfg.int1.num), pio_event_mode_rising);

	csr_magnetometer_start_i2c();
	csr_magnetometer_reg_read(WHO_AM_I, &temp, 1);
	if(temp != 0xC4) {//magneometer error
		return 1;
	}else {
		temp = 0x80;
		csr_magnetometer_reg_write(CTRL_REG2, &temp, 1);		
		csr_magnetometer_to_idle();
		csr_magnetometer_to_active();
		return 0;
	}
}

magnetometer_t csr_magnetometer = {
	.magnetometer_init = csr_magnetometer_init,
	.magnetometer_read = csr_magnetometer_read,
};
