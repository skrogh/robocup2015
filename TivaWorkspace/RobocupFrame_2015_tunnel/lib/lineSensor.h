/*
 * lineSensor.h
 *
 *  Created on: 03/04/2014
 *      Author: Soren
 */

#ifndef LINESENSOR_H_
#define LINESENSOR_H_

#define LINE_WHITE 0
#define LINE_BLACK 1
#define LINE_LEFT 0
#define LINE_RIGHT 1

#include <stdbool.h>
#include <stdint.h>

void lineSensorInit( bool initDist );
void lineSensorUpdate( bool initDist );
int32_t* lineSensorGetRaw( void );
float* lineSensorGetCalib( void );
float* lineSensorGetScaled( void );
float lineSensorGetMax( void );
float lineSensorGetMin( void );
float lineSensorGetFastMax( void );
float lineSensorGetFastMin( void );
float lineSensorGetMidt( bool black );
float lineSensorGetEdge( bool black, bool right );
float lineSensorGetWidth( bool black );
int32_t* distanceSensorGetRaw( void );

void lineSensorSetDark( void );
void lineSensorSetLight( void );

#endif /* LINESENSOR_H_ */
