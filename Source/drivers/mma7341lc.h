/**
 * \ingroup		grp_drivers 
 *
 * \file		mma7341lc.h
 * \since		17.09.2010
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 *
 * \brief		Header file of the MMA7341LC ±3g, ±11g Three Axis Low-g Micromachined Accelerometer driver.
 *
 * $Id: mma7341lc.h 52 2010-09-21 13:37:56Z andrei-jakab $
 */

#ifndef __MMA7341LC_H__
#define __MMA7341LC_H__

//----------------------------------------------------------------------------------------------------------
//   								Application-Specific Definitions
//----------------------------------------------------------------------------------------------------------
#if (HW_VERSION == 30)
// MMA7341LC Port
#define MMA7341LC_PORT			PORTB			///< Data register (i.e. PORTx) of the AVR port to which the MMA7341LC's control lines are connected
#define MMA7341LC_DDR			DDRB			///< Data direction register (i.e. DDRx) of the AVR port to which the MMA7341LC's control lines are connected

// Pins used by MMA7341LC
#define MMA7341LC_G				0x00			///< port pin to which the MMA7341LC's g-Select control line is connected
#define MMA7341LC_SLEEP			PB2				///< port pin to which the MMA7341LC's Sleep' control line is connected
#define MMA7341LC_SELFTEST		0x00			///< port pin to which the MMA7341LC's Self Test control line is connected
#endif

//----------------------------------------------------------------------------------------------------------
//   								Structs/Enums
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
//   								Prototypes
//----------------------------------------------------------------------------------------------------------
void	mma7341lc_init(void);
void	mma7341lc_setSensitivity(const BOOL blnExtendedRange);
void	mma7341lc_setSleepMode(const BOOL blnSleepOn);

#endif
