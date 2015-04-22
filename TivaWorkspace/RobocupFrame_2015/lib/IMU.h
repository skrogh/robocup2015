/*
 * IMU.h
 *
 *  Created on: 04/04/2014
 *      Author: Soren
 */

#ifndef IMU_H_
#define IMU_H_

extern float attitude[4];

void IMUInit( void );
void IMUUpdate( void );

void IMUUpdateRef( void );

int32_t* IMUGetMagRaw( void );
int32_t* IMUGetAccRaw( void );
int32_t* IMUGetGyroRaw( void );

float IMUGetPlaneAngleVect( float x, float y );
float IMUGetPlaneAngleNorth( void );
float IMUGetHorizDiff( float x, float y, float z );

void sendCalibData( void );
void IMUSave( uint32_t slot );
void IMURecall( uint32_t slot );

void IMUCalib( void );

#endif /* IMU_H_ */
