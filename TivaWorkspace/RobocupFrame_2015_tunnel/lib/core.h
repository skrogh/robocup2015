/*
 * core.h
 *
 *  Created on: 03/04/2014
 *      Author: Soren
 */

#ifndef CORE_H_
#define CORE_H_

#include <stdbool.h>
#include <stdint.h>

#define LOOP_FREQ 500
#define LOOP_PERI 1.0/LOOP_FREQ

volatile extern uint32_t hardFault;
volatile extern uint32_t softFault;
volatile extern bool realTimeWait;

void coreInit( void );

#endif /* CORE_H_ */
