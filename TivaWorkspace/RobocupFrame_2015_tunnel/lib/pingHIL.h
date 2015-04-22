/*
 * ultrasonicHIL.h
 *
 *  Created on: 26/02/2015
 *      Author: Soren
 */

#ifndef ULTRASONICHIL_H_
#define ULTRASONICHIL_H_

//
// Enable periphial
//
void ping0SetupPeriph( void );
void ping1SetupPeriph( void );

//
// Initialize hardware modules
//
void ping0InitPeriph( void );
void ping1InitPeriph( void );

//
// Start sensing
//
void ping0Start( void );
void ping1Start( void );

//
// Get value, returns 0 if no new value
//
uint32_t ping0Get( void );
uint32_t ping1Get( void );



#endif /* ULTRASONICHIL_H_ */
