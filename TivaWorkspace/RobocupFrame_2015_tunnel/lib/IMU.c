/*
 * IMU.c
 *
 *  Created on: 04/04/2014
 *      Author: Soren
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/i2c.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "I2C_Stellaris_API.h"

#include "sensorCalib.h"
#include "MadgwickAHRS.h"
#include "quart.h"
#include "uart.h"
#include "core.h"
#include <math.h>
#define sqrt __sqrtf // use hardware sqrt

#define GYRO_ADDR 0x69
#define ACC_ADDR  0x53
#define MAG_ADDR  0x1E

#define GYRO_ID 211
#define ACC_ID 229
#define MAG_ID 72

float initQ[4] = // Has been pre-inverted.
{ -0.9677773, -0.01570825, 0.009795288, -0.2538495 };

#define SAVES 10
float IMUSaves[SAVES][4]; // Saved values

int32_t magRaw[3];
int32_t accRaw[3];
int32_t gyroRaw[3];

float mag[3]; // Magnetometer in 1/1000 std field strength
float acc[3]; // Acceleration in g
float gyro[3]; // Gyro in rad/s
float attitude[4]; // rotation quarternion
float headding;

void IMUReset( void );

void IMUInit( void ) {
	// The I2C1 peripheral must be enabled before use.
	SysCtlPeripheralEnable( SYSCTL_PERIPH_I2C1 );
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOA );

	// Configure the pin muxing for I2C3 functions on port D0 and D1.
	GPIOPinConfigure( GPIO_PA6_I2C1SCL );
	GPIOPinConfigure( GPIO_PA7_I2C1SDA );

	// Select the I2C function for these pins.  This function will also
	// configure the GPIO pins pins for I2C operation, setting them to
	// open-drain operation with weak pull-ups.  Consult the data sheet
	// to see which functions are allocated per pin.
	GPIOPinTypeI2CSCL( GPIO_PORTA_BASE, GPIO_PIN_6 );
	GPIOPinTypeI2C( GPIO_PORTA_BASE, GPIO_PIN_7 );

	// Initialize I2C3 peripheral at 400KHz
	I2CMasterInitExpClk( I2C1_BASE, SysCtlClockGet(), true );

	// Wait for power on to all units (0.1s)
	ROM_SysCtlDelay( ROM_SysCtlClockGet()/30 );

	// Setup magnatometer
	I2CRegWrite( I2C1_BASE, MAG_ADDR, 0x00, 0x18 ); // Set at 75Hz, no averaging, no load
	I2CRegWrite( I2C1_BASE, MAG_ADDR, 0x01, 0x20 ); // Set at +/- 1.8 Ga
	I2CRegWrite( I2C1_BASE, MAG_ADDR, 0x02, 0x00 ); // Set to continuous mode

	// Setup Accelerometer
	I2CRegWrite( I2C1_BASE, ACC_ADDR, 0x2D, 0x08 ); // Wake, tell never to sleep
	I2CRegWrite( I2C1_BASE, ACC_ADDR, 0x31, 0x0B ); // Use full resolution, full scale, right justified.
	I2CRegWrite( I2C1_BASE, ACC_ADDR, 0x31, 0x0F ); // Full rate (1.6KHz), could be set lower I guess..

	// Setup Gyro
	I2CRegWrite( I2C1_BASE, GYRO_ADDR, 0x20, 0xFF ); // Full speed, 115Hz lowpass, all axis on
	I2CRegWrite( I2C1_BASE, GYRO_ADDR, 0x23, 0xD0 ); // block update, big endian, 500dps,

	UARTprintf( "Mag is: 72/%d\nAcc is: 229/%d\nGyro is: 211/%d\n",
			I2CRegRead( I2C1_BASE, MAG_ADDR, 10 ),
			I2CRegRead( I2C1_BASE, ACC_ADDR, 0 ),
			I2CRegRead( I2C1_BASE, GYRO_ADDR, 0x0F )
	);
	if( ( MAG_ID != I2CRegRead( I2C1_BASE, MAG_ADDR, 10 ) ) ||
			( ACC_ID != I2CRegRead( I2C1_BASE, ACC_ADDR, 0 ) ) ||
			( GYRO_ID != I2CRegRead( I2C1_BASE, GYRO_ADDR, 0x0F ) ) )
		hardFault = 1;
}

void IMUUpdate( void ) {
	// Get magnatometer
	uint32_t tmp[6];
	I2CReadData( I2C1_BASE, MAG_ADDR, 0x03, tmp, 6 ); // Read 6 bytes from addr 0x03 and forward.
	magRaw[0] =  (int16_t) ( ( tmp[4]<<8 ) | tmp[5] ); // x = z
	magRaw[1] =  (int16_t) ( ( tmp[0]<<8 ) | tmp[1] ); // y = x
	magRaw[2] =  (int16_t) ( ( tmp[2]<<8 ) | tmp[3] ); // z = y

	// Get accelerometer
	I2CReadData( I2C1_BASE, ACC_ADDR, 0x32, tmp, 6 ); // Read 6 bytes from addr 0x32 and forward.
	accRaw[0] = (int16_t) ( ( tmp[3]<<8 ) | tmp[2] ); // x = y
	accRaw[1] = (int16_t) ( ( tmp[1]<<8 ) | tmp[0] ); // y = x
	accRaw[2] = (int16_t) ( ( tmp[5]<<8 ) | tmp[4] ); // z = z

	// Get gyro
	I2CReadData( I2C1_BASE, GYRO_ADDR, 0x28 | 0x80 , tmp, 6 ); // Read 6 bytes from addr 0x28 and forward.
	gyroRaw[0] =  -(int16_t) ( ( tmp[2]<<8 ) | tmp[3] ); // x = y
	gyroRaw[1] =  -(int16_t) ( ( tmp[0]<<8 ) | tmp[1] ); // y = -x
	gyroRaw[2] =  -(int16_t) ( ( tmp[4]<<8 ) | tmp[5] ); // z = -z

	// Calibrate sensors
	uint32_t i;
	for ( i = 0; i < 3; i++ ) {
		mag[i] = magRaw[i] - magOffset[i];
		mag[i] *= magScale[i];
		acc[i] = accRaw[i] - accOffset[i];
		acc[i] *= accScale[i];
		gyro[i] = gyroRaw[i] - gyroOffset[i];
		gyro[i] *= gyroScale[i];
	}

	// Update orientation
	//filterUpdate( gyro[0], gyro[1], gyro[2], acc[0], acc[1], acc[2], mag[0], mag[1], mag[2]);
	MadgwickAHRSupdate( gyro[0], gyro[1], gyro[2], acc[0], acc[1], acc[2], mag[0], mag[1], mag[2]);

	// Subtract initial frame;
	float inter[4];
	inter[0] = QUART_MUL_0( initQ[0], initQ[1], initQ[2], initQ[3], q0, q1, q2, q3 );
	inter[1] = QUART_MUL_X( initQ[0], initQ[1], initQ[2], initQ[3], q0, q1, q2, q3 );
	inter[2] = QUART_MUL_Y( initQ[0], initQ[1], initQ[2], initQ[3], q0, q1, q2, q3 );
	inter[3] = QUART_MUL_Z( initQ[0], initQ[1], initQ[2], initQ[3], q0, q1, q2, q3 );

	// invert, to get earth to body instead of body to earth
	attitude[0] = inter[0];
	attitude[1] = inter[1];
	attitude[2] = -inter[2];
	attitude[3] = -inter[3];
	/* for vector rotate
	 *
	tmp1[0] = QUART_VEC_ROT_X( tmp[0], tmp[1], tmp[2], attitude[0], attitude[1], attitude[2], attitude[3] );
	tmp1[1] = QUART_VEC_ROT_Y( tmp[0], tmp[1], tmp[2], attitude[0], attitude[1], attitude[2], attitude[3] );
	tmp1[2] = QUART_VEC_ROT_Z( tmp[0], tmp[1], tmp[2], attitude[0], attitude[1], attitude[2], attitude[3] );
	 */
}

void IMUUpdateRef( void ) {
	initQ[0] = q0;
	initQ[1] = -q1;
	initQ[2] = -q2;
	initQ[3] = -q3;
}

int32_t* IMUGetMagRaw( void ) {
	return magRaw;
}
int32_t* IMUGetAccRaw( void ) {
	return accRaw;
}
int32_t* IMUGetGyroRaw( void ) {
	return gyroRaw;
}

float IMUGetPlaneAngleVect( float x, float y ) {
	float norm = 1 / sqrt( x*x +y*y );
	x *= norm;
	y *= norm;
	float tmp[3] = { 0, 1, 0 };
	float tmp1[3];
	tmp1[0] = QUART_VEC_ROT_X( tmp[0], tmp[1], tmp[2], attitude[0], attitude[1], attitude[2], attitude[3] );
	tmp1[1] = QUART_VEC_ROT_Y( tmp[0], tmp[1], tmp[2], attitude[0], attitude[1], attitude[2], attitude[3] );
	tmp1[2] = QUART_VEC_ROT_Z( tmp[0], tmp[1], tmp[2], attitude[0], attitude[1], attitude[2], attitude[3] );
	norm = 1 / sqrt( tmp1[0]*tmp1[0] + tmp1[1]*tmp1[1] );
	tmp1[0] *= norm;
	tmp1[1] *= norm;
	float dot = tmp1[0]*x + tmp1[1]*y;
	float cross = tmp1[0]*y - tmp1[1]*x;
	return atan2f( cross, dot )*57.2958; // 57.2958 is 180/pi
}
float IMUGetPlaneAngleNorth( void ) {
	return IMUGetPlaneAngleVect( 0, 1 );
}
float IMUGetHorizDiff( float x, float y, float z) {
	float norm = 1 / sqrt( x*x + y*y + z*z );
	x *= norm;
	y *= norm;
	z *= norm;
	float tmp[3] = { 0, 0, 1 };
	float tmp1[3];
	tmp1[0] = QUART_VEC_ROT_X( tmp[0], tmp[1], tmp[2], attitude[0], attitude[1], attitude[2], attitude[3] );
	tmp1[1] = QUART_VEC_ROT_Y( tmp[0], tmp[1], tmp[2], attitude[0], attitude[1], attitude[2], attitude[3] );
	tmp1[2] = QUART_VEC_ROT_Z( tmp[0], tmp[1], tmp[2], attitude[0], attitude[1], attitude[2], attitude[3] );
	float cross[3] = {
			y*tmp1[2] - z*tmp1[1],
			z*tmp1[0] - x*tmp1[2],
			x*tmp1[1] - y*tmp1[0]
	};

	return asinf ( sqrt( cross[0]*cross[0] + cross[1]*cross[1] + cross[2]*cross[2] ) )*57.2958; // 57.2958 is 180/pi;
}

void IMUReset( void ) {
	IMUInit( );
}

void IMUSave( uint32_t slot ){
	if ( slot >= SAVES )
		return;
	IMUSaves[slot][0] = q0;
	IMUSaves[slot][1] = -q1;
	IMUSaves[slot][2] = -q2;
	IMUSaves[slot][3] = -q3;
}

void IMURecall( uint32_t slot ){
	if ( slot >= SAVES )
		return;
	initQ[0] = IMUSaves[slot][0];
	initQ[1] = IMUSaves[slot][1];
	initQ[2] = IMUSaves[slot][2];
	initQ[3] = IMUSaves[slot][3];
}

void IMUCalib( void ) {
	UARTprintf( "%8d %8d %8d %8d %8d %8d %8d %8d %8d\n",
			(int32_t)( IMUGetAccRaw( )[0]  ), (int32_t)( IMUGetAccRaw( )[1]  ), (int32_t)( IMUGetAccRaw( )[2]  ),
			(int32_t)( IMUGetGyroRaw( )[0]  ), (int32_t)( IMUGetGyroRaw( )[1]  ), (int32_t)( IMUGetGyroRaw( )[2]  ),
			(int32_t)( IMUGetMagRaw( )[0]  ), (int32_t)( IMUGetMagRaw( )[1]  ), (int32_t)( IMUGetMagRaw( )[2]  )
	);
}
