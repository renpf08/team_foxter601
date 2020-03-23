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
#define SDA_HIGH(sda_pin)     	PioSets(sda_pin, 1UL)
#define SDA_LOW(sda_pin)		PioSets(sda_pin, 0UL)
#define SDA_GET(sda_pin)      	PioGet(sda_pin)

#define SCL_MODE_OUT(scl_pin)   PioSetDir(scl_pin, PIO_DIR_OUTPUT)
#define SCL_MODE_IN(scl_pin)   	PioSetDir(scl_pin, PIO_DIR_INPUT)
#define SCL_HIGH(scl_pin)     	PioSets(scl_pin, 1UL)
#define SCL_LOW(scl_pin)		PioSets(scl_pin, 0UL)

#define I2C_DELAY(us)			TimeDelayUSec(us)

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
    I2C_DELAY(2);

	SCL_HIGH(csr_mag3110.cfg.scl.num);
    I2C_DELAY(2);

	SDA_LOW(csr_mag3110.cfg.sda.num);
    I2C_DELAY(4); 
}

static void i2c_stop(void) 
{ 
	SCL_LOW(csr_mag3110.cfg.scl.num);
	SDA_MODE_OUT(csr_mag3110.cfg.sda.num);
	SDA_LOW(csr_mag3110.cfg.sda.num);
    I2C_DELAY(2);

	SCL_HIGH(csr_mag3110.cfg.scl.num);
    I2C_DELAY(2);

	SDA_HIGH(csr_mag3110.cfg.sda.num);
    I2C_DELAY(4); 
} 

static void send_ack(void) 
{
	SCL_LOW(csr_mag3110.cfg.scl.num);
	SDA_MODE_OUT(csr_mag3110.cfg.sda.num);
	SDA_LOW(csr_mag3110.cfg.sda.num);
    I2C_DELAY(2);
	
	SCL_HIGH(csr_mag3110.cfg.scl.num);
	I2C_DELAY(2);

	SCL_LOW(csr_mag3110.cfg.scl.num);
	I2C_DELAY(2);
} 

static void send_noack(void)
{
	SCL_LOW(csr_mag3110.cfg.scl.num);
	SDA_MODE_OUT(csr_mag3110.cfg.sda.num);
	SDA_HIGH(csr_mag3110.cfg.sda.num);
    I2C_DELAY(2);

	SCL_HIGH(csr_mag3110.cfg.scl.num);
	I2C_DELAY(2);

	SCL_LOW(csr_mag3110.cfg.scl.num);
	I2C_DELAY(2);
}

static bool check_ack(void)
{
	bool temp;

	SCL_LOW(csr_mag3110.cfg.scl.num);
	SDA_MODE_IN(csr_mag3110.cfg.sda.num);
	SDA_HIGH(csr_mag3110.cfg.sda.num);

	I2C_DELAY(2);
	SCL_HIGH(csr_mag3110.cfg.scl.num);

	temp = SDA_GET(csr_mag3110.cfg.sda.num);
	I2C_DELAY(2);

	SCL_LOW(csr_mag3110.cfg.scl.num);
	I2C_DELAY(2);
	return temp;
}

static void write_byte(u8 data)
{
	u8 cnt = 0;
	u8 temp = data;

	for(cnt = 0; cnt < 8; cnt++) {
		SCL_LOW(csr_mag3110.cfg.scl.num);
		I2C_DELAY(2);
	
		SDA_MODE_OUT(csr_mag3110.cfg.sda.num);
		if(temp & 0x80) {
			SDA_HIGH(csr_mag3110.cfg.sda.num);
		} else {
			SDA_LOW(csr_mag3110.cfg.sda.num);
		}
		I2C_DELAY(2);

		SCL_HIGH(csr_mag3110.cfg.scl.num);
		I2C_DELAY(2);

		temp <<= 1;
	}
	SCL_LOW(csr_mag3110.cfg.scl.num);
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
		I2C_DELAY(2);

		SCL_HIGH(csr_mag3110.cfg.scl.num);
		I2C_DELAY(2);

		bit = SDA_GET(csr_mag3110.cfg.sda.num);

		if(true == bit) {
			temp |= 0x01;
		}else {
			temp |= 0xfe;
		}
	}
	SCL_LOW(csr_mag3110.cfg.scl.num);
	return temp;
}

static void csr_magnetometer_start_i2c(void)
{
	//SDA CONFIGURATION
    PioSetMode(csr_mag3110.cfg.sda.num, pio_mode_user);
	PioSetDir(csr_mag3110.cfg.sda.num, TRUE);
	PioSetPullModes(BIT_MASK(csr_mag3110.cfg.sda.num), pio_mode_strong_pull_up);
	SDA_HIGH(csr_mag3110.cfg.sda.num);

	//SCL CONFIGURATION
    PioSetMode(csr_mag3110.cfg.scl.num, pio_mode_user);	 
	PioSetDir(csr_mag3110.cfg.scl.num, TRUE);
	PioSetPullModes(BIT_MASK(csr_mag3110.cfg.scl.num), pio_mode_strong_pull_up);
	SCL_HIGH(csr_mag3110.cfg.scl.num);
}

static void csr_magnetometer_stop_i2c(void)
{
	//SDA CONFIGURATION
    PioSetMode(csr_mag3110.cfg.sda.num, pio_mode_user);
	PioSetDir(csr_mag3110.cfg.sda.num, TRUE);
	PioSetPullModes(BIT_MASK(csr_mag3110.cfg.sda.num), pio_mode_no_pulls);
	SDA_HIGH(csr_mag3110.cfg.sda.num);

	//SCL CONFIGURATION
    PioSetMode(csr_mag3110.cfg.scl.num, pio_mode_user);	 
	PioSetDir(csr_mag3110.cfg.scl.num, TRUE);
	PioSetPullModes(BIT_MASK(csr_mag3110.cfg.scl.num), pio_mode_no_pulls);
	SCL_HIGH(csr_mag3110.cfg.scl.num);
}

static u8 csr_magnetometer_reg_write(u8 reg, u8 *buf, u8 num)
{
	u8 cnt = 0;
	//u8 ack = 0;

	csr_magnetometer_start_i2c();
	for(cnt = 0; cnt < 3; cnt++) {
		i2c_start();
		write_byte(MAG3110_IIC_ADDRESS_WRITE);
		if(no_ack == check_ack()) {
			i2c_stop();
			if(2 == cnt) {
				return 1;
			}
		}else {
			break;
		}
	}

	write_byte(reg);
	if(no_ack == check_ack()) {
		i2c_stop();
		return 1;
	}

	for(cnt = 0; cnt < num; cnt++) {
		write_byte(buf[cnt]);
		if(no_ack == check_ack()) {
			return 1;
		}
	}
	i2c_stop();
	return 0;
}

static u8 csr_magnetometer_reg_read(u8 reg, u8 *buf, u8 num)
{
	u8 cnt = 0;

	csr_magnetometer_start_i2c();
	for(cnt = 0; cnt < 3; cnt++) {
		i2c_start();
		write_byte(MAG3110_IIC_ADDRESS_WRITE);
		if(no_ack == check_ack()) {
			i2c_stop();
			if(2 == cnt) {
				return 1;
			}
		}else {
			break;
		}
	}

	write_byte(reg);
	if(no_ack == check_ack()) {
		i2c_stop();
		return 1;
	}

	i2c_start();
	write_byte(MAG3110_IIC_ADDRESS_READ);
	if(no_ack == check_ack()) {
		i2c_stop();
		return 1;
	}

	for(cnt = 0; cnt < num; cnt++) {
		buf[cnt] = read_byte();
		if(cnt < (num - 1)) {
			send_ack();
		}else {
			send_noack();
		}
	}
	i2c_stop();

	return 0;
}

static s16 csr_magnetometer_read(void *args)
{
	//u8 temp = 0xA1;
	u8 i = 0;
	mag_data_t *data = (mag_data_t *)args;
	
	csr_magnetometer_start_i2c();
	/*TempVal=0xA5;  OutPut Rate 2.5HZ, Over Sample Ratio 16,Bit2-Fast Read 8bit,Bit1-Normal Opterate, Bit0-Standby*/
	//csr_magnetometer_reg_write(CTRL_REG1, &temp, 1);

	for(i = 0; i < 250; i++) {
		I2C_DELAY(1000);
		I2C_DELAY(1000);
		I2C_DELAY(1000);
		I2C_DELAY(1000);
	}
	csr_magnetometer_reg_read(OUT_X_MSB, (u8 *)data, 6);
	return 0;
}

static s16 csr_magnetometer_init(cfg_t *args, event_callback cb)
{
    u8 temp = 0;

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

	//pin init
	PioSetMode(csr_mag3110.cfg.int1.num, pio_mode_user);
    PioSet(csr_mag3110.cfg.int1.num, 1);
	PioSetDir(csr_mag3110.cfg.int1.num, FALSE);
	PioSetPullModes(BIT_MASK(csr_mag3110.cfg.int1.num), pio_mode_strong_pull_up);
	
	csr_magnetometer_start_i2c();

	csr_magnetometer_reg_read(WHO_AM_I, &temp, 1);
	if(temp != MAG3110_NAME) {
		return 1;
	}

	temp = 0x80;
	csr_magnetometer_reg_write(CTRL_REG2, &temp, 1);
	
	//temp = 0xA0;
	//csr_magnetometer_reg_write(CTRL_REG1, &temp, 1);//mag3110 stop

	temp = 0xA1;
	csr_magnetometer_reg_write(CTRL_REG1, &temp, 1);
	csr_magnetometer_stop_i2c();
	return 0;
}

static s16 csr_magnetometer_uninit(void)
{
	csr_magnetometer_stop_i2c();
	
	//scl
	csr_mag3110.cfg.scl.group = 0;
	csr_mag3110.cfg.scl.num = 0;

	//sda	
	csr_mag3110.cfg.sda.group = 0;
	csr_mag3110.cfg.sda.num = 0;

	//int1
	csr_mag3110.cfg.int1.group = 0;
	csr_mag3110.cfg.int1.num = 0;

	csr_mag3110.magnetometer_cb = NULL;
	return 0;
}

magnetometer_t csr_magnetometer = {
	.magnetometer_init = csr_magnetometer_init,
	.magnetometer_read = csr_magnetometer_read,
	.magnetometer_uninit = csr_magnetometer_uninit,
};
