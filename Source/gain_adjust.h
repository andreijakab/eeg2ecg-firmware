/**
 * \ingroup		grp_functions
 *
 * \file		gain_adjust.h
 * \since		29.07.2009
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 *
 * \brief		Header file of module that dynamically adjusts the adepter's gain based on the peak-to-peak amplitude of the EEG signal.
 *
 * $Id: gain_adjust.h 53 2011-02-10 22:13:03Z andrei-jakab $
 */

#ifndef __GAINADJUST_H__
#define __GAINADJUST_H__

//----------------------------------------------------------------------------------------------------------
//   								Definitions
//----------------------------------------------------------------------------------------------------------
//#define GAINADJUST_DATAWINDOW		2604							///< number of EEG samples that must be analyzed prior to gain adjustment \n NOTE 1: must be a power of 2 \n NOTE 2: if value greater or equal than 2^16, must change data type of associated variables to \c int32
#define GAINADJUST_DATAWINDOW		2500							///< number of EEG samples that must be analyzed prior to gain adjustment \n NOTE 1: must be a power of 2 \n NOTE 2: if value greater or equal than 2^16, must change data type of associated variables to \c int32
#define GAINADJUST_NSTAGES			4								///< number of gain levels

//----------------------------------------------------------------------------------------------------------
//   								Enums
//----------------------------------------------------------------------------------------------------------
/**
 * Enumeration whose members are used as return values of ga_newsample()
 */
enum GA_STATE {GA_GATHERANLYZE1 = 0,	///< gain level has not changed
			   GA_GATHERANLYZE2,		///<
			   GA_WAIT					///<
			  };

//----------------------------------------------------------------------------------------------------------
//   								Prototypes
//----------------------------------------------------------------------------------------------------------
void			ga_init(void);
void			ga_reset(void);
BOOL			ga_newsample(const uint8_t uintNewSample);
void			ga_enterDisplayScale(void);
void			ga_exitDisplayScale(void);

#endif
