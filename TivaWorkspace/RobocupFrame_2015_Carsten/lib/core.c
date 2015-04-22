/*
 * core.c
 *
 *  Created on: 03/04/2014
 *      Author: Soren
 */


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"

#include "inc/hw_ints.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "core.h"

volatile bool realTimeWait = true;
volatile uint32_t hardFault = 0;
volatile uint32_t softFault = 0;

void realTimeLoopInt( void ) {
	TimerIntClear( WTIMER0_BASE, TIMER_TIMA_TIMEOUT );
	realTimeWait = false;
}

void coreInit( void ) {
	//
	// Set the clocking to run at 80MHz. PLL from 16MHz Xtal
	//
	SysCtlClockSet( SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
			SYSCTL_OSC_MAIN );

	// Setup real time loop

	// Enable periphial
	SysCtlPeripheralEnable( SYSCTL_PERIPH_WTIMER0 );

	// Set timer period to 1/loop_freq
	TimerLoadSet( WTIMER0_BASE, TIMER_A, SysCtlClockGet( ) / LOOP_FREQ - 1 );

	// Register handler
	IntRegister( INT_WTIMER0A, &realTimeLoopInt );

	// Enable this interrupt
	IntEnable( INT_WTIMER0A );
	TimerIntEnable( WTIMER0_BASE, TIMER_TIMA_TIMEOUT );

	// Enable timer
	TimerEnable(WTIMER0_BASE, TIMER_A);

	// Enable master interrupt
	IntMasterEnable();
}
