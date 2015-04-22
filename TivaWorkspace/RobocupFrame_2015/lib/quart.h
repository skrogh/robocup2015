/*
 * quart.h
 *
 *  Created on: 05/04/2014
 *      Author: Soren
 */

#ifndef QUART_H_
#define QUART_H_

#define QUART_INV_0( q0, qX, qY, qZ ) (q0)
#define QUART_INV_X( q0, qX, qY, qZ ) (-qX)
#define QUART_INV_Y( q0, qX, qY, qZ ) (-qY)
#define QUART_INV_Z( q0, qX, qY, qZ ) (-qZ)

#define QUART_MUL_0( q10, q1X, q1Y, q1Z, q20, q2X, q2Y, q2Z ) ( q10*q20 - q1X*q2X - q1Y*q2Y - q1Z*q2Z )
#define QUART_MUL_X( q10, q1X, q1Y, q1Z, q20, q2X, q2Y, q2Z ) ( q1X*q20 + q10*q2X - q1Z*q2Y + q1Y*q2Z )
#define QUART_MUL_Y( q10, q1X, q1Y, q1Z, q20, q2X, q2Y, q2Z ) ( q1Y*q20 + q1Z*q2X + q10*q2Y - q1X*q2Z )
#define QUART_MUL_Z( q10, q1X, q1Y, q1Z, q20, q2X, q2Y, q2Z ) ( q1Z*q20 - q1Y*q2X + q1X*q2Y + q10*q2Z )

#define QUART_VEC_ROT_X( vX, vY, vZ, q0, qX, qY, qZ )  ( q0*q0*vX + 2*q0*vZ*qY - 2*q0*vY*qZ + 2*vZ*qX*qZ - qY*qY*vX + 2*qY*qX*vY + vX*qX*qX - vX*qZ*qZ )
#define QUART_VEC_ROT_Y( vX, vY, vZ, q0, qX, qY, qZ )  ( q0*q0*vY - 2*q0*vZ*qX + 2*q0*vX*qZ + 2*vZ*qY*qZ + qY*qY*vY + 2*qY*vX*qX - qX*qX*vY - vY*qZ*qZ )
#define QUART_VEC_ROT_Z( vX, vY, vZ, q0, qX, qY, qZ )  (-qY*qY*vZ - 2*qY*vX*q0 + 2*qY*vY*qZ + 2*vX*qX*qZ - qX*qX*vZ + 2*qX*vY*q0 + q0*q0*vZ + vZ*qZ*qZ )

#endif /* QUART_H_ */
