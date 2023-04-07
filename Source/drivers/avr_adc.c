/**
 * \ingroup		grp_drivers
 *
 * \file		avr_adc.c
 * \since		13.09.2010
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 * \version		1.0.0
 *
 * \brief		AVR ADC driver for the ATmega164/324/644/1284 family.
 *
 * $Id: avr_adc.c 55 2011-03-03 13:43:50Z andrei-jakab $
 */

//----------------------------------------------------------------------------------------------------------
//   								Includes
//----------------------------------------------------------------------------------------------------------
// AVR-LibC headers
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

// standard C headers (also from AVR-LibC)
#include <math.h>
#include <stdio.h>

// application headers
#include "../globals.h"
#include "avr_adc.h"
#include "avr_timer1.h"
#include "../alarms.h"

//----------------------------------------------------------------------------------------------------------
//   								Constants
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
//   								Module Variables
//----------------------------------------------------------------------------------------------------------
volatile uint8_t				m_uintEEGSamples[256];					///< buffer in which the EEG samples are stored (256 was chosen as length so that the 8-bit read & write pointers wrap around by themselves when they overflow)
volatile uint8_t				m_uintNUnreadSamplesEEG;				///< number of unread samples in \a m_uintEEGSamples
volatile uint8_t				m_uintEEGSamplesWPtr;					///< position in \a m_uintEEGSamples where the next data sample will be stored
volatile uint8_t				m_uintEEGSamplesRPtr;					///< position in \a m_uintEEGSamples from where a data sample will be read next

//----------------------------------------------------------------------------------------------------------
//   								Code
//----------------------------------------------------------------------------------------------------------

/**
 * \brief		Initializes everything that is required for the operation of the AVR ADC.
 *
 * \details		Performs the following initialization tasks: \n - configures the required ADC port pins \n - initializes the all of the variables associated with the data buffers \n - configures the ADC (trigger source, clock, enables the interrupt, and sets the reference) \n - starts the trigger source (if necessary)
 *
 * \note		This function must be called before any other function in this driver.
 *
 * \param[in]	CurrentState	mode in which the driver should be initialized (should correspond under normal circumstances to the current state of the main state machine)
 */
void avr_adc_init(void)
{
	// initialize variables
	m_uintNUnreadSamplesEEG  = m_uintEEGSamplesWPtr  = m_uintEEGSamplesRPtr  = 0;

	// configure required Port A pins for ADC usage	
	PORTA &= (uint8_t) ~(_BV(PA0) | _BV(PA1) | _BV(PA2) | _BV(PA7));	// no internal pull-up
	DDRA  &= (uint8_t) ~(_BV(PA0) | _BV(PA1) | _BV(PA2) | _BV(PA7));	// set pins to input

	// AVCC pin as voltage reference; ADC Result Left-Adjusted; Analog Channel ADC_EEG
	ADMUX = (uint8_t) (_BV(REFS0) | _BV(ADLAR) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0));

	// ADC Interrupt Enable;  Clear Interrupt Flag; Ck/16 ADC Clock prescaler (250kHz @ 4MHz)
	ADCSRA = (uint8_t) (_BV(ADIE) | _BV(ADPS2));
}

/**
 * \brief Disables the on-board ADC and its trigger source.
 *
 *	The function stops Timer/Counter1 and clears the ADEN bit in the ADCSRA register.
 */
void avr_adc_disable(void)
{	
	// stop ADC
	ADCSRA &= (uint8_t) ~(_BV(ADEN));
}

/**
 * \brief Enables the on-board ADC and its trigger source.
 *
 *	The function resets the read & write pointers of the ADC data buffers, restarts Timer/Counter1 and sets the ADEN bit in the ADCSRA register.
 */
void avr_adc_enable(void)
{
	// reset control variables
	m_uintNUnreadSamplesEEG = m_uintEEGSamplesWPtr = m_uintEEGSamplesRPtr = 0;
	
	// start ADC
	ADCSRA |= (uint8_t) (_BV(ADEN));
}

void avr_adc_startConversion(void)
{
	// start conversion
	ADCSRA |= (uint8_t) (_BV(ADSC));
}

/**
 * \brief		ADC Conversion Complete ISR
 *
 * \details		Performs the following tasks (in this order): \n - retrieves the data sample from the ADC data register and stores it in a temporary variable \n - resets the trigger source (if necessary) \n - stores the data sample in the appropriate data buffer \n - configures the ADC for the next conversion (if necessary)
 */
ISR(ADC_vect)
{
	PORTC ^= _BV(PC0);

	// read result from ADCH ( since result is left-adjusted
	// due to ADLAR = 1, can read only ADCH)
	uint8_t uintADCResult = ADCH;
	
	// disable ADC
	ADCSRA &= (uint8_t) ~(_BV(ADEN));
		
	// store new sample
	m_uintEEGSamples[m_uintEEGSamplesWPtr++] = uintADCResult;

	// increase # of unread samples variable
	if(m_uintNUnreadSamplesEEG == 255)
		m_uintNUnreadSamplesEEG = 0;
	else
		m_uintNUnreadSamplesEEG++;
}
