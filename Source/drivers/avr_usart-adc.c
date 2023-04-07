/**
 * \ingroup		grp_drivers
 *
 * \file		avr_usart-adc.c
 * \since		20.08.2009
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 * \version		1.0.0
 *
 * \brief		AVR USART driver that imitates the ADC's functionality.
 *
 * $Id: avr_usart-adc.c 43 2009-09-29 11:37:23Z andrei-jakab $
 */

// AVR-LibC headers
#include <avr/io.h>
#include <avr/interrupt.h>

// application headers
#include "../globals.h"
#include "avr_usart-adc.h"

extern uint8_t					m_uintGainStage;

extern enum BACKGROUND_STATES	m_bkgState;

extern volatile uint8_t			m_uintADCSequencePointer;
extern volatile uint8_t			m_uintEEGSamples[256], m_uintXAccSamples[256], m_uintYAccSamples[256];
extern volatile uint8_t			m_uintNUnreadSamplesEEG, m_uintNUnreadSamplesXAcc, m_uintNUnreadSamplesYAcc;
extern volatile uint8_t			m_uintEEGSamplesRPtr, m_uintXAccSamplesRPtr, m_uintYAccSamplesRPtr;
extern volatile uint8_t			m_uintEEGSamplesWPtr, m_uintXAccSamplesWPtr, m_uintYAccSamplesWPtr;

//----------------------------------------------------------------------------------------------------------
//   								Code
//----------------------------------------------------------------------------------------------------------
/**
 * \brief		Initializes everything that is required for the operation of the AVR USART.
 *
 * \details		Performs the following initialization tasks: \n - sets the baud rate based on the \a USART_BAUDRATE macro \n - sets the frame format to 8N1 \n - enables the receiver & transmission circuitry \n - enables the receive complete interrupt
 *
 * \note		This function must be called before any other function in this driver.
 */
void avr_usart_init(enum BACKGROUND_STATES CurrentState)
{
	m_uintADCSequencePointer = 1;

	// set baud rate
	UBRRL = BAUD_PRESCALE_NS;						// load lower 8-bits of the baud rate value into the low byte of the UBRR register
	UBRRH = (uint8_t) (BAUD_PRESCALE_NS >> 8);		// load upper 8-bits of the baud rate value into the high byte of the UBRR register
	
	// set double-speed operation (see Table 69 for justification)
	//UCSRA = U2X;

	// set frame format: 8N1 (8-bit characters, no parity bits, 1 stop bits)
	// NOTE: URSEL bit set to select the UCRSC register
	UCSRC = (uint8_t) (URSEL | UCSZ_8 );

	// Turn on the transmission and reception circuitry; enable receive complete interrupt
	UCSRB = (uint8_t) (RXEN | TXEN | RXCIE);
}

/**
 * \brief		Enables the on-board USART and resets the necessary module variables.
 *
 * \details		The function resets the read & write pointers of the ADC data buffers and performs the same initialization of UCSRB as in the init function.
 */
void avr_usart_enable(void)
{
	// reset control variables
	m_uintNUnreadSamplesEEG = m_uintNUnreadSamplesXAcc = m_uintNUnreadSamplesYAcc = 0;
	m_uintEEGSamplesWPtr = m_uintXAccSamplesWPtr = m_uintYAccSamplesWPtr = 0;
	m_uintEEGSamplesRPtr = m_uintXAccSamplesRPtr = m_uintYAccSamplesRPtr = 0;

	// Turn on the transmission and reception circuitry; enable receive complete interrupt
	UCSRB = (uint8_t) (RXEN | TXEN | RXCIE);
}

/**
 * \brief		Disables the on-board USART.
 *
 * \note		If a transmission is in progress when this function is called, it will be aborted and the
 *				data will be lost.
 */
void avr_usart_disable(void)
{
	// Turn off the transmission and reception circuitry; disable receive complete interrupt
	UCSRB = (uint8_t) 0;
}

/**
 * \brief		USART Receive Complete ISR
 */
ISR(USART_RXC_vect)
{
	uint8_t uintReceivedByte;

	// store received byte
	uintReceivedByte = UDR;
	
	// store result and increment pointers
	m_uintEEGSamples[m_uintEEGSamplesWPtr++] = uintReceivedByte;
	m_uintNUnreadSamplesEEG++;									// wraps around by itself at 255 since it overflows
	
	// echo received byte back to PC
	UDR = (uint8_t) m_uintGainStage;
	//UDR = (uint8_t) m_bkgState;
}
