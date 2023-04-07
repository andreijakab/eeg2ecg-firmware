/**
 * \ingroup		grp_functions
 *
 * \file		acc_check.h
 * \since		11.01.2011
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 *
 * \brief		Header file of acceleration check module.
 *
 * $Id$
 */

#ifndef __ACCCHECK_H__
#define __ACCCHECK_H__

//----------------------------------------------------------------------------------------------------------
//   								Hardware-Related Definitions
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
//   								Application-Specific Definitions
//----------------------------------------------------------------------------------------------------------
#define AC_AVERAGE_INTERVAL_DEC			512			///< number of data samples	to be averaged (in decimal)
#define AC_AVERAGE_INTERVAL_POW_2		9			///< 
#define AC_ACCELERATION_THRESHOLD		

//----------------------------------------------------------------------------------------------------------
//   								Enums/Structs
//----------------------------------------------------------------------------------------------------------
/**
 * 
 */
enum AC_CHANNEL {AC_X = 0,	///< 
				 AC_Y,		///< 
				 AC_Z		///< 
				};

//----------------------------------------------------------------------------------------------------------
//   								Prototypes
//----------------------------------------------------------------------------------------------------------
void ac_init(void);
void ac_new_sample(uint8_t sample, enum AC_CHANNEL channel);

#endif