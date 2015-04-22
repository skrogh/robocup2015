//*****************************************************************************
//
// hello.c - Simple hello world example.
//
// Copyright (c) 2012-2013 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
// This is part of revision 1.0 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "lib/core.h"
#include "lib/motor.h"
#include "lib/uart.h"
#include "lib/lineSensor.h"
#include "lib/IMU.h"
#include "lib/state.h"
#include "lib/distanceSensor.h"
#include "lib/LED.h"

volatile uint32_t free = 0;

int
main(void)
{
	coreInit( );
	uartInit( );

	motorInit( );
	IMUInit( );
	lineSensorInit( false ); // init distance as well
	distanceSensorInit( );
	LEDInit( );

	// print sensor values
	{
		int i;
		for ( i = 0; i < 10; i++ ) {
			while( realTimeWait );
			realTimeWait = true;
			lineSensorUpdate( true );
		}
		int32_t* sensorRaw = lineSensorGetRaw();
		UARTprintf( "\033[H\n\n\n\n %d, %d, %d, %d, %d, %d",
				sensorRaw[0], sensorRaw[1], sensorRaw[2],
				sensorRaw[3], sensorRaw[4], sensorRaw[5] );
	}

	stateInit( );

	// Main code here:
	for ( ;; ) {
		uint32_t tmp = 0;
		while( realTimeWait )
			tmp++;
		free = tmp*5;
		realTimeWait = true;
		IMUUpdate( );
		lineSensorUpdate( false );
		distanceSensorUpdate( );

		//IMUCalib( );

		UARTprintf( "\033[H %4d %4d %4d %4d", (int32_t)( IMUGetHorizDiff(0,0,1) ),
				(int32_t)( IMUGetPlaneAngleNorth( ) ),
				(int32_t)( lineSensorGetMin()*1000 ),
				(int32_t)( lineSensorGetMax()*1000 ) );

		stateUpdate( );
	}

}
