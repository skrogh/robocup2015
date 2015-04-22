/*
 * MARGfilter.h
 *
 *  Created on: 13/11/2013
 *      Author: Soren
 */

#ifndef MARGFILTER_H_
#define MARGFILTER_H_

// System constants
#define deltat 0.05f // sampling period in seconds (shown as 2 ms)
#define gyroMeasError 3.14159265358979 * (5.0f / 180.0f) // gyroscope measurement error in rad/s (shown as 5 deg/s)
#define gyroMeasDrift 3.14159265358979 * (0.2f / 180.0f) // gyroscope measurement error in rad/s/s (shown as 0.2f deg/s/s)
#define beta 0.8660254038f * gyroMeasError // compute beta
#define zeta 0.8660254038f * gyroMeasDrift // compute zeta
// Global system variables
extern float a_x, a_y, a_z; // accelerometer measurements
extern float w_x, w_y, w_z; // gyroscope measurements in rad/s
extern float m_x, m_y, m_z; // magnetometer measurements
extern float SEq_1, SEq_2, SEq_3, SEq_4; // estimated orientation quaternion elements with initial conditions
extern float b_x, b_z; // reference direction of flux in earth frame
extern float w_bx, w_by, w_bz; // estimate gyroscope biases error

extern void filterUpdate(float w_x, float w_y, float w_z, float a_x, float a_y, float a_z, float m_x, float m_y, float m_z);


#endif /* MARGFILTER_H_ */
