/*
 * distanceSensor.h
 *
 *  Created on: 06/04/2014
 *      Author: Soren
 */

#ifndef DISTANCESENSOR_H_
#define DISTANCESENSOR_H_

#include <stdbool.h>
#include <stdint.h>

extern float distanceSensorRawFilt[2];

// MUST BE INITIALIZED FROM LINE SENSOR FIRST, AS THEY USE SAME HARDWARE
void distanceSensorInit( void );
// MUST UPDATE LINE SENSOR FIRST!
void distanceSensorUpdate( void );
float distanceSensorGet( uint32_t sensor );

#endif /* DISTANCESENSOR_H_ */
