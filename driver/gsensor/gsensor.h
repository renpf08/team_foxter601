#ifndef GSENSOR_H
#define GSENSOR_H

#define LIS3DH_STATUS_REG_AUX      0x07     /*R*/
#define LIS3DH_OUT_ADC1_L          0x08     /*R*/
#define LIS3DH_OUT_ADC1_H          0x09     /*R*/
#define LIS3DH_OUT_ADC2_L          0x0A     /*R*/
#define LIS3DH_OUT_ADC2_H          0x0B     /*R*/
#define LIS3DH_OUT_ADC3_L          0x0C     /*R*/
#define LIS3DH_OUT_ADC3_H          0x0D     /*R*/
#define LIS3DH_INT_COUNTER_REG     0x0E     /*R*/
#define LIS3DH_WHO_AM_I            0x0F     /*R*/
#define LIS3DH_TEMP_CFG_REG        0x1F     /*RW*/
#define LIS3DH_CTRL_REG1           0x20     /*RW*/
#define LIS3DH_CTRL_REG2           0x21     /*RW*/
#define LIS3DH_CTRL_REG3           0x22     /*RW*/
#define LIS3DH_CTRL_REG4           0x23     /*RW*/
#define LIS3DH_CTRL_REG5           0x24     /*RW*/
#define LIS3DH_CTRL_REG6           0x25     /*RW*/
#define LIS3DH_REFERENCE           0x26     /*RW*/
#define LIS3DH_STATUS_REG2         0x27     /*R*/
#define LIS3DH_OUT_X_L             0x28     /*R*/
#define LIS3DH_OUT_X_H             0x29     /*R*/
#define LIS3DH_OUT_Y_L             0x2A     /*R*/
#define LIS3DH_OUT_Y_H             0x2B     /*R*/
#define LIS3DH_OUT_Z_L             0x2C     /*R*/
#define LIS3DH_OUT_Z_H             0x2D     /*R*/
#define LIS3DH_FIFO_CTRL_REG       0x2E     /*RW*/
#define LIS3DH_FIFO_SRC_REG        0x2F     /*R*/
#define LIS3DH_INT1_CFG            0x30     /*RW*/
#define LIS3DH_INT1_SOURCE         0x31     /*R*/
#define LIS3DH_INT1_THS            0x32     /*RW*/
#define LIS3DH_INT1_DURATION       0x33     /*RW*/
#define LIS3DH_CLICK_CFG           0x38     /*RW*/
#define LIS3DH_CLICK_SRC           0x39     /*R*/
#define LIS3DH_CLICK_THS           0x3A     /*RW*/
#define LIS3DH_TIME_LIMIT          0x3B     /*RW*/
#define LIS3DH_TIME_LATENCY        0x3C     /*RW*/
#define LIS3DH_TIME_WINDOW         0x3D     /*RW*/

#define LIS3DH_NAME_VALUE          0x33

#endif
