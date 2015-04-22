/*
 * ultrasonicHIL.c
 *
 *  Created on: 26/02/2015
 *      Author: Soren
 */

#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "inc/hw_timer.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"


#include "pingHIL.h"


#define GPIO_PERIPH_PING0	(SYSCTL_PERIPH_GPIOD)
#define GPIO_BASE_PING0		(GPIO_PORTD_BASE)
#define GPIO_PIN_PING0		(GPIO_PIN_0)
#define GPIO_PERIPH_PING1	(SYSCTL_PERIPH_GPIOD)
#define GPIO_BASE_PING1		(GPIO_PORTD_BASE)
#define GPIO_PIN_PING1		(GPIO_PIN_1)

#define TIMER_PERIPH_PING0	(SYSCTL_PERIPH_WTIMER2)
#define TIMER_BASE_PING0	(WTIMER2_BASE)
#define TIMER_PING0			(TIMER_A)
#define	TIMER_INT_PING0		(INT_WTIMER2A)
#define TIMER_PERIPH_PING1	(SYSCTL_PERIPH_WTIMER2)
#define TIMER_BASE_PING1	(WTIMER2_BASE)
#define TIMER_PING1			(TIMER_B)
#define TIMER_INT_PING1		(INT_WTIMER2B)


#define TIMER_PING0_TR		(TIMER_O_TAR)
#define TIMER_PING0_TV		(TIMER_O_TAV)
#define TIMER_PING1_TR		(TIMER_O_TBR)
#define TIMER_PING1_TV		(TIMER_O_TBV)

#define TIMER_PING0_MATCH		(TIMER_CAPA_MATCH)
#define TIMER_PING0_EVENT		(TIMER_CAPA_EVENT)
#define TIMER_PING0_TIMEOUT		(TIMER_TIMA_TIMEOUT)
#define TIMER_PING0_ONE_SHOT	(TIMER_CFG_A_ONE_SHOT)
#define TIMER_PING0_TIME_UP		(TIMER_CFG_A_CAP_TIME_UP)
#define TIMER_PING1_MATCH		(TIMER_CAPB_MATCH)
#define TIMER_PING1_EVENT		(TIMER_CAPB_EVENT)
#define TIMER_PING1_TIMEOUT		(TIMER_TIMB_TIMEOUT)
#define TIMER_PING1_ONE_SHOT	(TIMER_CFG_B_ONE_SHOT)
#define TIMER_PING1_TIME_UP		(TIMER_CFG_B_CAP_TIME_UP)

#define TIMER_MUX_PING0		(GPIO_PD0_WT2CCP0)
#define TIMER_MUX_PING1		(GPIO_PD1_WT2CCP1)
#define GPIO_MUX_PING0		(GPIO_PD0_WT2CCP0 & 0xffffff00)
#define GPIO_MUX_PING1		(GPIO_PD1_WT2CCP1 & 0xffffff00)

#define INIT_PULSE_N		(400)		// 400 = 5us * 80MHz
#define HOLDOFF_N			(20000)		// 20000 = 250탎 * 80MHz
#define PAUSE_TIME_N		(800000)	// 800000 = 10ms * 80MHz
#define PAUSE_TIME_MIN_N	(20000)		// 20000 = 250us * 80MHz

#define MAX(a,b) \
		({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; })
#define MIN(a,b) \
		({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a < _b ? _a : _b; })

/*
 * States of the ultrasonic driver
 * The currect state indicates what wea re currently doing
 * RECHARGE: In recharge we wait for any echo from the previous pulse to disappear. (tihs is just a delay, nothing happens)
 * INIT_PULSE: Here we wait for the 5us initiating pulse to be sent
 * HLDOFF: Here we simply wait for the ping))) to accept the initpulse and drive the gpio low
 * WAIT_RISING: We wait for the start of the ping))) response
 * WAIT_FALLING: We wait for the end of the ping))) response
 */
typedef enum {RECHARGE, INIT_PULSE, HOLDOFF, WAIT_RISING, WAIT_FALLING} pingState_t;

pingState_t ping0state = RECHARGE;
pingState_t ping1state = RECHARGE;

volatile uint32_t ping0start = 0; // these two are temporary values used to get the tof of the sound wave
volatile uint32_t ping0now = 0;   // ^
volatile uint32_t ping0time = 0;
volatile bool ping0updated = false;

void ping0Int( void ) {
	// clear interrupts.
	ROM_TimerIntClear( TIMER_BASE_PING0, TIMER_PING0_MATCH|TIMER_PING0_EVENT|TIMER_PING0_TIMEOUT );
	switch( ping0state ) {
	case RECHARGE: // We are now done recharging:
		ping0state = INIT_PULSE; // Now we should start again
		// set IO pin as output
		ROM_GPIOPinTypeGPIOOutput( GPIO_BASE_PING0, GPIO_PIN_PING0 );
		ROM_GPIOPinConfigure( GPIO_MUX_PING0 );
		// drive IO pin high
		ROM_GPIOPinWrite( GPIO_BASE_PING0, GPIO_PIN_PING0, GPIO_PIN_PING0 );
		// stop timer
		ROM_TimerDisable( TIMER_BASE_PING0, TIMER_PING0 );
		// set timer for 2-5탎, single shot (start of with 5탎)
		ROM_TimerConfigure( TIMER_BASE_PING0, TIMER_CFG_SPLIT_PAIR | TIMER_PING0_ONE_SHOT );
		ROM_TimerLoadSet( TIMER_BASE_PING0, TIMER_PING0, INIT_PULSE_N );
		// set timer interrupt for end of count
		ROM_TimerIntDisable( TIMER_BASE_PING0, TIMER_PING0_MATCH|TIMER_PING0_EVENT );
		ROM_TimerIntEnable( TIMER_BASE_PING0, TIMER_PING0_TIMEOUT );
		// enable timer
		ROM_TimerEnable( TIMER_BASE_PING0, TIMER_PING0 );
		break;
	case INIT_PULSE: // We are now done sending the init pulse:
		ping0state = HOLDOFF; // Now we should wait some
		// drive low
		ROM_GPIOPinWrite( GPIO_BASE_PING0, GPIO_PIN_PING0, 0 );
		// set IO pin as input
		ROM_GPIOPinTypeGPIOInput( GPIO_BASE_PING0, GPIO_PIN_PING0 );
		ROM_GPIOPinConfigure( GPIO_MUX_PING0 );
		// stop timer
		ROM_TimerDisable( TIMER_BASE_PING0, TIMER_PING0 );
		// set timer for 500탎, single shot (holdoff is ~750탎)
		ROM_TimerConfigure( TIMER_BASE_PING0, TIMER_CFG_SPLIT_PAIR | TIMER_PING0_ONE_SHOT );
		ROM_TimerLoadSet( TIMER_BASE_PING0, TIMER_PING0, HOLDOFF_N );
		// set timer interrupt for end of count
		ROM_TimerIntDisable( TIMER_BASE_PING0, TIMER_PING0_MATCH|TIMER_PING0_EVENT );
		ROM_TimerIntEnable( TIMER_BASE_PING0, TIMER_PING0_TIMEOUT );
		// enable timer
		ROM_TimerEnable( TIMER_BASE_PING0, TIMER_PING0 );
		break;
	case HOLDOFF: // We are now done wating for the ping))) to start
		ping0state = WAIT_RISING; // Now we should wait for the begining of its reply
		// set IO pin as CCP
		ROM_GPIOPinTypeTimer( GPIO_BASE_PING0, GPIO_PIN_PING0 );
		ROM_GPIOPinConfigure( TIMER_MUX_PING0 );
		// stop timer
		ROM_TimerDisable( TIMER_BASE_PING0, TIMER_PING0 );
		// set timer to edge time
		ROM_TimerConfigure( TIMER_BASE_PING0, TIMER_CFG_SPLIT_PAIR | TIMER_PING0_TIME_UP );
		ROM_TimerLoadSet( TIMER_BASE_PING0, TIMER_PING0, 0xffffffff );
		// set count edge to rising
		ROM_TimerControlEvent( TIMER_BASE_PING0, TIMER_PING0, TIMER_EVENT_POS_EDGE );
		// set timer interrupt for event
		ROM_TimerIntDisable( TIMER_BASE_PING0, TIMER_PING0_TIMEOUT|TIMER_PING0_MATCH );
		ROM_TimerIntEnable( TIMER_BASE_PING0, TIMER_PING0_EVENT );
		// enable timer
		ROM_TimerEnable( TIMER_BASE_PING0, TIMER_PING0 );
		break;
	case WAIT_RISING: // We are now done wating for the ping))) to begin its relpy
		ping0state = WAIT_FALLING; // Now we should wait for the end of its reply
		ping0start = HWREG( TIMER_BASE_PING0 + TIMER_PING0_TR );
		ping0now = HWREG( TIMER_BASE_PING0 + TIMER_PING0_TV );
		// IO pin already CCP
		// stop timer
		ROM_TimerDisable( TIMER_BASE_PING0, TIMER_PING0 );
		// set count edge to falling
		ROM_TimerControlEvent( TIMER_BASE_PING0, TIMER_PING0, TIMER_EVENT_NEG_EDGE );
		// enable timer
		ROM_TimerEnable( TIMER_BASE_PING0, TIMER_PING0 );
		// Check if falling edge already occured, if it did: fall trough
		if ( ROM_GPIOPinRead( GPIO_BASE_PING0, GPIO_PIN_PING0 ) )
			break;
	case WAIT_FALLING: // We are now done wating for the ping))) to end its relpy
		ping0state = RECHARGE; // Now we should wait for the end of its reply
		ping0time = HWREG( TIMER_BASE_PING0 + TIMER_PING0_TR ) + ( ping0now - ping0start );
		ping0updated = true;


		int32_t pauseTime = PAUSE_TIME_N - ping0time;
		if ( pauseTime < PAUSE_TIME_MIN_N )
			pauseTime = PAUSE_TIME_MIN_N;

		// set IO pin as output
		ROM_GPIOPinTypeGPIOOutput( GPIO_BASE_PING0, GPIO_PIN_PING0 );
		ROM_GPIOPinConfigure( GPIO_MUX_PING0 );
		// drive IO pin low
		ROM_GPIOPinWrite( GPIO_BASE_PING0, GPIO_PIN_PING0, 0 );
		// stop timer
		ROM_TimerDisable( TIMER_BASE_PING0, TIMER_PING0 );
		// set timer for 200탎, single shot
		ROM_TimerConfigure( TIMER_BASE_PING0, TIMER_PING0_ONE_SHOT );
		ROM_TimerLoadSet( TIMER_BASE_PING0, TIMER_PING0, pauseTime );
		// set timer interrupt for end of count
		ROM_TimerIntDisable( TIMER_BASE_PING0, TIMER_PING0_MATCH|TIMER_PING0_EVENT );
		ROM_TimerIntEnable( TIMER_BASE_PING0, TIMER_PING0_TIMEOUT );
		// enable timer
		ROM_TimerEnable( TIMER_BASE_PING0, TIMER_PING0 );
		break;

	}


}

void ping0SetupPeriph( void ) {
	//
	// Enable input pin
	//
	ROM_SysCtlPeripheralEnable( GPIO_PERIPH_PING0 );
	//
	// Enable timer base
	//
	ROM_SysCtlPeripheralEnable( TIMER_PERIPH_PING0 );
}
void ping1SetupPeriph( void ) {

}


void ping0InitPeriph( void ) {
	//
	// Disable timer and set parameters
	//
	ROM_TimerDisable( TIMER_BASE_PING0,	TIMER_PING0 );
	ROM_TimerPrescaleSet( TIMER_BASE_PING0, TIMER_PING0, 0 );
	ROM_TimerPrescaleMatchSet( TIMER_BASE_PING0, TIMER_PING0, 0 );
	ROM_TimerControlStall( TIMER_BASE_PING0, TIMER_PING0, false );

	IntPrioritySet( TIMER_INT_PING0, 0x40 ); // set to a middle priority group
	//
	// Register inerrupt
	//
	TimerIntRegister( TIMER_BASE_PING0, TIMER_PING0, &ping0Int );
}
void ping1InitPeriph( void ) {

}


void ping0Start( void ) {
	ping0state = RECHARGE;
	ping0Int( );

}
void ping1Start( void ) {

}

//
// Get value, returns 0 if no new value
//
uint32_t ping0Get( void ){
	uint32_t ret = 0;
	if ( ping0updated )
		ret = ping0time;
	ping0updated = false;
	return ret;
}
uint32_t ping1Get( void );



