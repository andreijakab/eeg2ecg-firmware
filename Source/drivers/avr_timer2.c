/**
 * \ingroup		grp_drivers 
 *
 * \file		avr_timer2.c
 * \since		05.09.2009
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 * \version		1.0.0
 *
 * \brief		AVR 8-bit Timer/Counter0 driver for the ATmega164/324/644/1284 family.
 *
 * $Id: avr_timer2.c 54 2011-02-11 19:07:47Z andrei-jakab $
 */

//----------------------------------------------------------------------------------------------------------
//   								Includes
//----------------------------------------------------------------------------------------------------------
// AVR-LibC headers
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// application headers
#include "../globals.h"
#include "../alarms.h"
#include "avr_timer2.h"

//----------------------------------------------------------------------------------------------------------
//   								Variables
//----------------------------------------------------------------------------------------------------------
volatile uint8_t			m_uintTC2_Time2MeasureTouch;		///< indicates whether the interval specified by \a m_uintAsyncInterval has elapsed
volatile BOOL EEMEM			m_blnUSB_ChargingReset;				///<

static enum TIMER2_MODE		m_Mode = TMR2_OFF;					///< 
static volatile uint8_t		m_uintISRCount_OCR2A_LEDs;			///< number of times the Timer/Counter2 overflow ISR has been called
static volatile uint8_t		m_uintISRInterval_OCR2A_LEDs;		///< number of times the Timer/Counter2 overflow ISR has been called
			
extern volatile uint16_t	m_uintCurrentTimeTouch_msec;

//----------------------------------------------------------------------------------------------------------
//   								Code
//----------------------------------------------------------------------------------------------------------
/**
 * \brief		Initializes the required driver variables and hardware registers for the asynchronous operation of AVR Timer/Counter2.
 *
 * \details		The function follows the procedure detailed on pg. 151 of the ATmega32L datasheet in order to initialize timer/counter2
 *				in asynchronous mode. The prescaler is set to 151 so that overflow occurs every second with a 32.768kHz TOSC crystal.
 *
 * \param[in]	interval	number of seconds that must elapse in order to set \a m_blnTC2AsyncIntervalExpired to TRUE
 */
void avr_tc2_init(enum TIMER2_MODE mode)
{
	// set mode
	m_Mode = mode;
	
	// reset variables
	m_uintTC2_Time2MeasureTouch = 0u;
	m_uintISRCount_OCR2A_LEDs = 0;

	switch(m_Mode)
	{
		case TMR2_STANDBY:	
		case TMR2_RECORDING:
		case TMR2_DISPSCALE:
			//
			// enable asynchronous operation of Timer2
			// (procedure descibed on pg. 151)
			//
			// disable the Timer/Counter2 interrupts by clearing OCIE2A, OCIE2B and TOIE2
			TIMSK2 &= (uint8_t) ~(_BV(TOIE2) | _BV(OCIE2B) | _BV(OCIE2A));

			// set Timer/Counter2 to be asynchronous from the CPU clock
			ASSR |= (uint8_t) _BV(AS2);

			// set timer to CTC mode
			TCCR2A = (uint8_t) _BV(WGM21);
			
			//clock prescaler 128
			TCCR2B = (uint8_t) (_BV(CS22) | _BV(CS20));

			// initialize count
			OCR2A = (uint8_t) ((uint16_t) 32768 / ((uint16_t) 128 * QTOUCH_MEAS_FREQUENCY_HZ)) - 1;
			TCNT2 = 0x00;

			// wait for TCCR2, TCNT2  and OCR2 to be written
			while(ASSR & (_BV(TCN2UB) | _BV(OCR2AUB) | _BV(OCR2BUB) | _BV(TCR2AUB) | _BV(TCR2BUB)));

			// wait 1 second for oscillator to stabilize
			//_delay_ms(1000);

			// clear the Timer/Counter2 Interrupt Flags
			TIFR2 |= (uint8_t) _BV(OCF2A);

			// enable Timer/Counter2 Output Compare A interrupt
			TIMSK2 |= (uint8_t) _BV(OCIE2A);

			// set the ISR intervals to 0 so that the ISR actions are performed every time the ISR is called
			m_uintISRInterval_OCR2A_LEDs = (uint8_t) ((QTOUCH_MEAS_FREQUENCY_HZ/LED_BLINK_FREQUENCY_HZ) - 1);
		break;
		
		/*case TMR2_RECORDING:
		case TMR2_DISPSCALE:
			// CTC mode
			TCCR2A = (uint8_t) _BV(WGM21);

			// clkT2S/128 clock source
			TCCR2B = (uint8_t) (_BV(CS22) | _BV(CS21) | _BV(CS20));

			// clear the Timer/Counter2 Interrupt Flags
			TIFR2 |= (uint8_t) (_BV(OCF2A));

			// set Timer/Counter2 Overflow Interrupt Enable and Output Compare A
			TIMSK2 |= (uint8_t) (_BV(OCIE2A));

			// trigger OC interrupts with a frequency of 4MHz/(128*(1+124)) = 250 Hz
			OCR2A = (uint8_t) ((uint32_t) 4000000 / ((uint32_t) 1024 * QTOUCH_MEAS_FREQUENCY_HZ)) - 1;

			// 
			m_uintISRInterval_OCR2A_LEDs = (uint8_t) ((QTOUCH_MEAS_FREQUENCY_HZ/LED_BLINK_FREQUENCY_HZ) - 1);
		break;*/

		case TMR2_CHARGING:
			// CTC mode
			TCCR2A = (uint8_t) _BV(WGM21);

			// clkT2S/128 clock source
			TCCR2B = (uint8_t) (_BV(CS22) | _BV(CS20));

			// clear the Timer/Counter2 Interrupt Flags
			TIFR2 |= (uint8_t) (_BV(OCF2A));

			// set Timer/Counter2 Overflow Interrupt Enable and Output Compare A
			TIMSK2 |= (uint8_t) (_BV(OCIE2A));

			// trigger OC interrupt with a frequency of 4MHz/(128*(1+124)) = 250 Hz
			OCR2A = 0x7C;

			//
			m_uintISRInterval_OCR2A_LEDs = (uint8_t) ((250/LED_BLINK_FREQUENCY_HZ) - 1);
		break;

		default:
			alarms_set(AL_FATALERROR);
		break;
	}
}

void avr_tc2_stop(void)
{
	switch(m_Mode)
	{
		case TMR2_STANDBY:
		case TMR2_RECORDING:
		case TMR2_DISPSCALE:
			// disable Timer/Counter2 interrupts
			TIMSK2 &= (uint8_t) ~(_BV(OCIE2B) | _BV(OCIE2A) | _BV(TOIE2));

			// disable asynchronous Timer/Counter2 mode
			ASSR &= (uint8_t) ~(_BV(AS2));

			// clear control registers
			TCNT2 = OCR2A = OCR2B = TCCR2A = TCCR2B = 0x00;

			// wait for control registers to be updated
			while(ASSR & (_BV(TCN2UB) | _BV(OCR2AUB) | _BV(OCR2BUB) | _BV(TCR2AUB) | _BV(TCR2BUB)));

			// clear the Timer/Counter2 Interrupt Flags
			TIFR2 |= (uint8_t) (_BV(OCF2B) | _BV(OCF2A) | _BV(TOV2));
		break;

		/*case TMR2_RECORDING:
		case TMR2_DISPSCALE:
			TCCR2B &= (uint8_t) ~(_BV(CS22) | _BV(CS21) | _BV(CS20));
		break;*/

		default:
			alarms_set(AL_FATALERROR);
		break;
	}
}

/**
 * \brief 		Timer/Counter2 Compare Match A ISR
 *
 * \details		
 */
ISR(TIMER2_COMPA_vect)
{
	//
	// QTouch Key
	//
	if((m_Mode == TMR2_STANDBY) || (m_Mode == TMR2_RECORDING))
	{
		//  set flag: it's time to measure touch
		m_uintTC2_Time2MeasureTouch = 1u;

		//  update the current time
		m_uintCurrentTimeTouch_msec += QTOUCH_MEAS_PERIOD_MSEC;
	}

	//
	// USB connection Check
	//
	if(m_Mode != TMR2_CHARGING)
	{
		if(~CHARGER_PIN & _BV(CHARGER_CHG))
		{
			// write to EEPROM
			eeprom_write_byte((uint8_t *) &m_blnUSB_ChargingReset, TRUE);

			// block execution so that the MCU can be reset by watchdog
			while(1);
		}
	}


	//
	// Flash LEDs
	//
	if(m_uintISRCount_OCR2A_LEDs == m_uintISRInterval_OCR2A_LEDs)
	{
		alarms_flash();
		m_uintISRCount_OCR2A_LEDs = 0;
	}
	else
		m_uintISRCount_OCR2A_LEDs++;

}
