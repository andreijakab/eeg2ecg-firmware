/**
 * \ingroup		grp_functions
 *
 * \file		alarms.c
 * \since		03.08.2009
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 * \version		2.0.0
 *
 * \brief		Module used to interface with the adapter's LEDs.
 * \remarks		Followed "International Labeling Requirements for Medical Devices, Medical Equipment and Diagnostic
				Products", 2nd ed, pg. 231 (which in turn follow BS EN 60601-1:2006 "Medical electrical equipment. General
				requirements for basic safety and essential performance".
 *
 * $Id: alarms.c 53 2011-02-10 22:13:03Z andrei-jakab $
 */

//----------------------------------------------------------------------------------------------------------
//   								Includes
//----------------------------------------------------------------------------------------------------------
// AVR-LibC headers
#include <avr/io.h>

// application headers
#include "globals.h"
#include "alarms.h"

//----------------------------------------------------------------------------------------------------------
//   								Variables
//----------------------------------------------------------------------------------------------------------
static BOOL		m_blnFatalErrorOccured;				///< indicates whether a fatal error has occured
static uint8_t	m_uintFlashingLEDs;					///<

//----------------------------------------------------------------------------------------------------------
//   								Code
//----------------------------------------------------------------------------------------------------------
/**
 * \brief		Initializes the module.
 *
 * \details		Initializes all of the module's global variables, configures the AVR ports to which the LEDs are connected.
 *				and turns all LEDs off.
 *
 * \note		This function must be called before any other function in this module.
 */
void alarms_init(void)
{
	m_blnFatalErrorOccured = FALSE;
	m_uintFlashingLEDs = 0x00;

	// configure led pins as outputs
	LED_DDR |= (uint8_t) (_BV(LED_RED) | _BV(LED_GREEN) | _BV(LED_BLUE));

	// make sure all LEDs are off
	LED_PORT |= (uint8_t) (_BV(LED_RED) | _BV(LED_GREEN) | _BV(LED_BLUE));
}

/**
 * \brief		Clearn an alarm.
 *
 * \details		Turns off the appropriate LEDs based on the type of alarms specified by \a alarm.
 *
 * \param[in]	alarm	type of alarm to clear
 */
void alarms_clear(enum ALARM_TYPE alarm)
{
	// LEDs can be changed only if a fatal error hasn't occured
	if(!m_blnFatalErrorOccured)
	{
		switch(alarm)
		{
			case AL_RECORDING:
				m_uintFlashingLEDs &= ~_BV(LED_GREEN);
				LED_PORT |= (uint8_t) _BV(LED_GREEN);
			break;
			
			case AL_DISPLAYSCALE:
				m_uintFlashingLEDs &= ~(_BV(LED_RED) | _BV(LED_GREEN));
				LED_PORT |= (uint8_t) (_BV(LED_RED) | _BV(LED_GREEN));
			break;

			case AL_CHARGING:
				LED_PORT |= (uint8_t) _BV(LED_RED);
			break;

			case AL_KEY_PRESS:
				m_uintFlashingLEDs &= ~_BV(LED_BLUE);
				LED_PORT |= (uint8_t) _BV(LED_BLUE);
			break;

			case AL_KEY_HOLD:
				LED_PORT |= (uint8_t) _BV(LED_BLUE);
			break;

			case AL_MOVEMENT:
				m_uintFlashingLEDs &= ~_BV(LED_RED);
				LED_PORT |= (uint8_t) _BV(LED_RED);
			break;

			default:
				alarms_set(AL_FATALERROR);
			break;
		}
	}
}

void alarms_flash(void)
{
	LED_PORT ^= (uint8_t) m_uintFlashingLEDs;
}

/**
 * \brief		Sets an alarm.
 *
 * \details		Turns on the appropriate LEDs based on the type of alarms specified by \a alarm.
 *
 * \param[in]	alarm	type of alarm to be set
 */
void alarms_set(enum ALARM_TYPE alarm)
{
// when in debugging mode, this function should not change the LEDs
//#ifndef _DEBUG
	
	// LEDs can be changed only if a fatal error hasn't occured
	if(!m_blnFatalErrorOccured)
	{
		/// \b Alarm \b effects:
		switch(alarm)
		{
			/// - \e Recording: Flashing Green (Red & Blue = OFF, Green = Flashing)
			case AL_RECORDING:
				//m_uintFlashingLEDs |= _BV(LED_GREEN);
				LED_PORT &= (uint8_t) ~_BV(LED_GREEN);
			break;
			
			/// - \e Display \e Scale: Flashing Yellow (Blue = OFF, Red & Green = Flashing)
			case AL_DISPLAYSCALE:
				m_uintFlashingLEDs |= (_BV(LED_RED) | _BV(LED_GREEN));
			break;

			/// - \e Charging: Red (Blue & Green = OFF, Red = On)
			case AL_CHARGING:
				LED_PORT &= (uint8_t) ~_BV(LED_RED);
			break;

			/// - \e Key \e Press: Flashing Blue (Red & Green = OFF, Blue = Flashing)
			case AL_KEY_PRESS:
				m_uintFlashingLEDs |= _BV(LED_BLUE);
			break;

			/// - \e Key \e Hold: Blue (Red & Green = OFF, Blue = On)
			case AL_KEY_HOLD:
				LED_PORT &= (uint8_t) ~_BV(LED_BLUE);
			break;

			/// - \e Movement: Flashing Red (Blue & Green = OFF, Red = Flashing)
			case AL_MOVEMENT:
				m_uintFlashingLEDs |= _BV(LED_RED);
			break;

			/// - \e Fatal \e Error: Green & Blue = OFF, Red = ON
			case AL_FATALERROR:
			default:
				LED_PORT = (uint8_t) ((LED_PORT & ~_BV(LED_RED)) | (_BV(LED_GREEN) | _BV(LED_BLUE)));
				
				m_blnFatalErrorOccured = TRUE;
			break;
		}
	}

//#endif
}

void alarms_set_gain(const uint8_t uintGain)
{
	switch(uintGain)
	{
		case 0:
			LED_PORT = (uint8_t) (LED_PORT | (LED_GREEN | LED_BLUE | LED_RED));
		break;

		case 1:
			LED_PORT = (uint8_t) ((LED_PORT | (LED_GREEN | LED_RED)) & ~LED_BLUE);
		break;

		case 2:
			LED_PORT = (uint8_t) ((LED_PORT | (LED_BLUE | LED_RED)) & ~LED_GREEN);
		break;

		case 3:
			LED_PORT = (uint8_t) ((LED_PORT | LED_RED) & ~(LED_BLUE | LED_GREEN));
		break;

		case 4:
			LED_PORT = (uint8_t) ((LED_PORT & ~LED_RED) | (LED_GREEN | LED_BLUE));
		break;

		default:
			LED_PORT = (uint8_t) (LED_PORT & ~(LED_GREEN | LED_BLUE | LED_RED));
	
	}
}
