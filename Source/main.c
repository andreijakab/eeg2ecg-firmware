/**
 * \ingroup		grp_functions
 *
 * \file		firmware.c
 * \since		06.10.2008
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 * \version		3.0.0
 *
 * \brief		Main code module where the application entry point and the background loop are located.
 *
 * $Id: main.c 55 2011-03-03 13:43:50Z andrei-jakab $
 */

//----------------------------------------------------------------------------------------------------------
//   								Includes
//----------------------------------------------------------------------------------------------------------
// AVR-LibC headers
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// application headers
#include "globals.h"
#include "main.h"
#include "acc_check.h"
#include "alarms.h"
#include "gain_adjust.h"
#include "calibration/calib_RC_32kHz.h"
#include "drivers/avr_adc.h"
#include "drivers/avr_timer0.h"
#include "drivers/avr_timer1.h"
#include "drivers/avr_timer2.h"
#include "drivers/mma7341lc.h"
#include "drivers/qtouch_key.h"

//----------------------------------------------------------------------------------------------------------
//   								Constants
//----------------------------------------------------------------------------------------------------------
static void (*m_StateMachine[])(void) = {&state_init, &state_standby, &state_recording, &state_displayscale, &state_charging}; ///< array containing a pointer to the function to call in each state of the background loop state machine \n (the order of the pointers must follow the order of the states in the BACKGROUND_STATES enum)

//----------------------------------------------------------------------------------------------------------
//   								Variables
//----------------------------------------------------------------------------------------------------------
static enum BACKGROUND_STATES		m_bkgState;			///< current state of the background loop state machine

// Variables from the ADC driver
extern volatile uint8_t				m_uintEEGSamples[256];
extern volatile uint8_t				m_uintNUnreadSamplesEEG;
extern volatile uint8_t				m_uintEEGSamplesWPtr, m_uintEEGSamplesRPtr;

// variables from the Timer/Counter1 driver
extern volatile BOOL				m_blnTC1_ADC;
extern volatile BOOL				m_blnTC1_StateTransition;

// variables from the Timer/Counter2 driver
extern volatile uint8_t				m_uintTC2_Time2MeasureTouch;
extern volatile BOOL EEMEM			m_blnUSB_ChargingReset;

// variables from the QTouch driver
extern volatile uint16_t			m_uintCurrentTimeTouch_msec;

//----------------------------------------------------------------------------------------------------------
//   								Code
//----------------------------------------------------------------------------------------------------------
/**
 * \brief		Program entry point.
 * 
 * \details		The function contains the infinite background loop with its associated state machine.
 *
 * \return		0
 */
int main(void)
{
	while(1)
	{
		wdt_reset();
		m_StateMachine[m_bkgState]();
	}

	return 0;
}

/**
 * \brief		Code executed during the \b Initialize state.
 */
static void state_init(void)
{
#ifdef DEBUGGING
	dbg_indicate_state(BST_INIT);
#endif

	//
	// MCU Misc. 1
	//
	// check if Watchdog caused system reset
	if(MCUSR & _BV(WDRF))
	{
		// check if Watchdog reset was caused by USB charger
		if(eeprom_read_byte((uint8_t *) &m_blnUSB_ChargingReset))
			m_bkgState = BST_CHARGING;
	}
	eeprom_write_byte((uint8_t *) &m_blnUSB_ChargingReset, FALSE);

	// Clear MCU Status Register
	MCUSR = 0x00;

	// disable Watchdog (NOTE: must be done after the WDRF flag has been cleared)
	wdt_disable();

	// set pull-up resistors disable bit
	MCUCR |= (uint8_t) _BV(PUD);

	//
	// Hardware
	//
	// TI TS5A3159 SPDT Switch
	SWITCH_DDR |= (uint8_t) _BV(SWITCH_IN);
	SWITCH_PORT &= (uint8_t) ~_BV(SWITCH_IN);

	// Maxim MAX1555 Charger
	CHARGER_DDR &= (uint8_t) ~_BV(CHARGER_CHG);
	CHARGER_PORT &= (uint8_t) ~_BV(CHARGER_CHG);

	// Accelerometer
	mma7341lc_init();

	// calibrate internal oscillator
	calibRC_Init();
	calibRC_Calibrate();

	//
	// MCU Misc. 2
	//
	// configure Clock Prescale Register (CLKPR) so that MCU runs at 4MHz
	asm("ldi r16,0x80");
	asm("sts 0x61,r16");
	asm("ldi r16,0x01");
	asm("sts 0x61,r16");

	// configure Timer/Counter0
	avr_tc0_init();

	//
	// Software
	//
	// Alarms
	alarms_init();

	// Gain adjustment module
	ga_init();

	// enable watchdog
	wdt_enable(WDTO_2S);

	// set next state
	if(m_bkgState == BST_INIT)
	{
		m_bkgState = BST_STANDBY;
	}

	DDRC |= (uint8_t) (_BV(PC0) | _BV(PC1));
	//m_bkgState = BST_RECORDING;
}

/**
 * \brief		Code executed during the \b Standby state.
 */
static void state_standby(void)
{
#ifdef DEBUGGING
	dbg_indicate_state(BST_STANDBY);
#endif

	enum STANDBY_STATES state = SST_SLEEP;
	uint16_t uintCount = 0;

	// peripheral init
	cli();
	mma7341lc_setSleepMode(TRUE);
	qtouch_init();
	avr_tc2_init(TMR2_STANDBY);
	wdt_reset();
	sei();

	//pga112_setGain(PGA112_G1);

	while(m_bkgState == BST_STANDBY)
    {
		// kick the dog
		wdt_reset();

		switch(state)
		{
			case SST_SLEEP:
				// wait 1 TOSC1 cycle before entering sleep mode again (allows timer interrupt logic to reset)
				SPIN_ASYNC_TIMER2();
				
				// go to sleep
				SLEEP(SLEEP_MODE_PWR_SAVE);
				
				// kick the dog
				wdt_reset();

				if( m_uintTC2_Time2MeasureTouch )
				{
					// clear flag: it's time to measure touch
					m_uintTC2_Time2MeasureTouch = 0u;

					// check sensor
					if(qtouch_measure(m_uintCurrentTimeTouch_msec))
					{
						//
						// touch was detected
						//
						// stop TC2, initialize TC0 and flash BLUE LED
						alarms_set(AL_KEY_PRESS);
						
						// init touch counter and change mini-state
						uintCount = 0;
						state = SST_COUNT;
					}
				}
			break;

			case SST_COUNT:
				// wait 1 TOSC1 cycle before entering sleep mode again (allows timer interrupt logic to reset)
				SPIN_ASYNC_TIMER2();
				
				// go to sleep
				SLEEP(SLEEP_MODE_PWR_SAVE);

				// kick the dog
				wdt_reset();

				if( m_uintTC2_Time2MeasureTouch )
				{
					// clear flag: it's time to measure touch
					m_uintTC2_Time2MeasureTouch = 0u;

					// check sensor
					if(qtouch_measure(m_uintCurrentTimeTouch_msec))
					{
						uintCount++;

						if(uintCount == (STANDBY_TOUCH_LENGTH_MIN_MSEC/QTOUCH_MEAS_PERIOD_MSEC))
						{
							//
							// touch detected for the mininmum amount of time
							//
							// turn on blue LED 
							alarms_clear(AL_KEY_PRESS);
							alarms_set(AL_KEY_HOLD);

							// go to next mini-state
							state = SST_CHECK;

							// kick the dog
							wdt_reset();
						}
					}
					else
					{
						//
						// no touch detected
						//
						alarms_clear(AL_KEY_PRESS);

						// transition back to sleep mini-state
						state = SST_SLEEP;
						
						// kick the dog
						wdt_reset();
					}
				}
			break;
			
			case SST_CHECK:
				// wait 1 TOSC1 cycle before entering sleep mode again (allows timer interrupt logic to reset)
				SPIN_ASYNC_TIMER2();
				
				// go to sleep
				SLEEP(SLEEP_MODE_PWR_SAVE);
				
				// kick the dog
				wdt_reset();

				if( m_uintTC2_Time2MeasureTouch )
				{
					// clear flag: it's time to measure touch
					m_uintTC2_Time2MeasureTouch = 0u;

					// check sensor
					if(qtouch_measure(m_uintCurrentTimeTouch_msec))
					{
						uintCount++;

						if(uintCount == (STANDBY_TOUCH_LENGTH_MAX_MSEC/QTOUCH_MEAS_PERIOD_MSEC))
						{
							//
							// touch exceeded maximum amount of time allowed
							//
							// transition back to exceeded state
							state = SST_EXCEEDED;

							// kick the dog
							wdt_reset();
						}
					}
					else
					{
						//
						// no touch detected => go to recording state
						//
						alarms_clear(AL_KEY_HOLD);
						m_bkgState = BST_RECORDING;
					}
				}
			break;

			case SST_EXCEEDED:
				// wait 1 TOSC1 cycle before entering sleep mode again (allows timer interrupt logic to reset)
				SPIN_ASYNC_TIMER2();
				
				// go to sleep
				SLEEP(SLEEP_MODE_PWR_SAVE);
				
				// kick the dog
				wdt_reset();

				if( m_uintTC2_Time2MeasureTouch )
				{
					// clear flag: it's time to measure touch
					m_uintTC2_Time2MeasureTouch = 0u;

					// check sensor
					if(!qtouch_measure(m_uintCurrentTimeTouch_msec))
					{
						//
						// no touch detected => go to sleeping state
						//
						alarms_clear(AL_KEY_HOLD);
						state = SST_SLEEP;

						// kick the dog
						wdt_reset();
					}
				}
			break;

			default:
				alarms_set(AL_FATALERROR);
			break;
		}
    }

	avr_tc2_stop();
}

/**
 * \brief		Code executed during the \b Recording state.
 */
static void state_recording(void)
{
#ifdef DEBUGGING
	dbg_indicate_state(BST_RECORDING);
#endif

	uint8_t uintNewSample;

	// peripheral init:
	// - software modules: alarms, gain_adjust
	// - on-board: ADC, Timer/Counter0, Timer/Counter2
	// - external: accelerometer
	cli();
	mma7341lc_setSleepMode(FALSE);
	alarms_set(AL_RECORDING);
	qtouch_init();
	avr_adc_init();
	avr_tc1_init(TMR1_RECORDING, RECORDING_STATE_DURATION_SEC);
	avr_tc2_init(TMR2_RECORDING);
	ga_reset();
	qtouch_statemachine_init(RECORDING_TOUCH_LENGTH_MIN_MSEC, RECORDING_TOUCH_LENGTH_MAX_MSEC);
	sei();

	while(m_bkgState == BST_RECORDING)
	{
		//
		// start ADC conversion
		//
		if(m_blnTC1_ADC == TRUE)
		{
			m_blnTC1_ADC = FALSE;
			
			// enabled ADC and go into ADC noise canceling sleep mode (conversion automatically started by sleep mode)
			ENABLE_ADC;
			SLEEP(SLEEP_MODE_ADC);

			// kick the dog
			wdt_reset();
		}
		
		//
		// deal with the ADC results
		//
		while(m_uintNUnreadSamplesEEG > 0)
		{
			// get next new data sample & increase read pointer
			uintNewSample = m_uintEEGSamples[m_uintEEGSamplesRPtr++];
					
			// send new sample to module that adjusts the adapter's gain
			if(ga_newsample(uintNewSample))
				m_bkgState = BST_DISPLAYSCALE;

			// increase sample counters & decrease unread sample counter
			m_uintNUnreadSamplesEEG--;

			// kick the dog
			wdt_reset();
		}
		
		//
		// check Timer/Counter2
		//
		if(m_blnTC1_StateTransition)
		{
			m_blnTC1_StateTransition = FALSE;
			m_bkgState = BST_DISPLAYSCALE;
		}
		
		//
		// check QTouch button
		//
		if( m_uintTC2_Time2MeasureTouch )
		{
			// clear flag: it's time to measure touch
			m_uintTC2_Time2MeasureTouch = 0u;

			// check sensor
			if(qtouch_statemachine_measurement(qtouch_measure(m_uintCurrentTimeTouch_msec)))
			{
				m_bkgState = BST_STANDBY;
				break;
			}
		}

		// kick the dog
		wdt_reset();
	}

	alarms_clear(AL_RECORDING);
}

/**
 * \brief		Code executed during the \b Display \b Scale state.
 */
static void state_displayscale(void)
{
#ifdef DEBUGGING
	dbg_indicate_state(BST_DISPLAYSCALE);
#endif

	// set SPDT switch to the NO setting
	SWITCH_PORT |= (uint8_t) _BV(SWITCH_IN);

	// peripheral init: on-board Timer/Counter1 & Timer/Counter0 for display scaling mode
	cli();
	alarms_set(AL_DISPLAYSCALE);
	avr_tc0_init();													// outputs PWM signal
	avr_tc1_init(TMR1_DISPSCALE, DISPLAY_SCALE_STATE_DURATION_SEC);	// triggers transition back to recording state
	avr_tc2_init(TMR2_DISPSCALE);
	wdt_reset();
	sei();
	
	// set PGA gain according to the current gain stage
	ga_enterDisplayScale();
	
	// wait until DISPLAY_SCALE_STATE_DURATION_SEC elapse
	while(!m_blnTC1_StateTransition)
	{
		// enable Idle sleep mode, set sleep enable bit & catch some zzz's
		// (sleep bit cleared upon waking up)
		SLEEP(SLEEP_MODE_IDLE);

		// kick the dog
		wdt_reset();
	}

	// reset PGA to previous setting
	ga_exitDisplayScale();

	// set SPDT switch to the NC setting
	SWITCH_PORT &= (uint8_t) ~_BV(SWITCH_IN);
	
	// stop on-board Timer/Counter1 & Timer/Counter0
	avr_tc0_stop();
	alarms_clear(AL_DISPLAYSCALE);

	// set next state
	m_bkgState = BST_RECORDING;
}

/**
 * \brief		Code executed during the \b Charging state.
 */
static void state_charging(void)
{
#ifdef DEBUGGING
	dbg_indicate_state(BST_CHARGING);
#endif

	// peripheral init
	cli();
	alarms_set(AL_CHARGING);
	avr_tc2_init(TMR2_CHARGING);
	wdt_reset();
	sei();
	
	while(~CHARGER_PIN & _BV(CHARGER_CHG))
	{
		// enable Idle sleep mode, set sleep enable bit & catch some zzz's
		// (sleep bit cleared upon waking up)
		SLEEP(SLEEP_MODE_IDLE);

		// kick the dog
		wdt_reset();
	}
	
	alarms_clear(AL_CHARGING);

	// set next state
	m_bkgState = BST_STANDBY;
}

static void dbg_indicate_state(enum BACKGROUND_STATES state)
{
	DDRB	|= (uint8_t) (_BV(PB5) | _BV(PB6) | _BV(PB7));
	PORTB	&= (uint8_t) ~(_BV(PB5) | _BV(PB6) | _BV(PB7));

	switch(state)
	{
		case BST_INIT:
			PORTB |= (uint8_t) _BV(PB7);
		break;

		case BST_STANDBY:
			PORTB |= (uint8_t) _BV(PB6);
		break;

		case BST_RECORDING:
			PORTB |= (uint8_t) (_BV(PB6) | _BV(PB7));
		break;

		case BST_DISPLAYSCALE:
			PORTB |= (uint8_t) _BV(PB5);
		break;

		case BST_CHARGING:
			PORTB |= (uint8_t) (_BV(PB5) | _BV(PB7));
		break;
	}
}
