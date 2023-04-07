/**
 * \ingroup		grp_drivers 
 *
 * \file		pga112.h
 * \since		16.07.2010
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 *
 * \brief		Header file of the AVR driver for the TI PGA112.
 *
 * $Id: pga112.h 52 2010-09-21 13:37:56Z andrei-jakab $
 */

#ifndef __PGA112_H__
#define __PGA112_H__

//----------------------------------------------------------------------------------------------------------
//   								Hardware-Related Definitions
//----------------------------------------------------------------------------------------------------------
#if HW_VERSION == 30
#define USE_USART1_SPI
#endif

#ifndef USE_USART1_SPI

#define PGA112_PORT				PORTB	///< Data register (i.e. PORTx) of the AVR port to which the LCD is connected
#define PGA112_PIN 				PINB	
#define PGA112_DDR 				DDRB	///< Data direction register (i.e. DDRx) of the AVR port to which the LCD is connected

#define PGA112_SS 				PB4 	///< port pin to which the Register Select (RS) line is connected
#define PGA112_SCK				PB7 	///< port pin to which the Read/Write (R/W) line is connected
#define PGA112_MOSI				PB5 	///< port pin to which the Enable (E) line is connected
#define PGA112_MISO				PB6 	///< port pin to which the Data Bus pin 7 (DB7) is connected

#else

#define PGA112_PORT				PORTD	///< Data register (i.e. PORTx) of the AVR port to which the LCD is connected
#define PGA112_PIN 				PIND	
#define PGA112_DDR 				DDRD	///< Data direction register (i.e. DDRx) of the AVR port to which the LCD is connected

#define PGA112_SS 				PD1 	///< port pin to which the Register Select (RS) line is connected
#define PGA112_SCK				PD4 	///< port pin to which the Read/Write (R/W) line is connected
#define PGA112_MOSI				PD3 	///< port pin to which the Enable (E) line is connected
#define PGA112_MISO				PD2 	///< port pin to which the Data Bus pin 7 (DB7) is connected

#endif
//----------------------------------------------------------------------------------------------------------
//   								Application-Specific Definitions
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
//   								Enums/Structs
//----------------------------------------------------------------------------------------------------------
/**
 * States of the state maching that continually runs in the background loop.
 */
enum PGA112_GAINS {PGA112_G1 = 0,
				   PGA112_G2,
				   PGA112_G4,
				   PGA112_G8,
				   PGA112_G16,
				   PGA112_G32,
				   PGA112_G64,
				   PGA112_G128
				  };

/**
 * States of the state maching that continually runs in the background loop.
 */
enum PGA112_CHANNELS {PGA112_CH0 = 0,
					  PGA112_CH1 = 1,
					  PGA112_CAL1 = 12,
					  PGA112_CAL2 = 13,
					  PGA112_CAL3 = 14,
					  PGA112_CAL4 = 15
					 };

//----------------------------------------------------------------------------------------------------------
//   								Prototypes
//----------------------------------------------------------------------------------------------------------
void pga112_init(void);
void pga112_getConfiguration(enum PGA112_CHANNELS * p_channel, enum PGA112_GAINS * p_gain);
void pga112_setGain(enum PGA112_GAINS gain);
void pga112_setChannel(enum PGA112_CHANNELS channel);
void pga112_sleep(BOOL blnSleep);

#endif
