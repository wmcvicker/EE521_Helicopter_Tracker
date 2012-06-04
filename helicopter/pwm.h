//******************************************************************************
//  MSP430F22x4 PWM Signals - Timer_A and Timer_B PWM generation on 4 pins.
//
//  Description: This program generates two PWM outputs on P1.2,3 using
//  Timer_A configured for up mode.
//
//  A. Dannenberg
//  Texas Instruments Inc.
//  April 2006
//  Built with CCE Version: 3.2.0 and IAR Embedded Workbench Version: 3.41A
//
//  Modified by William McVicker, Cameron Nouri, Kenneth Chee, Danny Kassen, and Brian Wihl
//  California Polytechnic State University
//******************************************************************************

#ifndef PWM_H
#define PWN_H

#include <msp430f2274.h>
#include <stdint.h>
#include "mrfi.h"
#include "nwk_types.h"
#include "../common.h"

#define RUDDER_MILD_LEFT 1250
#define RUDDER_NORMAL 1500
#define RUDDER_MILD_RIGHT 1750 
#define RUDDER_FULL_LEFT 1000

/* The rudder strenghts             
 *------------------------------------*/
#define RUDDER_RIGHT_STR_ONE 1600
#define RUDDER_RIGHT_STR_TWO 1700
#define RUDDER_RIGHT_STR_THREE 1800
#define RUDDER_RIGHT_STR_FOUR 1900
#define RUDDER_RIGHT_STR_FIVE 2000

#define RUDDER_LEFT_STR_ONE 1400
#define RUDDER_LEFT_STR_TWO 1300
#define RUDDER_LEFT_STR_THREE 1200
#define RUDDER_LEFT_STR_FOUR 1100
#define RUDDER_LEFT_STR_FIVE 1000
/*------------------------------------*/


#define AILERON_MILD_RIGHT 1750 
#define AILERON_MILD_LEFT 1250
#define AILERON_NORMAL 1500

#define THROTTLE_OFF 1107         // 1ms - 1.001ms exact

#define ELEVATOR_UP 1600
#define ELEVATOR_DOWN 1400
#define ELEVATOR_NORMAL 1500

/* 2000us = 2000
 * 1900us = 1900
 * 1800us = 1800
 * 1700us = 1700 
 * 1600us = 1600
 * 1500us = 1500
 * 1400us = 1400
 * 1300us = 1300
 * 1200us = 1200
 * 1100us = 1100
 * 1000us = 1000
 * 
 * upper case X == drifting positively
 * lower case x == drifting negatively
 * 
 * same for y and z ^^
 * 
 * two characters which are an upper A and lower A
 * divide by 10 after combining and that will be our value.
*/

void init_pwm(void);
void testfunction_pwm_change(int dummydata);
void throttle_test(void);
void delay(int ms);
void drift(packet_t *status_packet, int desired_heading);
void Turn(uint8_t dir_strength);

#endif
