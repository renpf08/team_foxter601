/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      lcd_display.h
 *
 *  DESCRIPTION
 *      LCD display interface.
 *
 ******************************************************************************/


#ifndef __LCD_DISPLAY_H__
#define __LCD_DISPLAY_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>


/*============================================================================
 *  Local Header Files
 *============================================================================*/


/*============================================================================
 *  Public Defintions
 *============================================================================*/


/*============================================================================*
 *  Public Data Types
 *============================================================================*/


/*============================================================================*
 *  Public Data
 *============================================================================*/


/*============================================================================
 *  Public Function Prototypes
 *============================================================================*/

void LcdDisplayBacklight(uint8 dull, uint8 bright);
void LcdDisplayClear(uint8 i2c_address);
void LcdDisplayHome(uint8 i2c_address);
void LcdDisplayInit(uint8 i2c_address);
void LcdDisplayInitI2c(void);
void LcdDisplayInitPwm(void);
void LcdDisplayInitRst(void);
void LcdDisplayI2cWrite(uint8 i2c_address, const uint8 *data, uint8 num_bytes);
void LcdDisplayI2cWriteString(uint8 i2c_address, const uint8 *data);
void LcdDisplaySetCursor(uint8 i2c_address, uint8 column, uint8 line);
void LcdDisplayShiftCursorLeft(uint8 i2c_address);
void LcdDisplayShiftCursorRight(uint8 i2c_address);
void LcdDisplayShiftScreenLeft(uint8 i2c_address);
void LcdDisplayShiftScreenRight(uint8 i2c_address);

void TimeDelayMSec(uint16 delay);

#endif /* __LCD_DISPLAY_H__ */
