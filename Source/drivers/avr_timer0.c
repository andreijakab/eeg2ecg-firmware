/**
 * \ingroup		grp_drivers
 *
 * \file		avr_timer0.c
 * \since		20.07.2010
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 * \version		1.0.0
 *
 * \brief		AVR 16-bit Timer/Counter0 Driver for the ATmega164/324/644/1284 family.
 *
 * $Id: avr_timer0.c 53 2011-02-10 22:13:03Z andrei-jakab $
 */

//----------------------------------------------------------------------------------------------------------
//   								Includes
//----------------------------------------------------------------------------------------------------------
// AVR-LibC headers
#include <avr/io.h>
#include <avr/interrupt.h>

// application headers
#include "../globals.h"
#include "avr_timer0.h"

//----------------------------------------------------------------------------------------------------------
//   								Constants
//----------------------------------------------------------------------------------------------------------
static volatile uint8_t			mc_uintSineTable[128] = {128,134,140,147,153,159,165,171,177,182,188,193,199,204,209,213,218,222,226,230,234,237,240,243,245,248,250,251,253,254,254,255,255,255,254,254,253,251,250,248,245,243,240,237,234,230,226,222,218,213,209,204,199,193,188,182,177,171,165,159,153,147,140,134,128,122,116,109,103,97,91,85,79,74,68,63,57,52,47,43,38,34,30,26,22,19,16,13,11,8,6,5,3,2,2,1,1,1,2,2,3,5,6,8,11,13,16,19,22,26,30,34,38,43,47,52,57,63,68,74,79,85,91,97,103,109,116,122};	///< sine-wave look-up tables (one for each gain level)


//----------------------------------------------------------------------------------------------------------
//   								Module Variables
//----------------------------------------------------------------------------------------------------------
// DAC-related
static volatile uint16_t		m_uintSineTableIdx;					///< position in \a m_puintDACDataBuffer from where the next data sample will be read next

//----------------------------------------------------------------------------------------------------------
//   								Globally-accessible Code
//----------------------------------------------------------------------------------------------------------
/**
 * \brief		Initializes the required driver variables and hardware registers for the AVR Timer/Counter0.
 */
void avr_tc0_init(void)
{
	m_uintSineTableIdx = 0;

	// set PB4_OC0B pin as output and initialize to low
	DDRB |= (uint8_t) _BV(PB4);
	PORTB &= (uint8_t) ~(_BV(PB4));
	
	// initialize timer to fast PWM; clear OC0B on compare match & set OC0B at BOTTOM
	TCCR0A = (uint8_t) (_BV(WGM01) | _BV(WGM00) | _BV(COM0B1));
	
	// clock prescaler 8 (sets PWM frequency to fPWM = 4MHz/(8*256) = 1953 Hz)
	TCCR0B = (uint8_t) (_BV(CS01));

	// clear the timer/counter and its interrupt flags
	TCNT0 = 0;
	TIFR0 |= (uint8_t) (_BV(OCF0B) | _BV(OCF0A) | _BV(TOV0));

	// enable overflow interrupt
	TIMSK0 = (uint8_t) _BV(TOIE0);
}

/**
 * \brief		Stop the AVR Timer/Counter0.
 *
 * \details		Disconnects the timer/counter's clock source and clear its interrupt flags.
 */
void avr_tc0_stop(void)
{
	// disconnect clock source
	TCCR0B &= (uint8_t) (_BV(CS02) | _BV(CS01) |_BV(CS00));
}

//----------------------------------------------------------------------------------------------------------
//   								Interrupts
//----------------------------------------------------------------------------------------------------------
/**
 * \brief 	Timer/Counter0 Overflow ISR
 *
 * Used in \b Display \b Scale mode to load the next value from the sine look-up table.
 */
ISR(TIMER0_OVF_vect)
{
	// read out sample from the data buffer
	/*OCR1A = *(m_puintDACDataBuffer + m_uintDACDataBufferIdx);
	
	// increment sine table pointer
	m_uintDACDataBufferIdx = (m_uintDACDataBufferIdx + 1 == m_uintDACDataBufferLength) ? 0 : m_uintDACDataBufferIdx + 1;*/

	// read out sample from appropriate sine table
	OCR0B = mc_uintSineTable[m_uintSineTableIdx];

	// increment sine table pointer
	m_uintSineTableIdx = (m_uintSineTableIdx == 127) ? 0:m_uintSineTableIdx + 1;
}
