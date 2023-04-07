/**
 * \ingroup		grp_drivers 
 *
 * \file		mma7341lc.c
 * \since		17.09.2010
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 * \version		1.0.0
 *
 * \brief		MMA7341LC ±3g, ±11g Three Axis Low-g Micromachined Accelerometer driver.
 *
 * \note		Driver assumes that all MMA7341LC pins are connected to the same port.
 *
 * $Id: mma7341lc.c 52 2010-09-21 13:37:56Z andrei-jakab $
 */

//----------------------------------------------------------------------------------------------------------
//   								Includes
//----------------------------------------------------------------------------------------------------------
// AVR-LibC headers
#include <avr/io.h>

// application headers
#include "../globals.h"
#include "mma7341lc.h"

//----------------------------------------------------------------------------------------------------------
//   								Code
//----------------------------------------------------------------------------------------------------------

/**
 * \brief		Initializes the required driver variables and hardware pins.
 *
 * \details		The function sets the pins used for "g-Select", "Self Test" and "Sleep'" as outputs. The g-range
 *				is initialized to ±3g and the device is put in sleep mode.
 *
 * \note		This function must be called before any other function in this driver.
 */
void mma7341lc_init(void)
{
	// configure port pins directions ("g-Select", "Self Test" and "Sleep'": out)
	MMA7341LC_DDR |= (uint8_t) (1 << MMA7341LC_SLEEP | 0 << MMA7341LC_G | 0 << MMA7341LC_SELFTEST);
	
	// clear "Self Test" pin
	MMA7341LC_PORT &= (uint8_t) (0 << MMA7341LC_SELFTEST);

	mma7341lc_setSensitivity(FALSE);
	mma7341lc_setSleepMode(TRUE);
}

/**
 * \brief		Sets the accelerometer's g-range/sensitivity.
 *
 * \details		The function sets the g-range (and by association the sensitivity) of the accelerometer using the
 *				g-Select pin based on the value of \a blnExtendedRange.
 *
 * \param[in]	blnExtendedRange	determines the g-range (and sensitivity) to be set (TRUE = ±11g, 117.8mV/g; FALSE = ±3g, 440mV/g)
 */
void mma7341lc_setSensitivity(const BOOL blnExtendedRange)
{
	if(blnExtendedRange)
		MMA7341LC_PORT |= (uint8_t) (0 << MMA7341LC_G);
	else
		MMA7341LC_PORT &= (uint8_t) ~(0 << MMA7341LC_G);		
}

/**
 * \brief		Enables/disables the MMA7341LC's sleep mode.
 *
 * \param[in]	blnSleepOn	boolean that indicates whether the sleep mode should be enabled (TRUE) or disabled (FALSE)
 */
void mma7341lc_setSleepMode(const BOOL blnSleepOn)
{
	if(blnSleepOn)
	{
		// set 3g mode for lower power consumption
		mma7341lc_setSensitivity(FALSE);

		// enable sleep mode
		MMA7341LC_PORT &= (uint8_t) ~(1 << MMA7341LC_SLEEP);
	}
	else
		MMA7341LC_PORT |= (uint8_t) (1 << MMA7341LC_SLEEP);
}