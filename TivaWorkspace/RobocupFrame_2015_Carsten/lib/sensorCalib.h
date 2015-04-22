/*
 * sensorCalib.c
 *
 *  Created on: 03/04/2014
 *      Author: Soren
 */

#ifndef CALIB_H_
#define CALIB_H_

#include <stdint.h>

static int32_t lineSensorOffset[6] =
{ 247, 280, 215, 182, 297, 94 };
static float lineSensorScale[6] =
{ 2574-247, 2953-280, 2541-215, 2156-182, 2733-297, 1369-94};

static float magOffset[3] =
{ -738.4467, 80.5892, -41.3577 }; // -285.8069   61.1669  -75.4999
static float magScale[3] = // Scales so std magnetic strength is 1000
{ 2.2716, 1.8373, 2.1523 }; //  1.7108    1.6886    1.9556

static float accOffset[3] =
{ 26.7163, 72.1377, -174.3954 };
static float accScale[3] =
{ 0.4674, 0.4598, 0.4834 };

static float gyroOffset[3] =
{ -17.6413, 8.2700, -76.4439 };
static float gyroScale[3] =
{  3.0543e-04, 3.0543e-04, 3.0543e-04 }; // in rad/s

static float distOffset[2] =
{ -2.41557382, -0.6477724 };
static float distScale[2] =
{ 1.99449955E+04, 1.71013659e+04 };
/* from
0	1416       32
1	1161     3130
2	1023     2620
3	1482     3845
4	1175     3573
5	1083     2967
6	 790     2563
7	1168     2249
8	 833     2022
9	1004     1789
10	1232     1602
11	1572     1468
12	1124     1324
13	1191     1253
14	 997     1180
15	 994     1103
16	1142     1038
17	1432      967
18	 799      913
19	1296      870
20	1021      812
21	1092      790
22	 803      764
23	1079      740
24	 840      691
25	 783      643
26	 995      648
27	1179      604
28	 909      599
29	1401      598
30	1044      555
 */
#endif
