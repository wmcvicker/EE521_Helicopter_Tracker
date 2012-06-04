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
#include "uartComm.h"
#include "../common.h"

#define RUDDER_MILD_LEFT 1438
#define RUDDER_NORMAL 1659
#define RUDDER_MILD_RIGHT 1880
#define RUDDER_FULL_LEFT 1107     // 1ms - 1.001ms exact

#define AILERON_MILD_RIGHT 1880
#define AILERON_MILD_LEFT 1438
#define AILERON_NORMAL 1659

#define THROTTLE_OFF 1107         // 1ms - 1.001ms exact

#define ELEVATOR_UP 1770
#define ELEVATOR_DOWN 1549
#define ELEVATOR_NORMAL 1659

void init_pwm(void);
void testfunction_pwm_change(int dummydata);
void throttle_test(void);
void delay(int ms);
void drift(char x, char y, char z, int current_heading, int desired_heading);

#endif
