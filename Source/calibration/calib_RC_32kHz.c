/**
 * \ingroup		grp_calibration
 *
 * \file		calib_RC_32kHz.c
 * \since		17.09.2010
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 * \version		1.0.0
 *
 * \brief		Module that calibrates the internal RC oscillator using the TOSC crystal.
 * \remarks		Ported to AVR-GCC for the ATmega164/324/644/1284 from AVR055 source code
 *				(Revision: 1.2; RCSfile: calib_32kHz.c,v; Date: 2006/02/17 12:49:26)
 *
 * $Id: calib_RC_32kHz.c 54 2011-02-11 19:07:47Z andrei-jakab $
 */

//----------------------------------------------------------------------------------------------------------
//   								Includes
//----------------------------------------------------------------------------------------------------------
// AVR-LibC headers
#include <avr/io.h>
#include <util/delay.h>

// application headers
#include "calib_RC_32kHz.h"

//----------------------------------------------------------------------------------------------------------
//   								Variables
//----------------------------------------------------------------------------------------------------------
unsigned char neighborsSearched;						///< Holds the number of neighbors searched
unsigned char calStep;									///< The binary search step size
unsigned char bestCountDiff = 0xFF;						///< The lowest difference between desired and measured counter value
unsigned char bestCountDiff_first;						///< Stores the lowest difference between desired and measured counter value for the first search
unsigned char bestOSCCAL;								///< The OSCCAL value corresponding to the bestCountDiff
unsigned char bestOSCCAL_first;							///< Stores the OSCCAL value corresponding to the bestCountDiff for the first search
unsigned int countVal;									///< The desired counter value
unsigned int calibration;								///< Calibration status
signed char sign;										///< Stores the direction of the binary step (-1 or 1)

//----------------------------------------------------------------------------------------------------------
//   								Static Functions
//----------------------------------------------------------------------------------------------------------
/*! \brief The Counter function
*
* This function increments a counter for a given ammount of ticks on
* on the external watch crystal.
*/
static unsigned int calibRC_Counter(void)
{
	unsigned int cnt;

	cnt = 0;                                                    // Reset counter
	TIMER = 0x00;                                               // Reset async timer/counter
	
	while (ASSR & ((1<<OUTPUT_COMPARE_UPDATE_BUSY)|(1<<TIMER_UPDATE_BUSY)|(1<<ASYNC_TIMER_CONTROL_UPDATE_BUSY))); // Wait until async timer is updated  (Async Status reg. busy flags).
	
	do
	{                                                           // cnt++: Increment counter - takes 2 cycles of code.
		cnt++;                                                  // 1 cycle required to read async TCNT
	}
	while (TIMER < EXTERNAL_TICKS);                             // CPI takes 1 cycle, BRCS takes 2 cycles, resulting in: 2+1(or 2)+1+2=6(or 7) CPU cycles
	
	return cnt;													// NB! Different compilers may give different CPU cycles!
}                                                               // Until 32.7KHz (XTAL FREQUENCY) * EXTERNAL TICKS

/*! \brief The binary search method
*
* This function uses the binary search method to find the
* correct OSSCAL value.
*/
static void calibRC_BinarySearch(unsigned int ct)
{

	if (ct > countVal)                                          // Check if count is larger than desired value
	{
		sign = -1;                                              // Saves the direction
		OSCCAL -= calStep;										// Decrease OSCCAL if count is too high
		NOP();
	}
	else if (ct < countVal)                                     // Opposite procedure for lower value
	{
		sign = 1;
		OSCCAL += calStep;
		NOP();
	}
	else                                                        // Perfect match, OSCCAL stays unchanged
	{
		calibration = FINISHED;
	}
	
	calStep >>= 1;
}

/*! \brief The neighbor search method
*
* This function uses the neighbor search method to improve the
* binary search result. It will always be called with a binary search
* prior to it.
*/
static void calibRC_NeighborSearch(void)
{
	neighborsSearched++;
	
	if (neighborsSearched == 4)									// Finish if 3 neighbors searched
	{
		OSCCAL = bestOSCCAL;
		calibration = FINISHED;
	}
	else
	{
		OSCCAL+=sign;
		NOP();
	}
}

/*! \brief Calibration function
*
* Performs the calibration according to calibration method chosen.
* Compares different calibration results in order to achieve optimal results.
*/
static void calibRC_CalibrateInternalRc(void)
{
	unsigned int count;
	unsigned char countDiff;
	unsigned char neighborSearchStatus = FINISHED;

	//PREPARE_CALIBRATION();													// Sets initial stepsize and sets calibration state to "running"

#ifdef CALIBRATION_METHOD_SIMPLE											// Simple search method
	unsigned char cycles = 0x80;

	do
	{
		count = calibRC_Counter();
		
		if (count > countVal)
			OSCCAL--;														// If count is more than count value corresponding to the given frequency:
		
		NOP();																// - decrease speed
		
		if (count < countVal)
			OSCCAL++;
		
		NOP();																// If count is less: - increase speed
		
		if (count == countVal)
			cycles=1;			
	} while(--cycles);														// Calibrate using 128(0x80) calibration cycles

#else																		// Binary search with or without neighbor search

	while(calibration == RUNNING)
	{
		count = calibRC_Counter();											// Counter returns the count value after external ticks on XTAL
		if (calStep != 0)
		{
			calibRC_BinarySearch(count);									// Do binary search until stepsize is zero
		}
		else
		{
			if(neighborSearchStatus == RUNNING)
			{
				countDiff = ABS((signed int)count-(signed int)countVal);
				
				if (countDiff < bestCountDiff)								// Store OSCCAL if higher accuracy is achieved
				{
					bestCountDiff = countDiff;
					bestOSCCAL = OSCCAL;
				}
				
				calibRC_NeighborSearch();									// Do neighbor search
			}
			else															// Prepare and start neighbor search
			{
				#ifdef CALIBRATION_METHOD_BINARY_WITHOUT_NEIGHBOR			// No neighbor search if deselected
				calibration = FINISHED;
				countDiff = ABS((signed int)count-(signed int)countVal);
				bestCountDiff = countDiff;
				bestOSCCAL = OSCCAL;
				#else
				neighborSearchStatus = RUNNING;								// Do neighbor search by default
				neighborsSearched = 0;
				countDiff = ABS((signed int)count-(signed int)countVal);
				bestCountDiff = countDiff;
				bestOSCCAL = OSCCAL;
				#endif
			}
		}
	}

	STOP_ASYNC_TIMER();
#endif
}

//----------------------------------------------------------------------------------------------------------
//   								Globally-accessible Functions
//----------------------------------------------------------------------------------------------------------
/*! \brief Initializes the calibration.
*
* Computes the count value needed to compare the desired internal oscillator
* speed with the external watch crystal, and sets up the asynchronous timer.
*/
void calibRC_Init(void)
{
	// configure Clock Prescale Register (CLKPR) so that MCU runs at 8MHz
	asm("ldi r16,0x80");
	asm("sts 0x61,r16");
	asm("ldi r16,0x00");
	asm("sts 0x61,r16");

	// Computes countVal for use in the calibration
	COMPUTE_COUNT_VALUE();

	OSCCAL = DEFAULT_OSCCAL;
	NOP();

	// Asynchronous timer setup
	SETUP_ASYNC_TIMER();

	// wait 1 second for oscillator to stabilize
	//_delay_ms(1000);

	// Sets initial stepsize and sets calibration state to "running"
	PREPARE_CALIBRATION();
}

void calibRC_Calibrate(void)
{
	//
	// Calibration
	//
	calibRC_CalibrateInternalRc();											// Calibrates to selected frequency

#ifndef CALIBRATION_METHOD_SIMPLE											// If simple search method is chosen, there is no need to do two calibrations.
#ifdef TWO_RANGES															// For devices with splitted OSCCAL register.
	if (bestCountDiff != 0x00)												// Do not do a second search if perfect match
	{
		OSCCAL = DEFAULT_OSCCAL_HIGH;										// Sets search range to upper part of OSCCAL
		NOP();
		bestOSCCAL_first = bestOSCCAL;										// Save OSCCAL value and count difference achieved in first calibration
		bestCountDiff_first = bestCountDiff;
		PREPARE_CALIBRATION();												// Search performed in lower OSCCAL range, perform search in upper OSCCAl range
		SETUP_ASYNC_TIMER();
		calibRC_CalibrateInternalRc();										// Perform a second search in upper part of OSCCAL

		if (bestCountDiff > bestCountDiff_first)							// Check which search gave the best calibration
		{
			OSCCAL = bestOSCCAL_first;										// First calibration is more accurate and OSCCAL is written accordingly
			NOP();
		}
	}
#endif
#endif
}
