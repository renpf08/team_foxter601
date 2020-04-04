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

	csr_gsensor_reg_write(LIS3DH_CTRL_REG2, 0x00);	/*bit7-6 HPM10��ͨ�˲�ģʽ ����ģʽ��bit54 HPCF21 ��ͨ�˲�����ֹƵ�ʣ�bit3 FDS �˲�����ѡ��  �أ�*/
												/*bit2 HPCLICK ��ͨ�˲���CLICK���� �أ�bit10 HPIS21��ͨ�˲������AOI�������ж�2,1��  ��*/
	csr_gsensor_reg_write(LIS3DH_CTRL_REG3, 0x10);	/*bit7 I1_CLICK  CLICK�ж���INT1.��.bit6 I1_AOI1 AOI1�ж���INT1.��. bit5 I1_AOI2 AOI2�ж���INT1. ��. bit4 I1_DRDY1 DRDY1�ж���INT1. ��.
												  bit3 I1_DRDY2 DRDY2�ж���INT1.��. bit2 I1_WTM FIFO Watermark�ж���INT1.��. bit1 I1_OVERRUN FIFO Overrun�ж���INT1.��.*/
	csr_gsensor_reg_write(LIS3DH_CTRL_REG4, 0x80);	/*bit7 BDU �����ݸ���.��������0 bit6 BLE ���ֽڸ��ֽ��ȳ��趨.���ֽ��ȳ�1. bit54 FS1-FS0 ȫ��Χ�����趨(00: +/- 2G��
												  bit3 HR �߷ֱ������ģ���0 bit21 ST1-ST0 �Բ�ģʽ�趨.����ģʽ00 bit0 SIM SPI�ӿ�ģʽ. ����0
												*/
	csr_gsensor_reg_write(LIS3DH_CTRL_REG5, 0x48);	/*bit7 BOOT ���������ڴ�ֵ. ����ģʽ0, bit6 FIFO_EN FIFO����.��1, bit3 LIR_INT1 �ڼĴ���INT1_SRC�����ж�����,��1�� 
												  bit2 D4D_INT1 4D����: 4D detection is enabled on INT1 when 6D bit on INT1_CFG is set to 1.
												*/
	csr_gsensor_reg_write(LIS3DH_FIFO_CTRL_REG, 0x80);/*bit76 FM1-FM0 FIFOģʽѡ��. Stream mode 10, bit5	TR �����¼���INT1 0, bit4-0 FTH4:0 Default value: 0*/
	csr_gsensor_reg_write(LIS3DH_INT1_CFG, 0x00); 	/*bit7 AOI ��������ж� bit6 6D ������������жϿ��� bit5  ZHIE/ZUPE Z���¼������ڷ����϶��ж� ��0, bit4 ZLIE/ZDOWNE Z���¼������ڷ����϶��ж� ��0,
												bit3 YHIE/YUPE Y���¼������ڷ����϶��ж� ��0, bit2 YLIE/YDOWNE Y���¼������ڷ����϶��ж� ��0,bit1 XHIE/XUPE Y���¼������ڷ����϶��ж� ��0, bit0 XLIE/XDOWNE Y���¼������ڷ����϶��ж� ��0,
												*/
	csr_gsensor_reg_write(LIS3DH_CLICK_CFG, 0x00);
	return 0;
}

static s16 csr_gsensor_uninit(void)
{
	csr_gsensor_lis3dh.cfg.clk.group = 0;
	csr_gsensor_lis3dh.cfg.clk.num = 0;

	//mosi pin
	csr_gsensor_lis3dh.cfg.mosi.group = 0;
	csr_gsensor_lis3dh.cfg.mosi.num = 0;

	//miso pin
	csr_gsensor_lis3dh.cfg.miso.group = 0;
	csr_gsensor_lis3dh.cfg.miso.num = 0;

	//cs pin
	csr_gsensor_lis3dh.cfg.cs.group = 0;
	csr_gsensor_lis3dh.cfg.cs.num = 0;

	//int1 pin
	csr_gsensor_lis3dh.cfg.int1.group = 0;
	csr_gsensor_lis3dh.cfg.int1.num = 0;

	//int2 pin
	csr_gsensor_lis3dh.cfg.int2.group = 0;
	csr_gsensor_lis3dh.cfg.int2.num = 0;

	//cb
	csr_gsensor_lis3dh.gsensor_cb = NULL;

	return 0;
}

gsensor_t csr_gsensor = {
	.gsensor_init = csr_gsensor_init,
	.gsensor_read = csr_gsensor_read,
	.gsensor_uninit = csr_gsensor_uninit,
};
