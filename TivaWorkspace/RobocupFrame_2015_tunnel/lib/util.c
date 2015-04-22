/*
 * util.c
 *
 *  Created on: 03/04/2014
 *      Author: Soren
 */

/*
uint32_t i;
uint32_t calib[6] = {0,0,0,0,0,0};
uint32_t times = 1024;
for ( i = 0; i < times; i++ ) {
	while( realTimeWait );
	realTimeWait = true;
	lineSensorUpdate( false );
	int32_t* raw = lineSensorGetRaw();
	uint32_t j;
	for ( j = 0; j < 6; j++ )
		calib[j] += raw[j];
}

uint32_t j;
for ( j = 0; j < 6; j++ )
	calib[j] /= times;

UARTprintf( "\033[H \nSensor data is: %5d %5d %5d %5d %5d %5d",
		calib[0], calib[1],
		calib[2], calib[3],
		calib[4], calib[5] );
 */
