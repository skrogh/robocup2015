/*
 * steering.c
 *
 *  Created on: 06/04/2014
 *      Author: Soren
 */


#include "core.h"
#include "motor.h"
#include "uart.h"
#include "lineSensor.h"
#include "IMU.h"
#include "steering.h"

void turnVect( float x, float y ) {
	motorPolarSetSaturating( 0, IMUGetPlaneAngleVect( x, y ) * 0.1, -0.3, 0.3  );
}
void driveVect( float x, float y ) {
	motorPolarSetSaturating( 0.3, IMUGetPlaneAngleVect( x, y ) * 0.1, -0.1, 0.5  );
}
void driveVectSpeed( float x, float y, float speed ) {
	motorPolarSetSaturating( speed, IMUGetPlaneAngleVect( x, y ) * 0.1, speed - 0.1, speed + 0.1  );
}
