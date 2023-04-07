/**
 * \ingroup		grp_drivers 
 *
 * \file		avr_usart.h
 * \since		12.09.2010
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 *
 * \brief		Header file of the AVR USART driver for the ATmega164/324/644/1284 family.
 *
 * $Id: avr_usart.h 49 2010-09-17 13:50:29Z andrei-jakab $
 */

#ifndef __AVR_USART_H__
#define __AVR_USART_H__

//----------------------------------------------------------------------------------------------------------
//   								Hardware-Related Definitions
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
//   								Macros
//----------------------------------------------------------------------------------------------------------
#define USART_BAUDRATE		250000											///< desired UART baud rate
#define BAUD_PRESCALE_NS	(((F_CPU / (USART_BAUDRATE * 16UL))) - 1)		///< computes the UBRR value based on the desired baud rate (use only for normal speed operation)
#define BAUD_PRESCALE_DS	(((F_CPU / (USART_BAUDRATE * 8UL))) - 1)		///< computes the UBRR value based on the desired baud rate (use only for double speed operation)

//----------------------------------------------------------------------------------------------------------
//   								Prototypes
//----------------------------------------------------------------------------------------------------------
void avr_usart0_init(void);
void avr_usart0_disable(void);
void avr_usart0_echo(void);
BOOL avr_usart0_send(uint8_t * puintBuffer, uint8_t uintBufferLength);

#endif
