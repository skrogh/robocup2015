/*
 * state.c
 *
 *  Created on: 05/04/2014
 *      Author: Soren
 */
#define TAKE_TUNNEL

#include <stdint.h>
#include <stdbool.h>
#include "lib/core.h"
#include "lib/motor.h"
#include "lib/uart.h"
#include "lib/lineSensor.h"
#include "lib/IMU.h"
#include "lib/state.h"
#include "lib/steering.h"
#include "lib/distanceSensor.h"
#include "lib/LED.h"
#include "lib/MadgwickAHRS.h"

// GOTO is go to and GOTH is go through.
typedef enum {
	STATE_GOTO_1, STATE_GOTO_2, STATE_GOTO_3,
	STATE_GOTO_7,
	STATE_GODOWN_RAMP, STATE_GOTO_BACK_ON_BLACK,
	STATE_GOTO_SILVER_LINE1, STATE_TURN_SILVER_LINE1, STATE_GOTO_SILVER_LINE2, STATE_TURN_SILVER_LINE2, STATE_GOTO_SILVER_LINE_END, STATE_TURN_SILVER_LINE_END,
	STATE_GOTO_RR, STATE_TURN_RR_1, STATE_GOTH_RR, STATE_TURN_RR_END,

	STATE_GOTO_TUNNEL_1, STATE_GOOUT_TUNNEL_1, STATE_GOOUT_TUNNEL_2, STATE_GOOUT_TUNNEL_3, STATE_GOOUT_TUNNEL_4,
 	STATE_CLOSE_TUNNEL_1, STATE_CLOSE_TUNNEL_2, STATE_CLOSE_TUNNEL_3, STATE_CLOSE_TUNNEL_4, STATE_CLOSE_TUNNEL_5,


	STATE_GOTO_GOAL,


	STATE_GOTO_GOAL1, STATE_GOTO_GOAL2,


	STATE_GOALL,

	STATE_FOLLOW_WALL,

	STATE_START, STATE_SENSOR_CALIB,
	STATE_END,
	STATE_ERROR1, STATE_ERROR2
} STATES;

STATES stateStart = STATE_TURN_RR_END;

STATES state = STATE_SENSOR_CALIB;
float stateTime = 0;
float stateTimer1 = 0;
float stateTimer2 = 0; // timer 2 is not auto incremented or reset.
uint32_t stateCounter = 0; // is not reset automatically
bool stateChanged = true;
STATES statePrev;

void stateUpdateEveryTime( void ) {
	if ( hardFault && ( state != STATE_ERROR1 ) && ( state != STATE_ERROR2 ) )
		state = STATE_ERROR1;

	bool res = state != statePrev;
	statePrev = state;
	stateTime += LOOP_PERI;
	stateTimer1 += LOOP_PERI;
	stateTimer2 += LOOP_PERI;
	if ( res ) {
		stateChanged = true;
		stateTime = 0;
		stateTimer1 = 0;
		stateTimer2 = 0;
	} else {
		stateChanged = false;
	}
	LEDSetSeq( state );
}

void stateInit( void ) {
	statePrev = STATE_END; // to get the changed flag
	//LEDSetColor( 1, 1, 0 );
}

void stateUpdate( void ) {
	stateUpdateEveryTime( );
	switch ( state ) {
	case STATE_START:
		if ( ( distanceSensorGet( 0 ) < 35 ) && ( stateTime > 1 ) )
			state = STATE_GOTO_1;
		motorPolarSet( 0, 0 );
		break;


	case STATE_GOTO_1:
		if ( ( distanceSensorGet( 0 ) < 35 ) && ( stateTime > 1 ) )
			state = STATE_GOTO_2;
		motorPolarSet( 0.5, 0.5 * lineSensorGetMidt( LINE_WHITE ) );
		break;
	case STATE_GOTO_2:
		if ( ( distanceSensorGet( 0 ) < 35 ) && ( stateTime > 1 ) )
			state = STATE_GOTO_3;
		motorPolarSet( 0.5, 0.5 * lineSensorGetMidt( LINE_WHITE ) );
		break;
	case STATE_GOTO_3:
		if ( ( distanceSensorGet( 0 ) < 35 ) && ( stateTime > 1 ) )
			state = STATE_GOTO_7;
		motorPolarSet( 0.5, 0.5 * lineSensorGetMidt( LINE_WHITE ) );
		break;
	case STATE_GOTO_7:
		if ( ( distanceSensorGet( 0 ) < 35 ) && ( stateTime > 1 ) )
			state = STATE_GODOWN_RAMP;
		motorPolarSet( 0.5, 0.5 * lineSensorGetEdge( LINE_WHITE, LINE_RIGHT ) );
		break;
	case STATE_GODOWN_RAMP:
		if ( ( IMUGetHorizDiff(0,0,1) < 5 ) && ( stateTime > 1 ) )
			state = STATE_GOTO_BACK_ON_BLACK;
		motorPolarSet( 0.3, 0.3 * lineSensorGetMidt( LINE_WHITE ) );
		break;
	case STATE_GOTO_BACK_ON_BLACK:
		if ( ( lineSensorGetFastMin() < 0.5 ) && ( stateTime > 1 ) )
			state = STATE_GOTO_SILVER_LINE1;
		motorPolarSet( 0.5, 0.5 * lineSensorGetMidt( LINE_WHITE ) );
		break;

	case STATE_GOTO_SILVER_LINE1:
		if ( ( lineSensorGetWidth( LINE_WHITE ) > 1.5 ) && ( stateTime > 1 ) )
			state = STATE_TURN_SILVER_LINE1;
		motorPolarSet( 0.3, 0.3 * lineSensorGetMidt( LINE_WHITE ) );
		break;
	case STATE_TURN_SILVER_LINE1:
		if ( stateTime > 0.5 )
			state = STATE_GOTO_SILVER_LINE2;
		motorPolarSet( 0.3, 0.3 );
		break;
	case STATE_GOTO_SILVER_LINE2:
		if ( ( lineSensorGetMin( ) > 0.25 ) && ( stateTime > 1 ) )
			state = STATE_TURN_SILVER_LINE2;
		motorPolarSet( 0.3, 0.3 * lineSensorGetMidt( LINE_WHITE ) );
		break;
	case STATE_TURN_SILVER_LINE2:
		if ( stateTime > 0.5 )
			state = STATE_GOTO_SILVER_LINE_END;
		motorPolarSet( 0.3, -0.3 );
		break;
	case STATE_GOTO_SILVER_LINE_END:
		if ( ( lineSensorGetMin( ) > 0.25 ) && ( stateTime > 1 ) )
			state = STATE_TURN_SILVER_LINE_END;
		motorPolarSet( 0.3, 0.3 * lineSensorGetMidt( LINE_WHITE ) );
		break;
	case STATE_TURN_SILVER_LINE_END:
		if ( stateTime > 0.5 )
			state = STATE_GOTO_RR;
		motorPolarSet( 0.3, 0.3 * lineSensorGetEdge( LINE_WHITE, LINE_RIGHT ) );
		break;
	case STATE_GOTO_RR:
		if ( ( lineSensorGetWidth( LINE_WHITE ) > 1.5 ) && ( stateTime > 0.5 ) )
			state = STATE_TURN_RR_1;
		motorPolarSet( 0.3, 0.3 * lineSensorGetEdge( LINE_WHITE, LINE_LEFT ) );
		break;
	case STATE_TURN_RR_1:
		if ( stateTime < 0.5 )
			motorPolarSet( 0.3, 0.3 * lineSensorGetEdge( LINE_WHITE, LINE_LEFT ) );
		else if ( stateTime < 1 )
			motorPolarSet( 0.3, 0.3 * lineSensorGetMidt( LINE_WHITE ) );
		else
			state = STATE_GOTH_RR;
		break;
	case STATE_GOTH_RR:
		if ( ( lineSensorGetWidth( LINE_WHITE ) > 1.5 ) && ( IMUGetHorizDiff(0,0,1) < 5 ) && ( stateTime > 2 ) )
			state = STATE_TURN_RR_END;
		motorPolarSet( 0.6, 0.6 * lineSensorGetEdge( LINE_WHITE, LINE_RIGHT ) );
		break;
	case STATE_TURN_RR_END:
		if ( ( stateTime > 0.5 ) )
			state = STATE_GOTO_TUNNEL_1;
		motorPolarSet( 0.3, 0.3 * lineSensorGetEdge( LINE_WHITE, LINE_RIGHT ) );
		break;
	case STATE_GOTO_TUNNEL_1:
		if ( stateTime < 4 )
			motorPolarSet( 0.3, 0.3 * lineSensorGetEdge( LINE_WHITE, LINE_RIGHT ) );
		else if ( stateTime < 17 )
			motorPolarSet( 0.3, 0.3 * lineSensorGetEdge( LINE_WHITE, LINE_LEFT ) );
		else if ( stateTime < 17.5 )
			motorPolarSet( -0.3, -0.3 );
		else
			state = STATE_FOLLOW_WALL;
		break;
	case STATE_FOLLOW_WALL:
		if ( ( lineSensorGetMax( ) > 0.5 ) && ( stateTime > 3 ) )
			state = STATE_GOOUT_TUNNEL_1;
		motorPolarSetSaturating( 0.2, 0.05 * ( distanceSensorGet( 0 ) - 25 ), 0.2, 0.3 );
		break;

	case STATE_GOOUT_TUNNEL_1:
		if ( stateTime < 1 )
			motorPolarSet( 0.3, 0.3 * lineSensorGetEdge( LINE_WHITE, LINE_LEFT ) );
		else if ( stateTime < 1.5 )
			motorPolarSet( 0.3, 0.3 * lineSensorGetEdge( LINE_WHITE, LINE_RIGHT ) );
		else
			motorPolarSet( 0.3, 0.3 * lineSensorGetEdge( LINE_WHITE, LINE_RIGHT ) );
		if ( ( lineSensorGetWidth( LINE_WHITE ) > 1.5 ) && ( stateTime > 2 ) )
			state = STATE_GOOUT_TUNNEL_2;
		break;
	case STATE_GOOUT_TUNNEL_2:
		if ( ( lineSensorGetWidth( LINE_WHITE ) > 1.5 ) && ( stateTime > 1 ) )
			state = STATE_GOOUT_TUNNEL_3;
		motorPolarSet( 0.3, 0.3 * lineSensorGetEdge( LINE_WHITE, LINE_RIGHT ) );
		break;
	case STATE_GOOUT_TUNNEL_3:
		if ( ( lineSensorGetWidth( LINE_WHITE ) > 1.5 ) && ( stateTime > 1 ) )
			state = STATE_GOOUT_TUNNEL_4;
		motorPolarSet( 0.3, 0.3 * lineSensorGetEdge( LINE_WHITE, LINE_RIGHT ) );
		break;
	case STATE_GOOUT_TUNNEL_4:
		if ( ( lineSensorGetWidth( LINE_WHITE ) > 1.5 ) && ( stateTime > 1 ) )
			state = STATE_CLOSE_TUNNEL_1;
		motorPolarSet( 0.3, 0.3 * lineSensorGetEdge( LINE_WHITE, LINE_RIGHT ) );
		break;

	case STATE_CLOSE_TUNNEL_1:
			if ( ( lineSensorGetWidth( LINE_WHITE ) > 1.5 ) && ( stateTime > 1 ) )
				state = STATE_GOTO_GOAL;
			motorPolarSet( 0.3, 0.3 * lineSensorGetEdge( LINE_WHITE, LINE_RIGHT ) );
			break;



	case STATE_GOTO_GOAL:
		motorPolarSet( 0.5, 0.5 * lineSensorGetMidt( LINE_WHITE ) );
		break;


	case STATE_GOTO_GOAL1:
		if ( ( lineSensorGetWidth( LINE_WHITE ) > 1.5 ) && ( stateTime > 1 ) )
			state = STATE_GOTO_GOAL2;
		motorPolarSet( 0.5, 0.5 * lineSensorGetMidt( LINE_WHITE ) );
		break;
	case STATE_GOTO_GOAL2:
		if ( ( lineSensorGetWidth( LINE_WHITE ) > 1.5 ) && ( stateTime > 1 ) )
			state = STATE_GOTO_GOAL1;
		motorPolarSet( 0.5, 0.5 * lineSensorGetMidt( LINE_WHITE ) );
		break;




	case STATE_SENSOR_CALIB:
		if ( stateTime > 5 ) {
			state = stateStart;
			MadgwickResetBeta( );
			IMUUpdateRef( );
		}
		if ( stateTime < 0.5 )
			MadgwickSetBeta( 10 );
		else if ( stateTime < 1 )
			MadgwickSetBeta( 1 );
		else if ( stateTime < 5 )
			MadgwickResetBeta( );
		motorPolarSet( 0, 0 );
		break;
	case STATE_END:
		motorPolarSet( 0, 0 );
		break;
	case STATE_ERROR1:
		if ( stateTime > 0.5 )
			state = STATE_ERROR2;
		break;
	case STATE_ERROR2:
		if ( stateTime > 0.5 )
			state = STATE_ERROR1;
		break;

	}

}

