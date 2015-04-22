#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"

#include "driverlib/pwm.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "motor.h"

#define DEADBAND_2 1000
#define DEADBAND DEADBAND_2/6


void motorInit( void ) {
	// Enable port B
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOB );
	// Enable timer0-3
	SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER0 );
	SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER1 );
	SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER2 );
	SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER3 );


	TimerDisable( TIMER0_BASE, TIMER_BOTH );
	TimerDisable( TIMER1_BASE, TIMER_BOTH );
	TimerDisable( TIMER2_BASE, TIMER_BOTH );
	TimerDisable( TIMER3_BASE, TIMER_BOTH );

	// Set to split PWM
	TimerConfigure( TIMER0_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM );
	TimerConfigure( TIMER1_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM );
	TimerConfigure( TIMER2_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM );
	TimerConfigure( TIMER3_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM );

	// Set period to longest
	TimerLoadSet( TIMER0_BASE, TIMER_BOTH , 0xFFFF );
	TimerLoadSet( TIMER1_BASE, TIMER_BOTH , 0xFFFF );
	TimerLoadSet( TIMER2_BASE, TIMER_BOTH , 0xFFFF );
	TimerLoadSet( TIMER3_BASE, TIMER_BOTH , 0xFFFF );

	// Set all pins to full on
	TimerMatchSet( TIMER0_BASE, TIMER_BOTH, 0x0000 );
	TimerMatchSet( TIMER1_BASE, TIMER_BOTH, 0x0000 );
	TimerMatchSet( TIMER2_BASE, TIMER_BOTH, 0x0000 );
	TimerMatchSet( TIMER3_BASE, TIMER_BOTH, 0x0000 );

	// Set pins to PWM
	GPIOPinTypeTimer( GPIO_PORTB_BASE,
			GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|
			GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 ); // Set pintype
	GPIOPinConfigure( GPIO_PB0_T2CCP0 );
	GPIOPinConfigure( GPIO_PB1_T2CCP1 );
	GPIOPinConfigure( GPIO_PB2_T3CCP0 );
	GPIOPinConfigure( GPIO_PB3_T3CCP1 );
	GPIOPinConfigure( GPIO_PB4_T1CCP0 );
	GPIOPinConfigure( GPIO_PB5_T1CCP1 );
	GPIOPinConfigure( GPIO_PB6_T0CCP0 );
	GPIOPinConfigure( GPIO_PB7_T0CCP1 );// Set pin PB1-3 to PWM

	// Start with offset
	TimerEnable( TIMER0_BASE, TIMER_A );
	TimerEnable( TIMER1_BASE, TIMER_B );
	TimerEnable( TIMER2_BASE, TIMER_A );
	TimerEnable( TIMER3_BASE, TIMER_B );

	ROM_SysCtlDelay( DEADBAND );

	TimerEnable( TIMER0_BASE, TIMER_B );
	TimerEnable( TIMER1_BASE, TIMER_A );
	TimerEnable( TIMER2_BASE, TIMER_B );
	TimerEnable( TIMER3_BASE, TIMER_A );

	// Make sure motors are stopped.
	motorRightStop();
	motorLeftStop();
}

void motorLeftSet( int32_t speed, bool direction ) {
	if ( speed > 0xFFFA - DEADBAND_2 )
		speed = 0xFFFA - DEADBAND_2;
	if ( speed < 0 )
		speed = 0;
	if ( direction ) {
		TimerMatchSet( TIMER2_BASE, TIMER_A, 0x0000 ); // PWM
		TimerMatchSet( TIMER2_BASE, TIMER_B, 0x0000 ); // PWM
		TimerMatchSet( TIMER3_BASE, TIMER_A, speed + ( DEADBAND_2 ) ); // On
		TimerMatchSet( TIMER3_BASE, TIMER_B, speed ); // On
	} else {
		TimerMatchSet( TIMER2_BASE, TIMER_A, speed ); // PWM
		TimerMatchSet( TIMER2_BASE, TIMER_B, speed + (DEADBAND_2 ) ); // PWM
		TimerMatchSet( TIMER3_BASE, TIMER_A, 0x0000 ); // On
		TimerMatchSet( TIMER3_BASE, TIMER_B, 0x0000 ); // On
	}
}

void motorRightSet( int32_t speed, bool direction ) {
	if ( speed > 0xFFFA - DEADBAND_2 )
		speed = 0xFFFA - DEADBAND_2;
	if ( speed < 0 )
		speed = 0;
	if ( direction ) {
		TimerMatchSet( TIMER0_BASE, TIMER_A, speed ); // PWM
		TimerMatchSet( TIMER0_BASE, TIMER_B, speed + ( DEADBAND_2 ) ); // PWM
		TimerMatchSet( TIMER1_BASE, TIMER_A, 0xFFFF ); // On
		TimerMatchSet( TIMER1_BASE, TIMER_B, 0xFFFF ); // On
	} else {
		TimerMatchSet( TIMER0_BASE, TIMER_A, 0xFFFF ); // PWM
		TimerMatchSet( TIMER0_BASE, TIMER_B, 0xFFFF ); // PWM
		TimerMatchSet( TIMER1_BASE, TIMER_A, speed + ( DEADBAND_2 ) ); // On
		TimerMatchSet( TIMER1_BASE, TIMER_B, speed ); // On
	}
}

void motorRightStop( void ) {
	TimerMatchSet( TIMER0_BASE, TIMER_A, 0xFFFF ); // PWM
	TimerMatchSet( TIMER0_BASE, TIMER_B, 0xFFFF ); // PWM
	TimerMatchSet( TIMER1_BASE, TIMER_A, 0xFFFF ); // On
	TimerMatchSet( TIMER1_BASE, TIMER_B, 0xFFFF ); // On
}

void motorLeftStop( void ) {
	TimerMatchSet( TIMER2_BASE, TIMER_A, 0xFFFF ); // PWM
	TimerMatchSet( TIMER2_BASE, TIMER_B, 0xFFFF ); // PWM
	TimerMatchSet( TIMER3_BASE, TIMER_A, 0xFFFF ); // On
	TimerMatchSet( TIMER3_BASE, TIMER_B, 0xFFFF ); // On
}

void motorPolarSet( float speed, float omega ) {
	float left = speed - omega;
	float right = speed + omega;
	if ( left > 0 )
		motorLeftSet( left * 0xFFFA, 1 );
	else
		motorLeftSet( -left * 0xFFFA, 0 );
	if ( right > 0 )
		motorRightSet( right * 0xFFFA, 1 );
	else
		motorRightSet( -right * 0xFFFA, 0 );
}

void motorPolarSetSaturating( float speed, float omega, float min, float max ) {
	float left = speed - omega;
	float right = speed + omega;
	// Saturate
	left = (left>max)?max:left;
	right = (right>max)?max:right;
	left = (left<min)?min:left;
	right = (right<min)?min:right;
	// Set speeds
	if ( left > 0 )
		motorLeftSet( left * 0xFFFA, 1 );
	else
		motorLeftSet( -left * 0xFFFA, 0 );
	if ( right > 0 )
		motorRightSet( right * 0xFFFA, 1 );
	else
		motorRightSet( -right * 0xFFFA, 0 );
}
