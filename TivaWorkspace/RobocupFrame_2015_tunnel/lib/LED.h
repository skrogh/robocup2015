/*
 * LED.h
 *
 *  Created on: 06/04/2014
 *      Author: Soren
 */

#ifndef LED_H_
#define LED_H_

#include <stdint.h>

void LEDInit( void );
void LEDSetColor( float red, float green, float blue );
void LEDSetRed( float red );
void LEDSetGreen( float green );
void LEDSetBlue( float blue );

void LEDSetSeq( uint32_t n );

#endif /* LED_H_ */

