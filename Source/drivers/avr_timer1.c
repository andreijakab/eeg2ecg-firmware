/**
 * \ingroup		grp_drivers
 *
 * \file		avr_timer1.c
 * \since		15.07.2010
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 * \version		1.0.0
 *
 * \brief		AVR 16-bit Timer/Counter1 Driver for the ATmega164/324/644/1284 family.
 *
 * $Id: avr_timer1.c 55 2011-03-03 13:43:50Z andrei-jakab $
 */

//----------------------------------------------------------------------------------------------------------
//   								Includes
//----------------------------------------------------------------------------------------------------------
// AVR-LibC headers
#include <avr/io.h>
#include <avr/interrupt.h>

// application headers
#include "../globals.h"
#include "../alarms.h"
#include "avr_adc.h"
#include "avr_timer1.h"

//----------------------------------------------------------------------------------------------------------
//   								Constants
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
//   								Module Variables
//----------------------------------------------------------------------------------------------------------
volatile BOOL				m_blnTC1_ADC;
volatile BOOL				m_blnTC1_StateTransition;

static enum TIMER1_MODE		m_Mode = TMR1_OFF;				///< 
static volatile uint32_t	m_uintISRCount_OCR2A_State;		///< number of times the Timer/Counter2 overflow ISR has been called
static volatile uint32_t	m_uintISRInterval_OCR2A_State;	///< number of times the Timer/Counter2 overflow ISR has been called

//----------------------------------------------------------------------------------------------------------
//   								Code
//----------------------------------------------------------------------------------------------------------
/**
 * \brief		Initializes the required driver variables and hardware registers for the AVR Timer/Counter1.
 *
 * state_change_interval		given in seconds
 */
void avr_tc1_init(enum TIMER1_MODE mode, uint8_t state_change_interval)
{
	DDRC |= (uint8_t) _BV(PC1);

	m_Mode = mode;
	m_blnTC1_StateTransition = m_blnTC1_ADC = FALSE;
	m_uintISRCount_OCR2A_State = 0;

	// Initialize timer to CTC mode w/ TOP from OCR1A ; Clock prescaler 64
	TCCR1B = (uint8_t) (_BV(WGM12) | _BV(CS11) | _BV(CS10));

	// set ADC's sampling rate: f = fIO/[64 * (1 + OCR1A)]
	//OCR1A = 24;				//  2500 Hz
	OCR1A = 17;				//  2500 Hz (actual w/ uncalibrated RC oscillator)

	// set state 
	m_uintISRInterval_OCR2A_State = (((uint32_t) 62500*state_change_interval)/((uint32_t) 1 + OCR1A)) - 1;;

	// clear timer
	TCNT1 = 0;

	// enable Output Compare A interrupt
	TIMSK1 = (uint8_t) _BV(OCIE1A);
}

/**
 * \brief		Restarts the count of the AVR Timer/Counter1.
 *
 * \details		Sets the timer/counter's count register to 0 and connects the clock source required for the current operating mode.
 */
void avr_tc1_restart(void)
{
	m_blnTC1_StateTransition = m_blnTC1_ADC = FALSE;
	m_uintISRCount_OCR2A_State = 0;
	
	// Clock prescaler 64
	TCCR1B |= (uint8_t) (_BV(CS11) | _BV(CS10));

	// clear the timer/counter and its interrupt flags
	TCNT1 = 0;
}

/**
 * \brief		Stop the AVR Timer/Counter1.
 *
 * \details		Disconnects the timer/counter's clock source and clear its interrupt flags.
 */
void avr_tc1_stop(void)
{
	// stop the timer by clearing the clock source from TCCR1B
	TCCR1B &= (uint8_t) ~(_BV(CS12) | _BV(CS11) | _BV(CS10));
}

//----------------------------------------------------------------------------------------------------------
//   								Interrupts
//----------------------------------------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect)
{
	//
	// ADC Trigger
	//
	m_blnTC1_ADC = TRUE;
	
	//
	// State transition
	//
	if(++m_uintISRCount_OCR2A_State == m_uintISRInterval_OCR2A_State)
	{
		PORTC ^= (uint8_t) _BV(PC1);
		m_blnTC1_StateTransition = TRUE;
		m_uintISRCount_OCR2A_State = 0;
	}
}
