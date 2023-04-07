/**
 * \ingroup		grp_drivers
 *
 * \file		qtouch_key.c
 * \since		19.07.2010
 * \author		Andrei Jakab (andrei.jakab@tut.fi)
 * \version		1.0.0
 *
 * \brief		AVR driver for one QTouch button.
 *
 * $Id: qtouch_key.c 53 2011-02-10 22:13:03Z andrei-jakab $
 */

//----------------------------------------------------------------------------------------------------------
//   								Includes
//----------------------------------------------------------------------------------------------------------
// AVR-LibC headers
#include <avr/io.h>
#include <avr/interrupt.h>

// application headers
#include "../globals.h"
#include "../alarms.h"
#include "qtouch_key.h"

// QTouch headers
#include "touch_api.h"

//----------------------------------------------------------------------------------------------------------
//   								Constants
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
//   								Module Variables
//----------------------------------------------------------------------------------------------------------
//uint16_t								qt_measurement_period_msec = 25u;		///< timer period (msec)
extern qt_touch_lib_config_data_t		qt_config_data;							///< onfiguration data structure parameters if needs to be changed will be changed in the qt_set_parameters function
extern qt_touch_lib_measure_data_t		qt_measure_data;						///< touch output - measurement data
extern int16_t							qt_get_sensor_delta( uint8_t sensor);	///< get sensor delta values

volatile uint16_t						m_uintCurrentTimeTouch_msec = 0u;		///< current time, set by timer ISRs

static enum QTOUCH_KEY_DEBOUNCE_STATES	m_qkdsStateMachine_State;
static uint16_t							m_uintStateMachine_Count;
static uint16_t							m_uintStateMachine_MinTouchLength_msec;	
static uint16_t							m_uintStateMachine_MaxTouchLength_msec;

#ifdef QTOUCH_STUDIO_MASKS
extern TOUCH_DATA_T						SNS_array[2][2];
extern TOUCH_DATA_T						SNSK_array[2][2];
#endif

#ifdef _DEBUG
uint8_t									sensor_config[QT_NUM_CHANNELS];
board_info_t							board_info;
#endif
//----------------------------------------------------------------------------------------------------------
//   								Locally-accessible Code
//----------------------------------------------------------------------------------------------------------
/**
 * \brief		Initializes global QTouch configuration data.
 * 
 * \details		This will fill the default threshold values in the configuration data structure.
 *				User can change the values of these parameters .
 */
static void qt_set_parameters( void )
{
    //  This will be modified by the user to different values   */
    qt_config_data.qt_di              = DEF_QT_DI;
    qt_config_data.qt_neg_drift_rate  = DEF_QT_NEG_DRIFT_RATE;
    qt_config_data.qt_pos_drift_rate  = DEF_QT_POS_DRIFT_RATE;
	qt_config_data.qt_max_on_duration = DEF_QT_MAX_ON_DURATION;
    qt_config_data.qt_drift_hold_time = DEF_QT_DRIFT_HOLD_TIME;
    qt_config_data.qt_recal_threshold = DEF_QT_RECAL_THRESHOLD;
    qt_config_data.qt_pos_recal_delay = DEF_QT_POS_RECAL_DELAY;

}

#ifdef _DEBUG

/**
 * \brief		transmit a byte over the debug interface
 *
 * \param[in]	data	byte to be transmitted
 */
static void qtouch_send_debug_byte( uint8_t data )
{
    uint8_t i;

    for( i = 0u; i < 8u; i++ )
    {
        /*  set data    */
        if( data & 0x80u )
        {
            REG( PORT, DBG_DATA_PORT ) |= (1u << DBG_DATA_BIT);
        }
        else
        {
            REG( PORT, DBG_DATA_PORT ) &= ~(1u << DBG_DATA_BIT);
        }

        /*  data set up time before clock pulse */
        __delay_cycles( 10UL );

        /*  shift next bit up, ready for output */
        data = (uint8_t)( data << 1u );

        /*  clock pulse */
        REG( PORT, DBG_CLK_PORT ) |= (1u << DBG_CLK_BIT );

        __delay_cycles( 10UL );

        REG( PORT, DBG_CLK_PORT ) &= ~(1u << DBG_CLK_BIT );

        /*  delay before next bit   */
        __delay_cycles( 10UL );
    }

    /*  hold data low between bytes */
    REG( PORT, DBG_DATA_PORT ) &= ~(1u << DBG_DATA_BIT );

    /*  inter-byte delay    */
    __delay_cycles( 50UL );
}

/**
 * \brief		Transmit multiple bytes over the debug interface.
 *
 * \param[in]	p		ptr to bytes to transmit
 * \param[in]	count	number of bytes to transmit
 */
static void qtouch_output_to_debugger( uint8_t *p, uint8_t count )
{
    uint8_t i;
    uint8_t data;

    for( i = 0u; i < count; i++ )
    {
        /*  get next byte to transmit   */
        data = *p;

        /*  transmit a byte over the debug interface    */
        qtouch_send_debug_byte( data );

        /*  point to next byte to transmit  */
        p++;
    }
}

/**
 * \brief		Report debug data to host.
 */
static void qtouch_report_debug_data( void )
{
    uint8_t i, zero = 0;
    int16_t sensor_delta;

    qtouch_output_to_debugger( (uint8_t *) &board_info, (uint8_t) sizeof( board_info ) );
    qtouch_output_to_debugger( (uint8_t *) &qt_measure_data.channel_signals[0], (uint8_t) sizeof( qt_measure_data.channel_signals ) );
    qtouch_output_to_debugger( (uint8_t *) &qt_measure_data.channel_references[0], (uint8_t) sizeof( qt_measure_data.channel_references ) );
    
	for( i = 0u; i < QT_NUM_CHANNELS; i++ )
    {
        sensor_delta = qt_get_sensor_delta( i );
        qtouch_output_to_debugger( (uint8_t *) &sensor_delta, sizeof( int16_t ) );
    }
    
	qtouch_output_to_debugger( (uint8_t *) &qt_measure_data.qt_touch_status, (uint8_t) sizeof( qt_measure_data.qt_touch_status.sensor_states ) );
    
	for( i = 0u; i < QT_MAX_NUM_ROTORS_SLIDERS; i++ )
    {
        qtouch_output_to_debugger( (uint8_t *) &qt_measure_data.qt_touch_status.rotor_slider_values[i], sizeof( uint8_t ) );
		qtouch_output_to_debugger( (uint8_t *) &zero, sizeof( uint8_t ) );
    }
    
    qtouch_output_to_debugger( (uint8_t *) &sensor_config[0], (uint8_t) sizeof( sensor_config ) );
}

/**
 * \brief		Initialize debug interface
 */
static void qtouch_init_debug_if( void )
{
    // init port pins
    REG( DDR, DBG_DATA_PORT ) |= (1u << DBG_DATA_BIT );
    REG( DDR, DBG_CLK_PORT ) |= (1u << DBG_CLK_BIT );
	REG( DDR, DBG_DETECT_PORT ) |= (1u << DBG_DETECT_PIN );

	// configure the debug data reported to the PC
    board_info.qt_max_num_rotors_sliders_board_id = ( ( QT_MAX_NUM_ROTORS_SLIDERS << 4 ) | BOARD_ID );
    board_info.qt_num_channels = QT_NUM_CHANNELS;
    sensor_config[0] = SENSOR_CONFIG( CHANNEL_0, CHANNEL_0, SENSOR_TYPE_KEY );
}
#endif

//----------------------------------------------------------------------------------------------------------
//   								Globally-accessible Code
//----------------------------------------------------------------------------------------------------------
void qtouch_init(void)
{/*
#ifdef QTOUCH_STUDIO_MASKS
	SNS_array[0][0] = 0x01;
	SNS_array[0][1] = 0x00;
	SNS_array[1][0] = 0x00;
	SNS_array[1][1] = 0x00;

	SNSK_array[0][0] = 0x02;
	SNSK_array[0][1] = 0x00;
	SNSK_array[1][0] = 0x00;
	SNSK_array[1][1] = 0x00;
#endif*/

#ifdef QTOUCH_STUDIO_MASKS
	SNS_array[0][0]= 0x1;
	SNS_array[0][1]= 0x0;
	SNS_array[1][0]= 0x0;
	SNS_array[1][1]= 0x0;

	SNSK_array[0][0]= 0x2;
	SNSK_array[0][1]= 0x0;
	SNSK_array[1][0]= 0x0;
	SNSK_array[1][1]= 0x0;
#endif


	// enable sensors 0 and assign it to adjacent key suppression group 1; detect threshold = 10 and hysteresis = 6.25%
	//qt_enable_key( CHANNEL_0, AKS_GROUP_1, 10u, HYST_6_25 );

	// enable sensors 0 and assign it to adjacent key suppression group 1; detect threshold = 75 and hysteresis = 6.25%
	qt_enable_key( CHANNEL_0, AKS_GROUP_1, 75u, HYST_6_25 );

	// calibrate configured channels and prepare sensors for capacitive measurement
	qt_init_sensing();

	// set global parameters like recalibration threshold, Max_On_Duration etc.
	qt_set_parameters( );

    // pass address of user function to be called after the library has made capacitive measurements, but before it has processed them
    qt_filter_callback = 0;

	m_uintCurrentTimeTouch_msec = 0u;

#ifdef _DEBUG
    // Initialize debug interface
    qtouch_init_debug_if ();
#endif
}

BOOL qtouch_measure( uint16_t uintCurrentTimeTouch_msec )
{
	// status flags to indicate the re-burst for library
    uint16_t status_flag = 0u;
    uint16_t burst_flag = 0u;
	
	// measure sensor(s)
	do
	{
		// disable pull-ups for all pins
		//MCUCR |= (uint8_t) _BV(PUD);

		// one time measure touch sensors
		status_flag = qt_measure_sensors( uintCurrentTimeTouch_msec );
		burst_flag = status_flag & QTLIB_BURST_AGAIN;

		// enable pull-ups for all pins
		//MCUCR &= (uint8_t) ~(_BV(PUD));

		/* Time-critical host application code goes here */
#ifdef _DEBUG
		qtouch_report_debug_data();
#endif
	}while (burst_flag) ;

	// check & return key state
	if(GET_SENSOR_STATE(0) != 0)
	{
#ifdef _DEBUG
		// set detect pin
		REG( PORT, DBG_DETECT_PORT ) |= _BV(DBG_DETECT_PIN);
#endif
		return TRUE;
	}
	else
	{
#ifdef _DEBUG
		// clear detect pin
		REG( PORT, DBG_DETECT_PORT ) &= ~_BV(DBG_DETECT_PIN);
#endif
		return FALSE;
	}
}

void qtouch_recalibrate ( void )
{
	qt_calibrate_sensing();
}

void qtouch_statemachine_init(uint16_t uintMinTouchLength_msec, uint16_t uintMaxTouchLength_msec)
{
	m_uintStateMachine_MinTouchLength_msec = uintMinTouchLength_msec;
	m_uintStateMachine_MaxTouchLength_msec = uintMaxTouchLength_msec;

	m_uintStateMachine_Count = 0;
	m_qkdsStateMachine_State = QKDS_OFF;
}

BOOL qtouch_statemachine_measurement(BOOL blnMeasurement)
{
	BOOL blnResult = FALSE;

	switch(m_qkdsStateMachine_State)
	{
		case QKDS_OFF:
			if(blnMeasurement)
			{
				m_uintStateMachine_Count = 0;
				m_qkdsStateMachine_State = QKDS_ON_COUNTING1;
				alarms_set(AL_KEY_PRESS);
			}
		break;

		case QKDS_ON_COUNTING1:
			if(blnMeasurement)
			{
				if(++m_uintStateMachine_Count == (m_uintStateMachine_MinTouchLength_msec/QTOUCH_MEAS_PERIOD_MSEC))
				{
					m_qkdsStateMachine_State = QKDS_ON_COUNTING2;

					// touch detected for minimum amount of time
					alarms_clear(AL_KEY_PRESS);
					alarms_set(AL_KEY_HOLD);
				}
			}
			else
			{
				m_qkdsStateMachine_State = QKDS_OFF;
				alarms_clear(AL_KEY_PRESS);
			}
		break;

		case QKDS_ON_COUNTING2:
			if(blnMeasurement)
			{
				if(++m_uintStateMachine_Count >= (m_uintStateMachine_MaxTouchLength_msec/QTOUCH_MEAS_PERIOD_MSEC))
				{
					m_qkdsStateMachine_State = QKDS_ON_DISCARD;
				}
			}
			else
			{
				blnResult = TRUE;
				m_qkdsStateMachine_State = QKDS_OFF;
				alarms_clear(AL_KEY_HOLD);
			}
		break;

		case QKDS_ON_DISCARD:
			if(!blnMeasurement)
			{
				m_qkdsStateMachine_State = QKDS_OFF;
				alarms_clear(AL_KEY_HOLD);
			}
		break;
	}

	return blnResult;
}
