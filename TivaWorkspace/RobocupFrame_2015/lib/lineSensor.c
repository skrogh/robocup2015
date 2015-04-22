/*
 * lineSensor.c
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

#include "driverlib/fpu.h"
#include "driverlib/adc.h"
#include "lineSensor.h"
#include "sensorCalib.h"

// Filter variables
float min = 1;
float max = 0;
float filtMin = 0;
float filtMax = 1;
float filtFastMin = 0;
float filtFastMax = 1;
float filtFastMaxMax = 0;

// Raw values
uint32_t ADCValueOff[8];
uint32_t ADCValueOn[8];
int32_t lineSensorValueRaw[6];
int32_t distanceSensorRaw[2];

// Calibrated values
float lineSensorValue[6];
float lineSensorValueScaled[6];

// Calibration
extern int32_t lineSensorOffset[6];
extern float lineSensorScale[6];

void lineSensorInit( bool initDist ) {
	// Set ADC clock
	SysCtlADCSpeedSet( SYSCTL_ADCSPEED_1MSPS );

	// Enable periphery
	SysCtlPeripheralEnable( SYSCTL_PERIPH_ADC0 );
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOE ); // Led
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOD ); // Sensors

	// Setup PE4 for output, LED on/off
	GPIOPinTypeGPIOOutput( GPIO_PORTE_BASE, GPIO_PIN_4 );
	GPIOPinWrite( GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_PIN_4 );
	GPIOPadConfigSet( GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD );

	// Enable ADC over sampling
	ADCHardwareOversampleConfigure( ADC0_BASE, 0x08 );

	// Setup sampling sequence. Sample all light sensors, then the two distance sensors
	ADCSequenceDisable(ADC0_BASE, 0);
	ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH1);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_CH2);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ADC_CTL_CH3);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 4, ADC_CTL_CH4);
	if ( initDist ) {
		ADCSequenceStepConfigure(ADC0_BASE, 0, 5, ADC_CTL_CH5);
		ADCSequenceStepConfigure(ADC0_BASE, 0, 6, ADC_CTL_CH6);
		ADCSequenceStepConfigure(ADC0_BASE, 0, 7, ADC_CTL_CH7 | ADC_CTL_IE | ADC_CTL_END);
	} else {
		ADCSequenceStepConfigure(ADC0_BASE, 0, 5, ADC_CTL_CH5 | ADC_CTL_IE | ADC_CTL_END);
	}
	ADCSequenceEnable(ADC0_BASE, 0);


	// FPU stuff:

	// Enable FPU
	FPUEnable( );

	// Disable FPU stacking, thus disallowing use of fpu in interrupt
	// Which is ok and gives faster interrupts
	FPUStackingDisable( );
}

void lineSensorUpdate( bool updateDist ) {
	// Start ADC
	ADCIntClear( ADC0_BASE, 0 );
	ADCProcessorTrigger( ADC0_BASE, 0 );
	// Wait for ADC to finish.
	while( !ADCIntStatus( ADC0_BASE, 0, false ) );

	// Read the current state of the GPIO LED pin and
	// write back the opposite state
	if(GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_4)) // background light is on
	{
		// Get ADC data
		ADCSequenceDataGet( ADC0_BASE, 0, ADCValueOn);
		// Turn LEDs off
		GPIOPinWrite( GPIO_PORTE_BASE, GPIO_PIN_4, 0);
		if ( updateDist ) {
			distanceSensorRaw[0] = ADCValueOn[6];
			distanceSensorRaw[1] = ADCValueOn[7];
		}
	} else {
		// Get ADC data
		ADCSequenceDataGet(ADC0_BASE, 0, ADCValueOff);
		// Turn LEDs on
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_PIN_4);
		if ( updateDist ) {
			distanceSensorRaw[0] = ADCValueOff[6];
			distanceSensorRaw[1] = ADCValueOff[7];
		}
	}


	// Calculate calibrated sensor values and find min/max.
	uint32_t i;
	min = 2; // some value >> 1
	max = -1; // some value << 0
	for ( i = 0; i < 6; i++ ) {
		lineSensorValueRaw[i] = ADCValueOff[i] - ADCValueOn[i];
		int32_t tmp = lineSensorValueRaw[i];
		tmp -= lineSensorOffset[i];
		lineSensorValue[i] = tmp / lineSensorScale[i];
		min = (min>lineSensorValue[i])?lineSensorValue[i]:min;
		max = (max<lineSensorValue[i])?lineSensorValue[i]:max;
	}

	// filter min/max with T=3 at 500Hz
	float min_z1 = filtMin;
	float max_z1 = filtMax;
	filtMin = min*0.003992 + 0.996*min_z1;
	filtMax = max*0.003992 + 0.996*max_z1;
	// filter min/max with T=0.15 at 500Hz (settle time ~= 0.5)
	min_z1 = filtFastMin;
	max_z1 = filtFastMax;
	filtFastMin = min*0.01324 + 0.9868*min_z1;
	filtFastMax = max*0.01324 + 0.9868*max_z1;

	filtFastMaxMax = (filtFastMaxMax>filtFastMax)?filtFastMaxMax:filtFastMax;

	// Scale
	for ( i = 0; i < 6; i++ ) {
		lineSensorValueScaled[i] = lineSensorValue[i] - filtMin;
		lineSensorValueScaled[i] /= filtMax-filtMin;
	}

}

int32_t* lineSensorGetRaw( void ) {
	return lineSensorValueRaw;
}

float* lineSensorGetCalib( void ) {
	return lineSensorValue;
}
float* lineSensorGetScaled( void ) {
	return lineSensorValueScaled;
}
float lineSensorGetMax( void ) {
	return max;
}
float lineSensorGetMin( void ) {
	return min;
}
float lineSensorGetFastMax( void ) {
	return filtFastMax;
}
float lineSensorGetFastMin( void ) {
	return filtFastMin;
}


float lineSensorGetMidt( bool black ) {
	uint32_t i;
	// Using own SSCoM algorithm
	// (scale-square-center-of-mass)


	// Get min and max value for scaling
	float min = 2; // some value >> 1
	float max = -1; // some value << 0
	for ( i = 0; i < 6; i++ ) {
		min = (min>lineSensorValue[i])?lineSensorValue[i]:min;
		max = (max<lineSensorValue[i])?lineSensorValue[i]:max;
	}

	// Scale and square, find CoM*6
	float CoM = 0;
	float mass = 0;
	float scaled[6];
	for ( i = 0; i < 6; i++ ) {
		// Scale
		scaled[i] = lineSensorValue[i] - min;
		scaled[i] /= max-min;
		// Invert if black
		if ( black )
			scaled[i] = 1.0 - scaled[i];
		// Square
		scaled[i] *= scaled[i];
		// CoM
		CoM += scaled[i]*i;
		mass += scaled[i];
	}
	// divide by 6.
	CoM /= mass;
	CoM -= 2.5;
	CoM /= 2.5;

	return CoM;
}

float lineSensorGetEdge( bool black, bool right ) {
	uint32_t i;

	// Invert if black
	float scaled[6];
	for ( i = 0; i < 6; i++ ) {
		// Scale
		scaled[i] = lineSensorValueScaled[i];
		// Invert if black
		if ( black )
			scaled[i] = 1.0 - scaled[i];
	}

	if ( right ) { // Find right side of line
		i = 0;
		while( ( scaled[i] < 0.5 ) && ( i < 5 ) )
			i++;
		if ( i == 0 ) // all to the left
			return -1;
		if ( i == 5 ) // all to the right
			return 1;
		float tmp = 0.5;
		tmp -= scaled[i-1];
		tmp /= scaled[i] - scaled[i-1];
		tmp += i-1;
		tmp -= 2.5;
		tmp /= 2.5;
		return tmp;
	} else {
		i = 5;
		while( ( scaled[i] < 0.5 ) && ( i > 0 ) )
			i--;
		if ( i == 0 ) // all to the left
			return -1;
		if ( i == 5 ) // all to the right
			return 1;
		float tmp = 0.5;
		tmp -= scaled[i+1];
		tmp /= scaled[i] - scaled[i+1];
		tmp = i + 1 - tmp;
		tmp -= 2.5;
		tmp /= 2.5;
		return tmp;
	}
}

float lineSensorGetWidth( bool black ) {
	return lineSensorGetEdge( black, LINE_LEFT ) - lineSensorGetEdge( black, LINE_RIGHT );
}

int32_t* distanceSensorGetRaw( void ) {
	return distanceSensorRaw;
}

void lineSensorSetDark( void ) {
	uint32_t i;
	for ( i = 0; i < 6; i++ ) {
		lineSensorOffset[i] = lineSensorValueRaw[i];
	}
}
void lineSensorSetLight( void ) {
	uint32_t i;
	for ( i = 0; i < 6; i++ ) {
		lineSensorScale[i] = lineSensorValueRaw[i] - lineSensorOffset[i];
	}
}
