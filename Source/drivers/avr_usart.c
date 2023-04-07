/**
 * \ingroup		grp_drivers
 *
 * \file		avr_usart.c
 * \since		12.09.2010
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 * \version		1.0.0
 *
 * \brief		AVR USART driver for the ATmega164/324/644/1284 family.
 *
 * $Id: avr_usart.c 52 2010-09-21 13:37:56Z andrei-jakab $
 */

// AVR-LibC headers
#include <avr/io.h>
#include <avr/interrupt.h>

// application headers
#include "../globals.h"
#include "avr_usart.h"

static uint8_t						m_uintBuffer0[256];					///< buffer in which the data to be transmitted by the UART0 is stored (256 was chosen as length so that the 8-bit read & write pointers wrap around by themselves when they overflow)
static volatile uint8_t				m_uintBuffer0WPtr;					///< position in \a m_uintBuffer where the next data byte to be transmitted will be stored (updated by )
static volatile uint8_t				m_uintBuffer0RPtr;					///< position in \a m_uintBuffer from where the next data byte to be transmitted will be read

//----------------------------------------------------------------------------------------------------------
//   								Globally-accessible Code
//----------------------------------------------------------------------------------------------------------
/**
 * \brief		Initializes everything that is required for the operation of the AVR USART0.
 *
 * \details		Performs the following initialization tasks:\n
 *				- initializes the driver's global variables\n
 *				- sets the baud rate based on the \a USART_BAUDRATE macro\n
 *				- sets the frame format to 8N1
 *
 * \param[in]	blnDiagnostics	can be set to FALSE in order to initialize in normal mode or to TRUE in order to initialize in diagnostic mode
 *
 * \note		This function must be called before any other function in this driver.
 */
void avr_usart0_init(void)
{
	m_uintBuffer0WPtr = m_uintBuffer0RPtr = 0;

	// set baud rate
	UBRR0L = (uint8_t) BAUD_PRESCALE_NS;			// load lower 8-bits of the baud rate value into the low byte of the UBRR register
	UBRR0H = (uint8_t) (BAUD_PRESCALE_NS >> 8);		// load upper 8-bits of the baud rate value into the high byte of the UBRR register

	// aynchronous USART; set frame format: 8N1 (8-bit characters, no parity bits, 1 stop bits)
	UCSR0C = (uint8_t) (_BV(UCSZ01) | _BV(UCSZ00));
}

/**
 * \brief		Disables USART0 by turning off the transmission & reception circuitry as well as all interrupts.
 */
void avr_usart0_disable(void)
{
	// turn off transmission & reception circuitry and any enabled interrupts
	UCSR0B = (uint8_t) 0;
}

/**
 * \brief		Makes the MCU enter an infinite loop in which all the characters received via the USART are echoed back.
 */
void avr_usart0_echo(void)
{
	uint8_t uintReceivedByte;

	// turn on the transmission & reception circuitry
	UCSR0B = (uint8_t) (_BV(RXEN0) | _BV(TXEN0));

	//infinite loop
	for(;;)
	{
		// wait for data to be received and ready to be read from UDR
		while ((UCSR0A & RXC0) == 0) {};

		// store received byte
		uintReceivedByte = UDR0;
		
		// wait for UDR to allow writing
		while ((UCSR0A & UDRE0) == 0) {};
		
		// echo the received byte back to the computer:
		UDR0 = uintReceivedByte;
	}
}

/**
 * \brief		Sends data using USART0.
 *
 * \details		If there is enough room in the local buffer, the data to be transmitted gets copied to the local buffer.
 *				Afterwards, the transmission circuitry & interrupt are enabled, and the transmission is started by
 *				sending the first byte of data.
 *
 * \return		TRUE if data was accepted for transmission (i.e. no buffer overflow), FALSE otherwise.
 *
 * \param[in]	puintBuffer			pointer to buffer containing data to be send
 * \param[in]	uintBufferLength	amount of bytes in \a puintBuffer
 */
BOOL avr_usart0_send(uint8_t * puintBuffer, uint8_t uintBufferLength)
{
	uint8_t i;

	if(uintBufferLength < 256)
	{
		// calculate emtpy buffer space
		if(m_uintBuffer0RPtr < m_uintBuffer0WPtr)
			i = 255 - (m_uintBuffer0WPtr - m_uintBuffer0RPtr);
		else
			i = 255 - (m_uintBuffer0RPtr - m_uintBuffer0WPtr);
	
		// add data to UART buffer if enough space is available
		if(uintBufferLength <= i)
		{
			// transfer data to be sent to local buffer
			for(i = 0; i < uintBufferLength; i++)
				m_uintBuffer0[m_uintBuffer0WPtr++] = puintBuffer[i];
			
			// enable transmission complete interrupt; turn on the transmission circuitry
			UCSR0B = (uint8_t) _BV(TXCIE0) | _BV(TXEN0);
			
			// start transmission by sending first byte
			UDR0 = m_uintBuffer0[m_uintBuffer0RPtr++];

			return TRUE;
		}
	}

	return FALSE;
}

//----------------------------------------------------------------------------------------------------------
//   								Interrupts
//----------------------------------------------------------------------------------------------------------
/**
 * \brief		USART0 Transmission Complete ISR
 * 
 * \details		Transmits any unsent bytes from \a m_uintBuffer0.
 */
ISR(USART0_TX_vect)
{
	if(m_uintBuffer0RPtr != m_uintBuffer0WPtr)
		UDR0 = m_uintBuffer0[m_uintBuffer0RPtr++];
	else
		avr_usart0_disable();
}

/**
 * \brief		USART Receive Complete ISR
 */
ISR(USART0_RX_vect)
{
	uint8_t uintReceivedByte;

	// store received byte
	uintReceivedByte = UDR0;
	
	// echo received byte back to PC
	UDR0 = uintReceivedByte;
}
