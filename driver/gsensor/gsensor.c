#include <types.h>
#include <pio.h>
#include <pio_ctrlr.h>
#include <spi.h>
#include <i2c.h>
#include "gsensor.h"
#include "../driver.h"

typedef struct {
	gsensor_cfg_t cfg;
	event_callback gsensor_cb;
}csr_gsensor_t;

static csr_gsensor_t csr_gsensor_lis3dh = {
	.cfg = {
		.clk = {
				.group = 0,
				.num = 0,
			},
		.mosi = {
				.group = 0,
				.num = 0,
			},
		.miso = {
				.group = 0,
				.num = 0,
			},
		.cs = {
				.group = 0,
				.num = 0,
			},
		.int1 = {
				.group = 0,
				.num = 0,
			},
		.int2 = {
				.group = 0,
				.num = 0,
			},
	},
	.gsensor_cb = NULL,
};

static s16 csr_gsensor_reg_write(u8 addr, u8 data)
{
    PioSetPullModes(BIT_MASK(csr_gsensor_lis3dh.cfg.cs.num)| \
    				BIT_MASK(csr_gsensor_lis3dh.cfg.mosi.num)| \
    				BIT_MASK(csr_gsensor_lis3dh.cfg.miso.num)| \
    				BIT_MASK(csr_gsensor_lis3dh.cfg.clk.num), 
					pio_mode_strong_pull_up);
	
    /*bit7  R/W  Bit6 M/S  Bit5-0  Addr*/
    SpiInit(csr_gsensor_lis3dh.cfg.mosi.num,
    		csr_gsensor_lis3dh.cfg.miso.num,
    		csr_gsensor_lis3dh.cfg.clk.num,
    		csr_gsensor_lis3dh.cfg.cs.num);
	
    SpiWriteRegister(addr & 0x3F, data);
	
	PioSetPullModes(BIT_MASK(csr_gsensor_lis3dh.cfg.cs.num)| \
					BIT_MASK(csr_gsensor_lis3dh.cfg.mosi.num)| \
					BIT_MASK(csr_gsensor_lis3dh.cfg.miso.num)| \
					BIT_MASK(csr_gsensor_lis3dh.cfg.clk.num), 
					pio_mode_no_pulls);
	return 0;
}

static s16 csr_gsensor_reg_read(u8 addr, u8 *buf, u8 len)
{
	u8 reg_addr;

    PioSetPullModes(BIT_MASK(csr_gsensor_lis3dh.cfg.cs.num)| \
    				BIT_MASK(csr_gsensor_lis3dh.cfg.mosi.num)| \
    				BIT_MASK(csr_gsensor_lis3dh.cfg.miso.num)| \
    				BIT_MASK(csr_gsensor_lis3dh.cfg.clk.num), 
					pio_mode_strong_pull_up);

	/*bit7	R/W  Bit6 M/S  Bit5-0  Addr*/
	SpiInit(csr_gsensor_lis3dh.cfg.mosi.num,
			csr_gsensor_lis3dh.cfg.miso.num,
			csr_gsensor_lis3dh.cfg.clk.num,
			csr_gsensor_lis3dh.cfg.cs.num);

    reg_addr = addr&0x3F;
    reg_addr |= 0x80;

	if(len > 1) {
		reg_addr |= 0x40;
	}

    SpiReadRegisterBurst(reg_addr, buf, len, FALSE);	
	PioSetPullModes(BIT_MASK(csr_gsensor_lis3dh.cfg.cs.num)| \
					BIT_MASK(csr_gsensor_lis3dh.cfg.mosi.num)| \
					BIT_MASK(csr_gsensor_lis3dh.cfg.miso.num)| \
					BIT_MASK(csr_gsensor_lis3dh.cfg.clk.num), 
					pio_mode_no_pulls);

	return 0;
}

#if 0
void csr_gsensor_powerdown(void)
{
	csr_gsensor_reg_write(LIS3DH_CTRL_REG1, 0x08);
}
#endif

static s16 csr_gsensor_read(void *args)
{
	u8 temp = 0;
	gsensor_data_t *data = (gsensor_data_t *)args;

	csr_gsensor_reg_read(LIS3DH_FIFO_SRC_REG,&temp,1);
	if((temp & 0x20) == 0) {
        csr_gsensor_reg_read(LIS3DH_OUT_X_L, (u8 *)data, 6);		
		return 0;
	}else {
		return -1;
	}
}

static s16 csr_gsensor_init(cfg_t *args, event_callback cb)
{
    u8 temp = 0;

	//clock pin
	csr_gsensor_lis3dh.cfg.clk.group = args->gsensor_cfg.clk.group;
	csr_gsensor_lis3dh.cfg.clk.num = args->gsensor_cfg.clk.num;

	//mosi pin
	csr_gsensor_lis3dh.cfg.mosi.group = args->gsensor_cfg.mosi.group;
	csr_gsensor_lis3dh.cfg.mosi.num = args->gsensor_cfg.mosi.num;

	//miso pin
	csr_gsensor_lis3dh.cfg.miso.group = args->gsensor_cfg.miso.group;
	csr_gsensor_lis3dh.cfg.miso.num = args->gsensor_cfg.miso.num;

	//cs pin
	csr_gsensor_lis3dh.cfg.cs.group = args->gsensor_cfg.cs.group;
	csr_gsensor_lis3dh.cfg.cs.num = args->gsensor_cfg.cs.num;

	//int1 pin
	csr_gsensor_lis3dh.cfg.int1.group = args->gsensor_cfg.int1.group;
	csr_gsensor_lis3dh.cfg.int1.num = args->gsensor_cfg.int1.num;

	//int2 pin
	csr_gsensor_lis3dh.cfg.int2.group = args->gsensor_cfg.int2.group;
	csr_gsensor_lis3dh.cfg.int2.num = args->gsensor_cfg.int2.num;

	//cb
	csr_gsensor_lis3dh.gsensor_cb = cb;
	
	//pin config
    PioSetModes(BIT_MASK(csr_gsensor_lis3dh.cfg.int1.num) | BIT_MASK(csr_gsensor_lis3dh.cfg.int2.num), pio_mode_user);
	PioSetDir(csr_gsensor_lis3dh.cfg.int1.num, PIO_DIR_INPUT);
	PioSetDir(csr_gsensor_lis3dh.cfg.int2.num, PIO_DIR_INPUT);
	PioSetPullModes(BIT_MASK(csr_gsensor_lis3dh.cfg.int1.num) | BIT_MASK(csr_gsensor_lis3dh.cfg.int2.num), pio_mode_no_pulls);

    PioSetModes(BIT_MASK(csr_gsensor_lis3dh.cfg.cs.num), pio_mode_user);
	PioSetDir(csr_gsensor_lis3dh.cfg.cs.num, PIO_DIR_OUTPUT);

	//reg init
	csr_gsensor_reg_read(LIS3DH_WHO_AM_I,&temp,1);
	if(temp!=LIS3DH_NAME_VALUE) {
		return -1;
	}

	csr_gsensor_reg_write(LIS3DH_TEMP_CFG_REG, 0x00);
	csr_gsensor_reg_write(LIS3DH_CTRL_REG1, 0x5F);
	csr_gsensor_reg_read(LIS3DH_CTRL_REG1, &temp, 1);
	if(temp != 0x5F) {
		return -2;
	}

	csr_gsensor_reg_write(LIS3DH_CTRL_REG2, 0x00);
	csr_gsensor_reg_write(LIS3DH_CTRL_REG3, 0x10);
	csr_gsensor_reg_write(LIS3DH_CTRL_REG4, 0x80);
	csr_gsensor_reg_write(LIS3DH_CTRL_REG5, 0x48);
	csr_gsensor_reg_write(LIS3DH_FIFO_CTRL_REG, 0x80);
	csr_gsensor_reg_write(LIS3DH_INT1_CFG, 0x00);
	csr_gsensor_reg_write(LIS3DH_CLICK_CFG, 0x00);
	return 0;
}

gsensor_t csr_gsensor = {
	.gsensor_init = csr_gsensor_init,
	.gsensor_read = csr_gsensor_read,
};
