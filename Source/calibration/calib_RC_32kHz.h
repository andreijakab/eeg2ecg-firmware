/**
 * \ingroup		grp_calibration  
 *
 * \file		calib_RC_32kHz.h
 * \since		17.09.2010
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 *
 * \brief		Header file of module that calibrates the internal RC oscillator using the TOSC crystal.
 * \remarks		Ported to AVR-GCC for the ATmega164/324/644/1284 from AVR055 source code
 *				(Revision: 1.2; RCSfile: calib_32kHz.c,v; Date: 2006/02/17 12:49:26)
 *
 * $Id: calib_RC_32kHz.h 53 2011-02-10 22:13:03Z andrei-jakab $
 */

#ifndef __CALIB_RC_32KHZ_H__
#define __CALIB_RC_32KHZ_H__

//----------------------------------------------------------------------------------------------------------
//   								Hardware-Related Definitions
//----------------------------------------------------------------------------------------------------------
#define ASYNC_TIMER                        AS2		///< Asynchronous Timer/Counter Enable/Disable Bit
#define NO_PRESCALING                      CS20		///< Asynchronous Timer/Counter Clock Select Bit 0
#define ASYNC_TIMER_CONTROL_REGISTER       TCCR2B	///< Asynchronous Timer/Counter Control Register A
#define ASYNC_TIMER_CONTROL_UPDATE_BUSY    TCR2AUB	///< Asynchronous Timer/Counter Control Register A Update Busy
#define OUTPUT_COMPARE_UPDATE_BUSY         OCR2AUB	///< Asynchronous Timer/Counter Output Compare Register A Update Busy
#define TIMER_UPDATE_BUSY                  TCN2UB	///< Asynchronous Timer/Counter Register Update Busy
#define TIMER                              TCNT2	///< Asynchronous Timer/Counter Count Register
#define OSCCAL_RESOLUTION                  7		///< Length of the OSCCAL register (in bits)
#define LOOP_CYCLES                        7		///< Length of calibRC_Counter() loop (in cycles)
#define TWO_RANGES									///< OSCCAL register is split in two ranges

//----------------------------------------------------------------------------------------------------------
//   								Application-Specific Definitions
//----------------------------------------------------------------------------------------------------------
// User defined values.
// --------------------
// The following values such as calibration method, calibration frequency,
// external watch crystal frequency, and external ticks, can be modified by the user
// to improve accuracy or change desired calibration frequency

// Calibration methods, Binary search WITH Neighborsearch is default method
// Uncomment to use ONE of the two following methods instead:
//#define CALIBRATION_METHOD_BINARY_WITHOUT_NEIGHBOR
//#define CALIBRATION_METHOD_SIMPLE

#define CALIBRATION_FREQUENCY 8000000													///< Frequency to which the RC oscillator should be calibrated. Modify CALIBRATION_FREQUENCY to desired calibration frequency

#define XTAL_FREQUENCY 32768															///< Frequency of the external oscillator. A 32kHz crystal is recommended
#define EXTERNAL_TICKS 100																///< Number of ticks on XTAL. Modify to increase/decrease accuracy

// Fixed calibration values and macros.
// ------------------------------------
// These values are fixed and used by all calibration methods. Not to be modified.
#define RUNNING						0													///< Value which indicates that the calibration is running.
#define FINISHED					1													///< Value which indicates that the calibration is finished.
#define DEFAULT_OSCCAL_MASK			0x00												///< Lower half of OSCCAL register.
#define DEFAULT_OSCCAL_MASK_HIGH	0x80												///< Upper half of OSCCAL register for devices with splitted OSCCAL register.

#define DEFAULT_OSCCAL_HIGH	((1 << (OSCCAL_RESOLUTION - 1)) | DEFAULT_OSCCAL_MASK_HIGH)	///< Value to which the upper half OSCCAL register is initialized at the start of calibration.
#define INITIAL_STEP		( 1 << (OSCCAL_RESOLUTION - 2))								///< Initial step used in the binary search (must be equal to 1/4 of the OSCCAL resolution)
#define DEFAULT_OSCCAL		((1 << (OSCCAL_RESOLUTION - 1)) | DEFAULT_OSCCAL_MASK)		///< Value to which the OSCCAL register is initialized at the start of calibration.

//----------------------------------------------------------------------------------------------------------
//   								Macros
//----------------------------------------------------------------------------------------------------------
// **** Functions implemented as macros to avoid function calls
#define PREPARE_CALIBRATION()	\
		calStep = INITIAL_STEP; \
		calibration = RUNNING;																///< Initialize the calibration step and the calibration state variables.

#define COMPUTE_COUNT_VALUE() \
		countVal = ((EXTERNAL_TICKS*CALIBRATION_FREQUENCY)/(XTAL_FREQUENCY*LOOP_CYCLES));	///< Calculate the exact number of times that the count loop must execute for perfect calibration.

#define SETUP_ASYNC_TIMER()			\
		ASSR |= (uint8_t) (1<<ASYNC_TIMER);	\
		ASYNC_TIMER_CONTROL_REGISTER = (1<<NO_PRESCALING); \
		TIMER = 0x00; \
		while(ASSR & _BV(TIMER_UPDATE_BUSY));												///< Set up timer to be ASYNCHRONOUS from the CPU clock with a second EXTERNAL 32,768kHz CRYSTAL driving it. No prescaling on asynchronous timer.

#define STOP_ASYNC_TIMER()								\
		ASSR &= (uint8_t) ~(1<<ASYNC_TIMER);			\
		TIMER = ASYNC_TIMER_CONTROL_REGISTER = 0x00;	\
		while (ASSR & ((1<<TIMER_UPDATE_BUSY)|(1<<ASYNC_TIMER_CONTROL_UPDATE_BUSY)));		///< Stop timer from being be ASYNCHRONOUS from the CPU clock

#define NOP() __asm__ __volatile__ ("nop" ::)												///< No Operation

#define ABS(var) (((var) < 0) ? -(var) : (var));											///< Absolute value macro.

//----------------------------------------------------------------------------------------------------------
//   								Prototypes
//----------------------------------------------------------------------------------------------------------
void				calibRC_Init(void);
void				calibRC_Calibrate(void);

#endif
