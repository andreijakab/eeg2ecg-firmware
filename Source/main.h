/**
 * \ingroup		grp_functions
 *
 * \file		main.h
 * \since		06.10.2008
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 *
 * \brief		Header file of main code module.
 *
 * $Id: main.h 55 2011-03-03 13:43:50Z andrei-jakab $
 */

#ifndef __MAIN_H__
#define __MAIN_H__

//----------------------------------------------------------------------------------------------------------
//   								Hardware-Related Definitions
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
//   								Application-Specific Definitions
//----------------------------------------------------------------------------------------------------------
#define STANDBY_SLEEP_LENGTH_SEC			1		///< time interval between activity checks while in the Standby state (in sec)
#define STANDBY_TOUCH_LENGTH_MIN_MSEC		5000	///< min. amount of time that touch must be detected in the Standby state (in msec)
#define STANDBY_TOUCH_LENGTH_MAX_MSEC		6000	///< max. amount of time that touch must be detected in the Standby state (in msec)

#define RECORDING_STATE_DURATION_SEC		120		///< time interval at which main state machine transitions out of he Recording state (in sec)
#define RECORDING_TOUCH_LENGTH_MIN_MSEC		1000	///< min. amount of time that touch must be detected in the Recording state (in msec)
#define RECORDING_TOUCH_LENGTH_MAX_MSEC		2000	///< max. amount of time that touch must be detected in the Recording state (in msec)

#define DISPLAY_SCALE_STATE_DURATION_SEC	5		///< time interval at which main state machine transitions out of the Display Scale state (in sec)

//----------------------------------------------------------------------------------------------------------
//   								Enums/Structs
//----------------------------------------------------------------------------------------------------------
/**
 * States of the state maching that continually runs in the background loop.
 */
enum BACKGROUND_STATES {BST_INIT = 0,		///< initializing
						BST_STANDBY,		///< standby
						BST_RECORDING,		///< recording 
						BST_DISPLAYSCALE,	///< display scale
						BST_CHARGING		///< battery charging
					   };

/**
 * 
 */
enum STANDBY_STATES {SST_SLEEP = 0,			///< 
					 SST_COUNT,				///< 
					 SST_CHECK,				///< 
					 SST_EXCEEDED			///< 
					};

/**
 * 
 */
enum EXECUTION_FLAGS {EF_STAY = 0x01,		///< 
					  EF_CHANGE = 0x02,		///< 
					  EF_CHARGING = 0x04	///< 
					 };

//----------------------------------------------------------------------------------------------------------
//   								Prototypes
//----------------------------------------------------------------------------------------------------------
int				main(void);
static void		dbg_indicate_state(enum BACKGROUND_STATES state);
static void		state_init(void);
static void		state_standby(void);
static void		state_recording(void);
static void		state_displayscale(void);
static void		state_charging(void);

#endif
