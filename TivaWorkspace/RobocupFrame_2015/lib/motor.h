/*
 * motor.h
 *
 *  Created on: 03/04/2014
 *      Author: Soren
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include <stdint.h>
#include <stdbool.h>


void motorInit( void );
void motorLeftSet( int32_t speed, bool direction );
void motorRightSet( int32_t speed, bool direction );
void motorRightStop( void );
void motorLeftStop( void );
void motorPolarSet( float speed, float omega );
void motorPolarSetSaturating( float speed, float omega, float min, float max );


#endif /* MOTOR_H_ */
