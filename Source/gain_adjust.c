/**
 * \ingroup		grp_functions
 *
 * \file		gain_adjust.c
 * \since		29.07.2009
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 * \version		4.0.0
 *
 * \brief		Module that dynamically adjusts the adepter's gain based on the peak-to-peak amplitude of the EEG signal.
 *
 * $Id: gain_adjust.c 55 2011-03-03 13:43:50Z andrei-jakab $
 */

//----------------------------------------------------------------------------------------------------------
//   								Includes
//----------------------------------------------------------------------------------------------------------
// AVR-LibC headers
#include <avr/pgmspace.h>

// standard C headers (also from AVR-LibC)
#include <stdint.h>

#include "globals.h"
#include "gain_adjust.h"
#include "alarms.h"
#include "drivers/pga112.h"

//----------------------------------------------------------------------------------------------------------
//   								Constants
//----------------------------------------------------------------------------------------------------------
static const uint8_t mc_uintPGAGains[GAINADJUST_NSTAGES] PROGMEM = {PGA112_G1,
																    PGA112_G2,
																    PGA112_G4,
																    PGA112_G8};	///< array containing the digipot level associated with each gain level

static const uint8_t mc_uintEEGLimits[GAINADJUST_NSTAGES][2] PROGMEM = {{89, 44 },
																	    {89, 44 },
																	    {89, 44 },
																	    {89, 44 }};

//----------------------------------------------------------------------------------------------------------
//   								Variables
//----------------------------------------------------------------------------------------------------------
static BOOL				m_blnWaitStateComplete;							///< 
static enum GA_STATE	m_gasCurrentState;								///< 

static uint16_t			m_uintSampleCounter;							///< amount of samples gathered so far
static uint8_t			m_uintLocalMax;									///< 
static uint8_t			m_uintLocalMin;									///< 

static uint8_t			m_uintGainStage;								///< current adapter gain level

//----------------------------------------------------------------------------------------------------------
//   								Code
//----------------------------------------------------------------------------------------------------------
/**
 * \brief		Initializes the gain adjustment module.
 *
 * \details		Initializes all of the module's global variables and sets the digipot to the level corresponding
 *			    to the maximum gain level. 
 *
 * \note		This function must be called before any other function in this module.
 */
void ga_init(void)
{
	m_uintGainStage = GAINADJUST_NSTAGES - 1;
	ga_reset();
	
	pga112_init();
	pga112_setChannel(PGA112_CH1);
	pga112_setGain(pgm_read_byte(&mc_uintPGAGains[m_uintGainStage]));

#ifdef DEBUGGING
	alarms_set_gain(m_uintGainStage);
#endif
}

/**
 * \brief		Resets the sample counter variables.
 */
void ga_reset(void)
{
	m_gasCurrentState = GA_GATHERANLYZE1;
}

/**
 * \brief		Handles new signal samples and, if neccessary, changes the gain level after \a GAINADJUST_INTERVAL samples have been gathered.
 *
 * \param[in]	uintSample	new signal sample
 *
 * \return		a member of the \c GA_STATE enumeration
 */
BOOL ga_newsample(const uint8_t uintNewSample)
{
	BOOL blnReturnValue = FALSE;
	uint8_t uintAmpPP;
	
	PORTC ^= _BV(PC1);

	switch(m_gasCurrentState)
	{
		case GA_GATHERANLYZE1:
			m_blnWaitStateComplete = FALSE;
			m_uintSampleCounter = 1;

			// initialize local min & max variables
			m_uintLocalMax = m_uintLocalMin = uintNewSample;

			// set next state
			m_gasCurrentState = GA_GATHERANLYZE2;
		break;

		case GA_GATHERANLYZE2:
			m_uintSampleCounter++;

			// check if sample represents new local min/max
			if(uintNewSample > m_uintLocalMax)
				m_uintLocalMax = uintNewSample;
			else if(uintNewSample < m_uintLocalMin)
				m_uintLocalMin = uintNewSample;

			// perform analysis if gathered enough data samples
			if(m_uintSampleCounter == GAINADJUST_DATAWINDOW)
			{
				// compute P-P amplitude
				uintAmpPP = m_uintLocalMax - m_uintLocalMin;

				// increase or decrease the gain depending on the P-P amplitude
				if(uintAmpPP <= pgm_read_byte(&mc_uintEEGLimits[m_uintGainStage][1]) && 
				   m_uintGainStage < (GAINADJUST_NSTAGES - 1))
				{
					// adjust gain state variable
					m_uintGainStage++;

					// change gain
					pga112_setGain(pgm_read_byte(&mc_uintPGAGains[m_uintGainStage]));

					// set next state & fcn return value
					m_gasCurrentState = GA_GATHERANLYZE1;
					blnReturnValue = TRUE;
				}
				else if(uintAmpPP >= pgm_read_byte(&mc_uintEEGLimits[m_uintGainStage][0]) && 
						m_uintGainStage > 0)
				{
					if(m_blnWaitStateComplete)
					{
						// adjust gain state variable
						m_uintGainStage--;

						// change gain
						pga112_setGain(pgm_read_byte(&mc_uintPGAGains[m_uintGainStage]));

						// set next state & fcn return value
						m_gasCurrentState = GA_GATHERANLYZE1;
						blnReturnValue = TRUE;
					}
					else
					{
						// reset sample counter
						m_uintSampleCounter = 0;

						// set next state
						m_gasCurrentState = GA_WAIT;
					}
				}
				else
					m_gasCurrentState = GA_GATHERANLYZE1;

#ifdef DEBUGGING
				alarms_set_gain(m_uintGainStage);
#endif
			}
		break;

		case GA_WAIT:
			if(++m_uintSampleCounter == GAINADJUST_DATAWINDOW)
			{
				m_blnWaitStateComplete = TRUE;

				// initialize local min & max variables
				m_uintLocalMax = m_uintLocalMin = uintNewSample;
				
				// reset sample counter
				m_uintSampleCounter = 1;

				// set next state
				m_gasCurrentState = GA_GATHERANLYZE2;
			}
		break;
	}

	return blnReturnValue;
}

void ga_enterDisplayScale(void)
{
	switch(m_uintGainStage)
	{
		// Gain step 1: 0.625 mV output signal required => set PGA gain = 1
		case 0:
			pga112_setGain(PGA112_G1);
		break;

		// Gain step 2: 1.250 mV output signal required => set PGA gain = 2
		case 1:
			pga112_setGain(PGA112_G2);
		break;

		// Gain step 3: 2.500 mV output signal required => set PGA gain = 4
		case 2:
			pga112_setGain(PGA112_G4);
		break;

		// Gain step 4: 5.00 mV output signal required => set PGA gain = 8
		case 3:
			pga112_setGain(PGA112_G8);
		break;

		default:
			alarms_set(AL_FATALERROR);
		break;
	}
}

void ga_exitDisplayScale(void)
{
	// set PGA gain to pre-Display Scale state value
	pga112_setGain(pgm_read_byte(&mc_uintPGAGains[m_uintGainStage]));
}
