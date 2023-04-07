/**
 * \ingroup		grp_drivers 
 *
 * \file		avr_timer2.h
 * \since		05.09.2010
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 *
 * \brief		Header file of the AVR 8-bit Timer/Counter0 driver for the ATmega164/324/644/1284 family.
 *
 * $Id: avr_timer2.h 53 2011-02-10 22:13:03Z andrei-jakab $
 */

#ifndef __AVR_TIMER2_H__
#define __AVR_TIMER2_H__

//----------------------------------------------------------------------------------------------------------
//   								Hardware-Related Definitions
//----------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------
//   								Application-Specific Definitions
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
//   								Macros
//----------------------------------------------------------------------------------------------------------
/*#define STOP_ASYNC_TIMER2()					\
		ASSR &= (uint8_t) ~(_BV(AS2));		\
		TCNT2 = TCCR2B = 0x00;				\
		while (ASSR & (_BV(TCN2UB) | _BV(TCR2BUB)));	///< Stop asynchrnous timer2*/

#define SPIN_ASYNC_TIMER2()						\
		do										\
		{										\
			TCNT2 = 0x00;						\
			while(ASSR & _BV(TCN2UB));			\
		} while(0)							///< macro used to allow the interrupt logic to reset (needs one TOSC1 cycle)

//----------------------------------------------------------------------------------------------------------
//   								Enums/Structs
//----------------------------------------------------------------------------------------------------------
/**
 * 
 */
enum TIMER2_MODE {TMR2_OFF = 0x00,			///< 
				  TMR2_STANDBY = 0x01,		///< Wake up from Power Save while in standby state
				  TMR2_RECORDING = 0x02,	///< 
				  TMR2_DISPSCALE = 0x03,	///< 
				  TMR2_CHARGING = 0x04		///<
				 };

//----------------------------------------------------------------------------------------------------------
//   								Prototypes
//----------------------------------------------------------------------------------------------------------
void avr_tc2_init(enum TIMER2_MODE mode);
void avr_tc2_stop(void);

#endif
