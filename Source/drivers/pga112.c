/**
 * \ingroup		grp_drivers
 *
 * \file		pga112.c
 * \since		16.07.2010
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 * \version		1.0.0
 *
 * \brief		AVR driver for the TI PGA112.
 *
 * $Id: pga112.c 52 2010-09-21 13:37:56Z andrei-jakab $
 */

//----------------------------------------------------------------------------------------------------------
//   								Includes
//----------------------------------------------------------------------------------------------------------
// AVR-LibC headers
#include <avr/io.h>
#include <avr/interrupt.h>

// application headers
#include "../globals.h"
#include "pga112.h"

//----------------------------------------------------------------------------------------------------------
//   								Constants
//----------------------------------------------------------------------------------------------------------
//
// SPI Commands
//
// Read
static uint8_t PGA112_READ_B2 = 0x6A;

// Write
static uint8_t PGA112_WRITE_B2 = 0x2A;

// Shutdown
static uint8_t PGA112_SDN_B2 = 0xE1;
static uint8_t PGA112_SDN_B1_E = 0xF1;

//----------------------------------------------------------------------------------------------------------
//   								Module Variables
//----------------------------------------------------------------------------------------------------------
BOOL					m_blnSleeping;
enum PGA112_GAINS		m_Gain;
enum PGA112_CHANNELS	m_Channel;

//----------------------------------------------------------------------------------------------------------
//   								Locally-accessible Code
//----------------------------------------------------------------------------------------------------------
static inline uint8_t spi_masterTransmit(uint8_t uintByte1, uint8_t uintByte0)
{
	uint8_t uintTemp;

	// enable chip to receive by setting SS low
	PGA112_PORT &= (uint8_t) ~_BV(PGA112_SS);

#ifndef USE_USART1_SPI
	// load first byte into shift register and wait for it to be transmitted
	SPDR = uintByte1;
	while(!(SPSR & _BV(SPIF)));

	// load second byte into shift register and wait for it to be transmitted
	SPDR = uintByte0;
	while(!(SPSR & _BV(SPIF)));

	// store received data
	uintTemp = SPDR;
#else
	// wait for empty transmit buffer
	while(!(UCSR1A & _BV(UDRE1)));

	// load first byte into shift register
	UDR1 = uintByte1;

	// wait for empty transmit buffer
	while(!(UCSR1A & _BV(UDRE1)));

	// wait for data to be received
	while(!(UCSR1A & _BV(RXC1)));

	// store received data
	uintTemp = UDR1;

	// load second byte into shift register
	UDR1 = uintByte0;

	// wait for data to be transmitted
	while(!(UCSR1A & _BV(TXC1)));

	// wait for data to be received
	while(!(UCSR1A & _BV(RXC1)));

	// store received data
	uintTemp = UDR1;

	// clear flags
	UCSR1A |= (uint8_t) (_BV(RXC1) | _BV(TXC1) | _BV(UDRE1));
#endif

	// finish transmission by setting SS high
	PGA112_PORT |= (uint8_t) _BV(PGA112_SS);

	return uintTemp;
}

static inline void pga112_read(uint8_t * puintConfiguration)
{
	// transmit read command
	spi_masterTransmit(PGA112_READ_B2, 0);

	// store & store reply
#ifndef USE_USART1_SPI
	*puintConfiguration = spi_masterTransmit(0, 0);
#else
	*puintConfiguration = spi_masterTransmit(0, 0);
#endif
}

static void pga112_write(uint8_t command)
{
	uint8_t uintByte1, uintByte0;

	uintByte1 = command;
	uintByte0 = 0;
	
	// determine second command byte
	if(uintByte1 == PGA112_WRITE_B2)
	{
		uintByte0 = (uint8_t) ((m_Gain << 4) | m_Channel);
	}
	else if(uintByte1 == PGA112_SDN_B2)
	{
		if(m_blnSleeping)
			uintByte0 = PGA112_SDN_B1_E;
	}
	
	// transmit command to PGA112
	spi_masterTransmit(uintByte1, uintByte0);
}

//----------------------------------------------------------------------------------------------------------
//   								Globally-accessible Code
//----------------------------------------------------------------------------------------------------------

/**
 * \brief		Initializes everything that is required for the operation of the PGA112.
 *
 * \details		Performs the following initialization tasks: \n - configures the required SPI pins \n - initializes the all of the variables associated with the data buffers \n - configures the ADC (trigger source, clock, enables the interrupt, and sets the reference) \n - starts the trigger source (if necessary)
 *
 * \note		This function must be called before any other function in this driver.
 */
void pga112_init(void)
{
#ifdef USE_USART1_SPI
	// disable USART1 Power Reduction bit (PRUSART1) to power up USART1 module
	PRR0 &= (uint8_t) ~_BV(PRUSART1);

	// set baud rate register to 0 (required to use USART in SPI mode)
	UBRR1 = 0;
#endif

	// set MOSI, SCK & SS as output and MISO as input
	PGA112_PORT &= (uint8_t) ~(_BV(PGA112_SCK) | _BV(PGA112_MISO) | _BV(PGA112_MOSI));	// no internal pull-ups / set pins outputs low
	PGA112_PORT |= (uint8_t) _BV(PGA112_SS);											// set SS to output high
	PGA112_DDR  |= (uint8_t) (_BV(PGA112_SCK) | _BV(PGA112_MOSI) | _BV(PGA112_SS));		// set MOSI, SCK & SS as outputs
	PGA112_DDR  &= (uint8_t) ~_BV(PGA112_MISO);											// set MISO as input

#ifndef USE_USART1_SPI
	// disable Power Reduction SPI bit (PRSPI) to power up SPI module
	PRR0 &= (uint8_t) ~_BV(PRSPI);

	// enable SPI; set as Master; set SPI mode 0; set clock rate fOSC/4 (fSPI = 1 MHz @ fOSC = 4 MHz)
	SPCR0 = (uint8_t) (_BV(SPE0) | _BV(MSTR0));
#else
	// Master SPI mode; SPI data mode 0
	UCSR1C = (uint8_t) (_BV(UMSEL11) | _BV(UMSEL10));

	// enable receiver and transmitter
	UCSR1B = (uint8_t) (_BV(RXEN1)| _BV(TXEN1));

	// set baud rate (i.e., XCK frequency) to 1 MHz
	UBRR1 = 0;		// 2 MHz
#endif
	
	// initialize variables
	m_Gain = PGA112_G1;
	m_Channel = PGA112_CH0;
}

void pga112_getConfiguration(enum PGA112_CHANNELS * p_channel, enum PGA112_GAINS * p_gain)
{
	uint8_t uintConfig;
	
	// read config from the IC
	pga112_read(&uintConfig);

	// decode channel
	*p_channel = (uint8_t) (0x0F & uintConfig);

	// decode gain
	*p_gain = (uint8_t) (uintConfig >> 4);
}

/**
 * \brief Disables the on-board ADC and its trigger source.
 *
 *	The function stops Timer/Counter1 and clears the ADEN bit in the ADCSRA register.
 */
void pga112_setGain(enum PGA112_GAINS gain)
{	
	if(gain != m_Gain)
	{
		m_Gain = gain;
		pga112_write(PGA112_WRITE_B2);
	}
}

/**
 * \brief Disables the on-board ADC and its trigger source.
 *
 *	The function stops Timer/Counter1 and clears the ADEN bit in the ADCSRA register.
 */
void pga112_setChannel(enum PGA112_CHANNELS channel)
{
	if(channel != m_Channel)
	{
		m_Channel = channel;
		pga112_write(PGA112_WRITE_B2);
	}
}

/**
 * \brief Enables the on-board ADC and its trigger source.
 *
 *	The function resets the read & write pointers of the ADC data buffers, restarts Timer/Counter1 and sets the ADEN bit in the ADCSRA register.
 */
void pga112_sleep(BOOL blnSleep)
{
	if(blnSleep ^ m_blnSleeping)
	{
		m_blnSleeping = blnSleep;
		pga112_write(PGA112_SDN_B2);
	}
}
