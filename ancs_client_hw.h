/*******************************************************************************
 *  Copyright 2014-2015 Qualcomm Technologies International, Ltd.
 *  Part of CSR uEnergy SDK 2.5.1
 *  Application version 2.5.1.0
 *
 *  FILE
 *      ancs_client_hw.h
 *
 *  DESCRIPTION
 *      This file defines all the function which interact with hardware
 *
 *  NOTES
 *
 ******************************************************************************/
#ifndef __ANCS_CLIENT_HW_H__
#define __ANCS_CLIENT_HW_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <timer.h>

/*============================================================================*
 *  Local Header File
 *============================================================================*/
#include "ancs_client.h"
#include "ancs_service_data.h"
#include "user_config.h"

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Button PIO */
#define BUTTON_PIO                        (11)
#define BUTTON_PIO_MASK                   (PIO_BIT_MASK(BUTTON_PIO))

#define PIO_BIT_MASK(pio)                 (0x01 << (pio))

/* PIO direction */
#define PIO_DIRECTION_INPUT               (FALSE)
#define PIO_DIRECTION_OUTPUT              (TRUE)

/* PIO state */
#define PIO_STATE_HIGH                    (TRUE)
#define PIO_STATE_LOW                     (FALSE)

/*============================================================================*
 *         Public Function Prototypes
 *============================================================================*/

/* Initialise the ANCS tag hardware */
extern void InitAncsHardware(void);

/* This function initialises the application hardware data */
extern void AppHwDataInit(void);

/* This function handles the PIO changed event. */
extern void HandlePIOChangedEvent(uint32 pio_changed);

#endif /*__ANCS_CLIENT_HW_H__*/
