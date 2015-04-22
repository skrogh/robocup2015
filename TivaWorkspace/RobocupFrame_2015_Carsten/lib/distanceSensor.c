/*
 * distanceSensor.c
 *
 *  Created on: 06/04/2014
 *      Author: Soren
 */

#include "distanceSensor.h"
#include "lineSensor.h"
#include "sensorCalib.h"
#include "pingHIL.h"

float distanceSensorVal[2] = {0, 0}; // in cm
float distanceSensorRawFilt[2] = {0, 0};
float z1[2] = {0, 0};
float z1_raw[2] = {0, 0};

#define b  0.07688
#define b_raw 0.09516
#define a (1-b)
#define a_raw (1-b_raw)

void distanceSensorInit( void ) {
	ping0SetupPeriph( );
	ping0InitPeriph( );
	ping0Start( );

}

void distanceSensorUpdate( void ) {
	uint32_t raw;
	// If we have a new value
	if( raw = ping0Get( ) ) {
		distanceSensorVal[0] = raw * 0.00021268125; // 34029cm/s / 80000000MHz * 1/2
	}
	distanceSensorVal[1] = 0;
}

float distanceSensorGet( uint32_t sensor ) {
	return distanceSensorVal[sensor];
}
