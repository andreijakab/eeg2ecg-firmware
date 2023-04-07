/**
 * \ingroup		grp_functions
 *
 * \file		acc_check.c
 * \since		11.01.2011
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 * \version		1.0.0
 *
 * \brief		.
 *
 * $Id$
 */

//----------------------------------------------------------------------------------------------------------
//   								Includes
//----------------------------------------------------------------------------------------------------------
// standard C headers (also from AVR-LibC)
#include <stdint.h>

// application headers
#include "globals.h"
#include "acc_check.h"
#include "alarms.h"

//----------------------------------------------------------------------------------------------------------
//   								Constants
//----------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------
//   								Variables
//----------------------------------------------------------------------------------------------------------
static uint32_t			m_uintRunningSum_X;			///<
static uint32_t			m_uintRunningSum_Y;			///<
static uint32_t			m_uintRunningSum_Z;			///<
static uint16_t			m_uintSampleCounter;		///<

//----------------------------------------------------------------------------------------------------------
//   								Local Code
//----------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------
//   								Globally-accessible Code
//----------------------------------------------------------------------------------------------------------
void ac_init(void)
{
	m_uintRunningSum_X = m_uintRunningSum_Y = m_uintRunningSum_Z = 0;
	m_uintSampleCounter = 0;
}

void ac_new_sample(uint8_t sample, enum AC_CHANNEL channel)
{
	// add new sample to the appropriate running sum
	switch(channel)
	{
		case AC_X:
			m_uintRunningSum_X += sample;
		break;

		case AC_Y:
			m_uintRunningSum_Y += sample;
		break;

		case AC_Z:
			m_uintRunningSum_Z += sample;
			m_uintSampleCounter++;
		break;
	}

	// if enough sample have been gathered, perform the rest of the calculation and
	// check result
	if(m_uintSampleCounter == AC_AVERAGE_INTERVAL_DEC)
	{
		// finnish interval average calculation
		m_uintRunningSum_X = m_uintRunningSum_X >> AC_AVERAGE_INTERVAL_POW_2;
		m_uintRunningSum_Y = m_uintRunningSum_Y >> AC_AVERAGE_INTERVAL_POW_2;
		m_uintRunningSum_Z = m_uintRunningSum_Z >> AC_AVERAGE_INTERVAL_POW_2;

		/// TODO: check result

	}
}