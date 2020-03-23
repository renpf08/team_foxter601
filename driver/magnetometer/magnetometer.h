#ifndef MAGNETOMETER_H
#define MAGNETOMETER_H

#define DR_STATUS   0x00    /*R*/
#define OUT_X_MSB	0x01	/*R*/
#define OUT_X_LSB	0x02	/*R*/
#define OUT_Y_MSB	0x03	/*R*/
#define OUT_Y_LSB	0x04	/*R*/
#define OUT_Z_MSB	0x05	/*R*/
#define OUT_Z_LSB	0x06	/*R*/
#define WHO_AM_I 	0x07	/*R*/
#define SYSMOD   	0x08	/*R*/
#define OFF_X_MSB	0x09	/*R/W*/
#define OFF_X_LSB	0x0A	/*R/W*/
#define OFF_Y_MSB	0x0B	/*R/W*/
#define OFF_Y_LSB	0x0C	/*R/W*/
#define OFF_Z_MSB	0x0D	/*R/W*/
#define OFF_Z_LSB	0x0E	/*R/W*/
#define DIE_TEMP 	0x0F	/*R*/
#define CTRL_REG1	0x10	/*R/W*/
#define CTRL_REG2	0x11	/*R/W*/

#define MAG3110_IIC_ADDRESS   		(0x0E)
#define MAG3110_IIC_ADDRESS_WRITE   (0x1C)
#define MAG3110_IIC_ADDRESS_READ    (0x1D)
#define MAG3110_NAME				(0xC4)


enum {
	ack = 0,
	no_ack = 1,
};

#endif
