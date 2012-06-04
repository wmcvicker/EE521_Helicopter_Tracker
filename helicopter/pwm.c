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

#include "pwm.h"

extern void toggleLED(uint8_t which);

void init_pwm(void)
{
//   WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

   P4DIR |= 0x10;                            // Pin 9 and 10 - P4.4 and 4.5
   P4SEL |= 0x10;                            // Pin 9 and 10 - P4.4 and 4.5

   BCSCTL2 |= 0x06;

   TBCCR0 = 24844;                           // PWM Period - Pin 9 and 10 - 42 Hz

   TBCTL = TBSSEL_2 + MC_1;                  // SMCLK, up mode


   TBCCTL1 = OUTMOD_7;                       // TACCR2 reset/set
   TBCCR1 = 1500;                             // TACCR2 PWM duty cycle - Pin 9
}

/* Turns left or right based on the number given. 0-4 turn left, 5 remain constant, 6-10 turn right.
 * The number passed to us tells us how hard we will be turning.
 *
 * @param dir_strength the direction and strength of the turn we are doing.
 */
void Turn(uint8_t dir_strength)
{
   int turn_strength = 0;
   switch(dir_strength)
   {
   case(0):
      turn_strength = RUDDER_LEFT_STR_ONE;
      break;
   case(1):
      turn_strength = RUDDER_LEFT_STR_TWO;
      break;
   case(2):
      turn_strength = RUDDER_LEFT_STR_THREE;
      break;
   case(3):
      turn_strength = RUDDER_LEFT_STR_FOUR;
      break;
   case(4):
      turn_strength = RUDDER_LEFT_STR_FIVE;
      break;
   case(5):
      turn_strength = RUDDER_NORMAL;
      break;
   case(6):
      turn_strength = RUDDER_RIGHT_STR_ONE;
      break;
   case(7):
      turn_strength = RUDDER_RIGHT_STR_TWO;
      break;
   case(8):
      turn_strength = RUDDER_RIGHT_STR_THREE;
      break;
   case(9):
      turn_strength = RUDDER_RIGHT_STR_FOUR;
      break;
   case(10):
      turn_strength = RUDDER_RIGHT_STR_FIVE;
      break;
   default:
      turn_strength = RUDDER_NORMAL;
      break;
   }

   TBCCTL2 = OUTMOD_1;                                 // TACCR2 reset/set
   TBCCR2 = turn_strength;                             // TACCR2 PWM duty cycle  -- Pin 10
   delay(500);
   TBCCTL2 = OUTMOD_1;
   TBCCR2= RUDDER_NORMAL;
}

void delay(int ms)
{
   /*   while(ms)
        {
        __delay_cycles(8000); // set for 16Mhz change it to 1000 for 1 Mhz
        ms--;
        }*/
   NWK_DELAY(ms);
}
