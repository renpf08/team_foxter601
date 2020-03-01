/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *  FILE
 *      lcd_display.c
 *
 *  DESCRIPTION
 *      LCD character display functions.
 *
 ******************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <i2c.h>            
#include <pio.h>            
#include <types.h>          

/*============================================================================
 *  Local Header Files
 *============================================================================*/

#include "lcd_display.h"

/*============================================================================
 *  Private Defintions
 *============================================================================*/

/* Change these pin assignments depending on the hardware used. */
#if defined(CNS12016) /* CNS12016: CSR1011 Development Board */
#define PIO_SDA (13)  /* J2.09   */
#define PIO_SCL (14)  /* J2.10   */
#define PIO_RST (15)  /* J2.11   */
#define PIO_LED (16)  /* J2.12   */
#else /* Assume H13137: uEnergy Starter Dev Kit using MOD1 (CSR1010) and J100 */
#define PIO_SDA (3)   /* J100.07 */
#define PIO_SCL (4)   /* J100.08 */
#define PIO_RST (9)   /* J100.13 */
#define PIO_LED (10)  /* J100.14 */
#endif /* defined(CNS12016) */

#define PWM_ID_LED (0)  /* PWM id for LED backlight. */
#define STRLEN_MAX (40) /* Max length of string for this display. */

/*============================================================================*
 *  Private Data Types
 *============================================================================*/


/*============================================================================*
 *  Private Data
 *============================================================================*/


/*============================================================================*
 *  Public Data
 *============================================================================*/


/*============================================================================
 *  Private Function Prototypes
 *============================================================================*/


/*============================================================================
 *  Private Function Implementations
 *============================================================================*/


/*============================================================================
 * Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      LcdDisplayBacklight
 *
 *  DESCRIPTION
 *      Set the PWM configuration for the LCD backlight.
 *      The brightness percentages are approximate.
 *
 *  PARAMETERS
 *      dull [in]            Percentage of brightness at dull end of sequence.
 *      bright [in]          Percentage of brightness at bright end of sequence.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

void LcdDisplayBacklight(uint8 dull, uint8 bright)
{
    PioConfigPWM
    (
        PWM_ID_LED, pio_pwm_mode_push_pull,
        /* All time units are ~30us except for hold_time (which is ~16ms). */
        dull,   100 - (dull   % 101), 20, /* dull_on/off/hold_time   */
        bright, 100 - (bright % 101), 20, /* bright_on/off/hold_time */
        255                               /* ramp_rate               */
    );
} /* LcdDisplayBacklight */


/*----------------------------------------------------------------------------*
 *  NAME
 *      LcdDisplayClear
 *
 *  DESCRIPTION
 *      Clears the LCD display.
 *
 *  PARAMETERS
 *      i2c_address [in]     The 7 bit I2C address.
 *                           This is left shifted when sent to the device.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

void LcdDisplayClear(uint8 i2c_address)
{
    /* Change these codes depending on the LCD display being used. */
    /* Clear (or Home) must be the last command. */
    uint8 lcd_cmd[] = {0x00, 0x01};
    LcdDisplayI2cWrite(i2c_address, lcd_cmd, sizeof(lcd_cmd)/sizeof(lcd_cmd[0]));
    TimeDelayUSec(1000); /* Need long delay after Clear and Home. */
} /* LcdDisplayClear */


/*----------------------------------------------------------------------------*
 *  NAME
 *      LcdDisplayHome
 *
 *  DESCRIPTION
 *      Homes the LCD display cursor.
 *
 *  PARAMETERS
 *      i2c_address [in]     The 7 bit I2C address.
 *                           This is left shifted when sent to the device.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

void LcdDisplayHome(uint8 i2c_address)
{
    /* Change these codes depending on the LCD display being used. */
    /* Clear (or Home) must be the last command. */
    uint8 lcd_cmd[] = {0x00, 0x02};
    LcdDisplayI2cWrite(i2c_address, lcd_cmd, sizeof(lcd_cmd)/sizeof(lcd_cmd[0]));
    TimeDelayUSec(1000); /* Need long delay after Clear and Home. */
} /* LcdDisplayHome */


/*----------------------------------------------------------------------------*
 *  NAME
 *      LcdDisplayInit
 *
 *  DESCRIPTION
 *      Initialises the LCD display.
 *      The RST pin is toggled and then the LCD initialisation codes are sent.
 *
 *  PARAMETERS
 *      i2c_address [in]     The 7 bit I2C address.
 *                           This is left shifted when sent to the device.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

void LcdDisplayInit(uint8 i2c_address)
{
    /* Reset the display in case it is in unknown state. */
    PioSet(PIO_RST, FALSE);
    TimeDelayUSec(MILLISECOND);
    PioSet(PIO_RST, TRUE);
    TimeDelayUSec(MILLISECOND);

    /* Change these codes depending on the LCD display being used. */
    /* Clear (or Home) must be the last command. */
    uint8 lcd_init[] = {0x00, 0x39, 0x14, 0x74, 0x54, 0x6F, 0x38, 0x0C, 0x06, 0x01};
    LcdDisplayI2cWrite(i2c_address, lcd_init, sizeof(lcd_init)/sizeof(lcd_init[0]));
    TimeDelayUSec(1000); /* Need long delay after Clear and Home. */
} /* LcdDisplayInit */


/*----------------------------------------------------------------------------*
 *  NAME
 *      LcdDisplayInitI2c
 *
 *  DESCRIPTION
 *      Initialise the LCD display SDA and SCL pins.
 *      This may need to be changed if there are multiple devices on the 
 *      same I2C bus.
 *      This needs to be called each time the LCD is communicated with if the
 *      I2C has been disabled following I2C access with the NVM.
 *
 *  PARAMETERS
 *      None.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

void LcdDisplayInitI2c(void)
{
    I2cEnable(TRUE);

    /* I2cInit(uint8 sda_pio, uint8 scl_pio, uint8 power_pio, pio_pull_mode pull); */
    /* I2cInit() resets clock to 100kHz. */
    I2cInit(PIO_SDA, PIO_SCL, I2C_POWER_PIO_UNDEFINED, pio_i2c_pull_mode_strong_pull_up);

    I2cConfigClock(I2C_SCL_400KBPS_HIGH_PERIOD, I2C_SCL_400KBPS_LOW_PERIOD);
} /* LcdDisplayInitI2c */


/*----------------------------------------------------------------------------*
 *  NAME
 *      LcdDisplayInitPwm
 *
 *  DESCRIPTION
 *      Initialise PWM for the backlight.
 *
 *  PARAMETERS
 *      None.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

void LcdDisplayInitPwm(void)
{
    PioSetMode(PIO_LED, pio_mode_user);
    PioSetDir(PIO_LED, TRUE); /* Output */
    PioSet(PIO_LED, FALSE);

    PioSetMode(PIO_LED, pio_mode_pwm0); /* PIO is controlled by PWM. */

    LcdDisplayBacklight(10, 10);        /* Start with dim light. */
    
    PioEnablePWM(PWM_ID_LED, TRUE);
} /* LcdDisplayInitPwm */


/*----------------------------------------------------------------------------*
 *  NAME
 *      LcdDisplayInitRst
 *
 *  DESCRIPTION
 *      Initialise the LCD RST pins.
 *
 *  PARAMETERS
 *      None.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

void LcdDisplayInitRst(void)
{
    /* Set up the PIOs for the RST pin on the LCD display. */
    PioSetPullModes((1UL << PIO_RST), pio_mode_no_pulls);
    PioSetMode(PIO_RST, pio_mode_user);
    PioSetDir(PIO_RST, TRUE); /* Output */
    PioSet(PIO_RST, TRUE);
} /* LcdDisplayInitRst */


/*----------------------------------------------------------------------------*
 *  NAME
 *      LcdDisplayI2cWrite
 *
 *  DESCRIPTION
 *      Write data to the I2C device.
 *
 *  PARAMETERS
 *      i2c_address [in]     The 7 bit I2C address.
 *                           This is left shifted when sent to the device.
 *      data [in]            Pointer to the data buffer to be transmitted.
 *      num_bytes [in]       Number of bytes to be transmitted.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

void LcdDisplayI2cWrite(uint8 i2c_address, const uint8 *data, uint8 num_bytes)
{
    LcdDisplayInitI2c();

    bool wait = TRUE;
    I2cRawStart(wait);
    I2cRawWriteByte(i2c_address << 1); /* Bit 0 is R/!W. */
    I2cRawWaitAck(wait);
    if((data[0] & 0x3F) != 0)
    {
        /* Caller did not put in leading command byte.  Assume data. */
        I2cRawWriteByte('@'); /* Data follows. */
        I2cRawWaitAck(wait);
    }
    I2cRawWrite(data, num_bytes);
    I2cRawStop(wait);
    
    I2cRawTerminate();
} /* LcdDisplayI2cWrite */

/*----------------------------------------------------------------------------*
 *  NAME
 *      LcdDisplayI2cWriteString
 *
 *  DESCRIPTION
 *      Write string to the I2C device.
 *
 *  PARAMETERS
 *      i2c_address [in]     The 7 bit I2C address.
 *                           This is left shifted when sent to the device.
 *      data [in]            Pointer to the string buffer to be transmitted.
 *                           Write the first 40 chars (or until the first zero).
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

void LcdDisplayI2cWriteString(uint8 i2c_address, const uint8 *data)
{
    LcdDisplayInitI2c();

    bool wait = TRUE;
    uint8 strlen;
    for(strlen = 0; (strlen <= STRLEN_MAX) && (data[strlen] != 0); strlen++){;} 
    I2cRawStart(wait);
    I2cRawWriteByte(i2c_address << 1); /* Bit 0 is R/!W. */
    I2cRawWaitAck(wait);
    I2cRawWriteByte('@'); /* Data follows. */
    I2cRawWaitAck(wait);
    I2cRawWrite(data, strlen);
    I2cRawStop(wait);
    
    I2cRawTerminate();
} /* LcdDisplayI2cWriteString */

/*----------------------------------------------------------------------------*
 *  NAME
 *      LcdDisplaySetCursor
 *
 *  DESCRIPTION
 *      Set the cursor position in the DDRAM.
 *      Top left is (0, 0).
 *      Currently assumes a 2 line display.
 *
 *  PARAMETERS
 *      i2c_address [in]     The 7 bit I2C address.
 *                           This is left shifted when sent to the device.
 *      column [in]          Column position (0 is first location).
 *      line [in]            Line number (0 is top line).
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

void LcdDisplaySetCursor(uint8 i2c_address, uint8 column, uint8 line)
{
    uint8 cmd[2] = {0, 0}; /* First zero means command follows. */
    
    /* Change these codes depending on the LCD display being used. */
    /* 0x80 is the command for SET_DDRAM_ADDRESS.                  */
    /* For a 2 line display: 0x40 is line 1 (bottom line).         */
    /* For a 3 line display: 0x10 is line 1, 0x20 is line 2.       */
    cmd[1] = 0x80 | (line == 0 ? 0 : 0x40) | (column & 0x3F);

    LcdDisplayI2cWrite(i2c_address, cmd, 2);
} /* LcdDisplaySetCursor */


void LcdDisplayShiftCursorLeft(uint8 i2c_address)
{
    /* Change these codes depending on the LCD display being used. */
    uint8 cmd[] = {0x00, 0x38, 0x10}; // 0x14 = cursor right, 0x10 = cursor left.
    LcdDisplayI2cWrite(i2c_address, cmd, sizeof(cmd)/sizeof(cmd[0]));
}
void LcdDisplayShiftCursorRight(uint8 i2c_address)
{
    /* Change these codes depending on the LCD display being used. */
    uint8 cmd[] = {0x00, 0x38, 0x14}; // 0x14 = cursor right, 0x10 = cursor left.
    LcdDisplayI2cWrite(i2c_address, cmd, sizeof(cmd)/sizeof(cmd[0]));
}
void LcdDisplayShiftScreenLeft(uint8 i2c_address)
{
    /* Change these codes depending on the LCD display being used. */
    uint8 cmd[] = {0x00, 0x38, 0x18}; // 0x1C = screen right, 0x18 = screen left.
    LcdDisplayI2cWrite(i2c_address, cmd, sizeof(cmd)/sizeof(cmd[0]));
}
void LcdDisplayShiftScreenRight(uint8 i2c_address)
{
    /* Change these codes depending on the LCD display being used. */
    uint8 cmd[] = {0x00, 0x38, 0x1C}; // 0x1C = screen right, 0x18 = screen left.
    LcdDisplayI2cWrite(i2c_address, cmd, sizeof(cmd)/sizeof(cmd[0]));
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      TimeDelayMSec
 *
 *  DESCRIPTION
 *      Delay for specified milliseconds (approximately).  Max 65535 (65 secs).
 *
 *  PARAMETERS
 *      delay [in]           Number of ms to delay.
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

void TimeDelayMSec(uint16 delay)
{
    for(; delay > 0; delay--)
    {
        TimeDelayUSec(1000);
    }
} /* TimeDelayMSec */


/*============================================================================
 * End of file: lcd_display.c
 *============================================================================*/

