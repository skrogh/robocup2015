/*
 * LED.c
 *
 *  Created on: 06/04/2014
 *      Author: Soren/lawrence_jeff
 */

#include "LED.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"

static uint32_t period = 1023;

float LEDSequence[8][3] = {
		{ 0, 0, 0 }, // Black
		{ 1, 0, 0 }, // Red
		{ 0, 1, 0 }, // Green
		{ 0, 0, 1 }, // Blue
		{ 1, 1, 0 }, // Yellow
		{ 0, 1, 1 }, // Cyano
		{ 1, 0, 1 }, // Fuchsia
		{ 0.9, 1, 0.9 } // White
};

void LEDInit( void ) {
	//Configure PWM Clock to match system
	SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

	// Enable the peripherals used by this program.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);  //The Tiva Launchpad has two modules (0 and 1). Module 1 covers the LED pins

	//Configure PF1,PF2,PF3 Pins as PWM
	GPIOPinConfigure(GPIO_PF1_M1PWM5);
	GPIOPinConfigure(GPIO_PF2_M1PWM6);
	GPIOPinConfigure(GPIO_PF3_M1PWM7);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

	//Configure PWM Options
	//PWM_GEN_2 Covers M1PWM4 and M1PWM5
	//PWM_GEN_3 Covers M1PWM6 and M1PWM7 See page 207 4/11/13 DriverLib doc
	PWMGenConfigure( PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC | PWM_GEN_MODE_DBG_RUN );
	PWMGenConfigure( PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC | PWM_GEN_MODE_DBG_RUN );

	//Set the Period (expressed in clock ticks)
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, period+3);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, period+3);

	//Set PWM to off
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, period/2);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, period/2);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, period/2);

	// Enable the PWM generator
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);

	// Turn on the Output pins
	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT |PWM_OUT_6_BIT|PWM_OUT_7_BIT, true);
}

void LEDSetColor( float red, float green, float blue ) {
	// Square to approximate exponential output.
	red *= red;
	green *= green;
	blue *= blue;
	uint32_t uRed = (uint32_t)( red*period );
	uint32_t uGreen = (uint32_t)( green*period );
	uint32_t uBlue = (uint32_t)( blue*period );
	PWMPulseWidthSet( PWM1_BASE, PWM_OUT_5, uRed );
	PWMPulseWidthSet( PWM1_BASE, PWM_OUT_6, uBlue );
	PWMPulseWidthSet( PWM1_BASE, PWM_OUT_7, uGreen );
}

void LEDSetSeq( uint32_t n ) {
	n &= 0x07; // mod 8;
	LEDSetColor( LEDSequence[n][0], LEDSequence[n][1], LEDSequence[n][2] );
}
