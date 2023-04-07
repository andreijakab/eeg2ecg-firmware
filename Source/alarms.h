/**
 * \ingroup		grp_functions
 *
 * \file		alarms.h
 * \since		03.08.2009
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 *
 * \brief		Header file of module used to interface with the adapter's LEDs.
 *
 * $Id: alarms.h 53 2011-02-10 22:13:03Z andrei-jakab $
 */

#ifndef __ALARMS_H__
#define __ALARMS_H__

//----------------------------------------------------------------------------------------------------------
//   								Hardware-Related Definitions
//----------------------------------------------------------------------------------------------------------
//
// LEDs
//
#if HW_VERSION == 30
#define LED_PORT		PORTD			///< Data register (i.e. PORTx) of the AVR port to which the LEDs are connected
#define LED_DDR			DDRD			///< Data direction register (i.e. DDRx) of the AVR port to which the LEDs are connected

#define LED_RED			PD6				///< port pin to which the red LED is connected
#define LED_GREEN		PD7				///< port pin to which the green LED is connected
#define LED_BLUE		PD5				///< port pin to which the blue LED is connected
#endif

//----------------------------------------------------------------------------------------------------------
//   								Application-Specific Definitions
//----------------------------------------------------------------------------------------------------------
#define LED_BLINK_INTERVAL_MSEC				250		///< 
#define LED_BLINK_FREQUENCY_HZ				4		///<

//----------------------------------------------------------------------------------------------------------
//   								Macros
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
//   								Enums/Structs
//----------------------------------------------------------------------------------------------------------
/**
 * Enumeration whose members are used as arguments for alarms_set().
 */
enum ALARM_TYPE {AL_STANDBY = 0,	///< set LEDs for the standby state
				 AL_RECORDING,		///< set LEDs for the recording state
				 AL_DISPLAYSCALE,	///< set LEDs for the display scale state
				 AL_CHARGING,		///< set LEDs for the charging state
				 AL_KEY_PRESS,		///< 
				 AL_KEY_HOLD,		///< 
				 AL_MOVEMENT,		///< 
				 AL_FATALERROR		///< set LEDs to indicate that a fatal error has occured
				};

//----------------------------------------------------------------------------------------------------------
//   								Prototypes
//----------------------------------------------------------------------------------------------------------
void	alarms_init(void);
void	alarms_clear(enum ALARM_TYPE alarm);
void	alarms_flash(void);
void	alarms_set(enum ALARM_TYPE alarm);
void	alarms_set_gain(const uint8_t uintGain);

#endif
