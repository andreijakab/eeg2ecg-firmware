/**
 * \ingroup		grp_drivers 
 *
 * \file		avr_usart-adc.h
 * \since		20.08.2009
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 *
 * \brief		Header file of the AVR USART driver that imitates the ADC's functionality.
 *
 * $Id: avr_usart-adc.h 43 2009-09-29 11:37:23Z andrei-jakab $
 */

#ifndef __AVR_USART_H__
#define __AVR_USART_H__

//----------------------------------------------------------------------------------------------------------
//   								Hardware-Related Definitions
//----------------------------------------------------------------------------------------------------------
// UCSRA - USART Control and Status Register A
#undef RXC
#define RXC			0x80			///< UCSRA: USART Receive Complete
#undef TXC
#define TXC			0x40			///< UCSRA: USART Transmit Complete
#undef UDRE
#define UDRE		0x20			///< UCSRA: USART Data Register Empty
#undef FE
#define FE			0x10			///< UCSRA: Frame Error
#undef DOR
#define DOR			0x08			///< UCSRA: Data OverRun
#undef PE
#define PE			0x04			///< UCSRA: Parity Error
#undef U2X
#define U2X			0x02			///< UCSRA: Double the USART Transmission Speed
#undef MPCM
#define MPCM		0x01			///< UCSRA: Multi-processor Communication Mode

// UCSRB - USART Control and Status Register B
#undef RXCIE
#define RXCIE		0x80			///< UCSRB: RX Complete Interrupt Enable
#undef TXCIE
#define TXCIE		0x40			///< UCSRB: TX Complete Interrupt Enable
#undef UDRIE
#define UDRIE		0x20			///< UCSRB: USART Data Register Empty Interrupt Enable
#undef RXEN
#define RXEN		0x10			///< UCSRB: Receiver Enable
#undef TXEN
#define TXEN		0x08			///< UCSRB: Transmitter Enable
#undef UCSZ2
#define UCSZ2		0x04			///< UCSRB: Character Size
#undef RXB8
#define RXB8		0x02			///< UCSRB: Receive Data Bit 8
#undef TXB8
#define TXB8		0x01			///< UCSRB: Receive Data Bit 8

// UCSRC - USART Control and Status Register C
#undef URSEL
#define URSEL		0x80			///< UCSRC: Register Select (0 = UBRH, 1 = UCSRC)
#undef UMSEL
#define UMSEL		0x40			///< UCSRC: USART Mode Select (0 = asynchronous, 1 = synchronous)
#undef USBS
#define USBS		0x08			///< UCSRC: Stop Bit Select (0 = 1-bit, 1 = 2-bit)
#undef UCPOL
#define UCPOL		0x01			///< UCSRC: Clock Polarity

#define UPM_EVEN	0x20			///< UCSRC: Parity Mode - Enabled, Even Parity
#define UPM_ODD		0x30			///< UCSRC: Parity Mode - Enabled, Odd Parity

#define UCSZ_6		0x02			///< UCSRC: Character Size - 6-bit
#define UCSZ_7		0x04			///< UCSRC: Character Size - 7-bit
#define UCSZ_8		0x06			///< UCSRC: Character Size - 8-bit

//----------------------------------------------------------------------------------------------------------
//   								Macros
//----------------------------------------------------------------------------------------------------------
#define USART_BAUDRATE		125000											///< desired USART baud rate
#define BAUD_PRESCALE_NS	(((F_CPU / (USART_BAUDRATE * 16UL))) - 1)		///< computes the UBRR value based on the desired baud rate (use only for normal speed operation)
#define BAUD_PRESCALE_DS	(((F_CPU / (USART_BAUDRATE * 8UL))) - 1)		///< computes the UBRR value based on the desired baud rate (use only for double speed operation)

//----------------------------------------------------------------------------------------------------------
//   								Prototypes
//----------------------------------------------------------------------------------------------------------
void avr_usart_init(enum BACKGROUND_STATES CurrentState);
void avr_usart_echo(void);
void avr_usart_disable(void);
void avr_usart_enable(void);

#endif
