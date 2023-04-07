/**
 * \ingroup		grp_functions
 *
 * \file		globals.h
 * \since		06.10.2008
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 *
 * \brief		Header file included in all of the project's code modules.
 *
 * $Id: globals.h 55 2011-03-03 13:43:50Z andrei-jakab $
 */

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#define DEBUGGING

//----------------------------------------------------------------------------------------------------------
//   								Doxygen Definitions
//----------------------------------------------------------------------------------------------------------
/**
 * \defgroup grp_drivers Drivers
 */
/**
 * \defgroup grp_calibration Calibration
 */
/**
 * \defgroup grp_functions Functionality
 */

//----------------------------------------------------------------------------------------------------------
//   								Application-Specific Definitions
//----------------------------------------------------------------------------------------------------------
#define HW_VERSION					30U								///< two digit decimal number \f$D_{1}D_{0}\f$ indicating the hardware version where: \n - \f$D_{1}\f$: major hardware revision \n - \f$D_{0}\f$: minor hardware revision

#ifndef F_CPU
#define F_CPU						4000000UL						///< defines MCU clock as 4MHz
#endif

typedef unsigned char BOOL;											///< defines boolean data type
#define TRUE  1														///< defines the boolean value TRUE 
#define FALSE 0														///< defines the boolean value FALSE

// QTouch
#define QTOUCH_MEAS_PERIOD_MSEC		100								///< time interval at which the state of the QTouch key is checked (in msec)
#define QTOUCH_MEAS_FREQUENCY_HZ	10


//----------------------------------------------------------------------------------------------------------
//   								Macros
//----------------------------------------------------------------------------------------------------------
#define SLEEP(mode)								\
		do										\
		{										\
			set_sleep_mode(mode);				\
			sleep_enable();						\
			sei();								\
			sleep_cpu();						\
												\
			sleep_disable();					\
		} while(0)													///< macro used to enter the sleep mode specified by the \a mode parameter (the values that can be used for \a mode are the same as the ones used for the set_sleep_mode() macro)

//----------------------------------------------------------------------------------------------------------
//   								Hardware-Related Definitions
//----------------------------------------------------------------------------------------------------------
#if (HW_VERSION == 30)
//
// TI TS5A3159 SPDT Switch
//
#define SWITCH_PORT		PORTD										///< Data register (i.e. PORTx) of the AVR port to which the switch's control lines are connected
#define SWITCH_DDR		DDRD										///< Data direction register (i.e. DDRx) of the AVR port to which the switch's control lines are connected

#define SWITCH_IN		PD0											///< port pin to which the switch's IN control line is connected
#endif

#if (HW_VERSION == 30)
//
// Maxim MAX1555 Li+ Battery Chargers
//
#define CHARGER_PORT	PORTB										///< Data register (i.e. PORTx) of the AVR port to which the charger's control lines are connected
#define CHARGER_DDR		DDRB										///< Data direction register (i.e. DDRx) of the AVR port to which the charger's control lines are connected
#define CHARGER_PIN		PINB										///< Pin register (i.e. PINx) of the AVR port to which the charger's control lines are connected

#define CHARGER_CHG		PB3											///< port pin to which the charger's CHG control line is connected
#endif

#endif
