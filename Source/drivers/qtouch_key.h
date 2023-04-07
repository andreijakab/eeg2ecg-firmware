/**
 * \ingroup		grp_drivers 
 *
 * \file		qtouch_key.h
 * \since		19.07.2010
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 *
 * \brief		Header file of the AVR driver for one QTouch button.
 *
 * $Id: qtouch_key.h 53 2011-02-10 22:13:03Z andrei-jakab $
 */

#ifndef __QTOUCH_KEY_H__
#define __QTOUCH_KEY_H__

//----------------------------------------------------------------------------------------------------------
//   								Hardware-Related Definitions
//----------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------
//   								Application-Specific Definitions
//----------------------------------------------------------------------------------------------------------
//
// QTouch Library
//
#ifndef _QTOUCH_
#define _QTOUCH_
#endif

#ifndef SNS1
#define	SNS1					B
#endif

#ifndef SNSK1
#define	SNSK1					B
#endif

#ifndef _SNS1_SNSK1_SAME_PORT_
#define	_SNS1_SNSK1_SAME_PORT_
#endif

#ifndef QT_NUM_CHANNELS
#define	QT_NUM_CHANNELS			4
#endif

#ifndef QT_DELAY_CYCLES
#define	QT_DELAY_CYCLES			10
#endif

#ifndef _POWER_OPTIMIZATION_
#define _POWER_OPTIMIZATION_	0
#endif

#ifndef QTOUCH_STUDIO_MASKS
#define QTOUCH_STUDIO_MASKS		1
#endif

#ifndef NUMBER_OF_PORTS
#define NUMBER_OF_PORTS			1
#endif

#ifdef _DEBUG
#define EVK2080A				0u
#define BOARD_ID				EVK2080A

// Debug related ports & pins
#define DBG_DATA_PORT   		B
#define DBG_CLK_PORT		    B
#define DBG_DETECT_PORT			B
#define DBG_DATA_BIT    		1
#define DBG_CLK_BIT     		0
#define DBG_DETECT_PIN			5
#endif

//----------------------------------------------------------------------------------------------------------
//   								Macros
//----------------------------------------------------------------------------------------------------------
#define GET_SENSOR_STATE(SENSOR_NUMBER)		(qt_measure_data.qt_touch_status.sensor_states[(SENSOR_NUMBER/8)] & (1 << (SENSOR_NUMBER % 8)))

#ifdef _DEBUG
#define __delay_cycles(n)					__builtin_avr_delay_cycles(n)
#define SENSOR_CONFIG( from, to, type ) 	( ( to << 5 ) | ( from << 2 ) | type )	///< fill out sensor config info for reporting in debug data
#endif

//----------------------------------------------------------------------------------------------------------
//   								Enums/Structs
//----------------------------------------------------------------------------------------------------------
#ifdef _DEBUG
/*  board info returned in debug data */
typedef struct tag_board_info_t
{

    uint8_t qt_max_num_rotors_sliders_board_id;	/*  board ID plus max num rotors/sliders
                                                *   bits 0..3: board ID
                                                *   bits 4..7: the max number of rotors or sliders supported by the library
                                                */
    uint8_t qt_num_channels;	/*  the number of touch channels supported by the library */

} board_info_t;
#endif

/**
 * 
 */
enum QTOUCH_KEY_DEBOUNCE_STATES {QKDS_OFF = 0,			///< 
								 QKDS_ON_COUNTING1,		///< 
								 QKDS_ON_COUNTING2,		///< 
								 QKDS_ON_DISCARD		///< 
								};
//----------------------------------------------------------------------------------------------------------
//   								Prototypes
//----------------------------------------------------------------------------------------------------------
void qtouch_init( void );
BOOL qtouch_measure( uint16_t uintCurrentTimeTouch_msec );
void qtouch_recalibrate( void );
void qtouch_statemachine_init(uint16_t uintMinTouchLength_msec, uint16_t uintMaxTouchLength_msec);
BOOL qtouch_statemachine_measurement(BOOL blnMeasurement);

#endif
