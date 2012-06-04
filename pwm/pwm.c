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


/* 2000us = 2211
 * 1900us = 2100
 * 1800us = 1990
 * 1700us = 1880
 * 1600us = 1770
 * 1500us = 1659
 * 1400us = 1549
 * 1300us = 1438
 * 1200us = 1328
 * 1100us = 1217
 * 1000us = 1107
 * 900us = 996
 * 800us = 885
 * 700us = 775
 * 600us = 664
 * 500us = 553
 * 400us = 443
 * 300us = 332
 * 
 * upper case X == drifting positively
 * lower case x == drifting negatively
 * 
 * same for y and z ^^
 * 
 * two characters which are an upper A and lower A
 * divide by 10 after combining and that will be our value.
*/

extern uint8_t newUARTPacket;


void main(void)
{
  char x,y,z;
  int heading;
  int newUARTPacket;

  packet_t status_packet;
  
  init_pwm();
  UART_Init();

  while(1)
  {
  	if(newUARTPacket)
  	{
  	   getUARTPacket(&status_packet);
  	   if(status_packet.ptype == 2)
  	   {
  	   	  x = status_packet.data[0];
  	   	  y = status_packet.data[1];
  	   	  z = status_packet.data[2];
  	   	  heading = (status_packet.data[3] << 8) | (status_packet.data[4]);
  	   	  
  	   	  drift(x, y, z, heading, 0);
  	   	  
  	   }	
  	}
  }
  
}

void init_pwm(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P2DIR |= 0x18;                            // Pin 6 and 7 - P2.3 and P2.4
  P2SEL |= 0x18;                            // Pin 6 and 7 - P2.3 and P2.4

  P4DIR |= 0x30;							// Pin 9 and 10 - P4.4 and 4.5                           
  P4SEL |= 0x30;                            // Pin 9 and 10 - P4.4 and 4.5
  
  TACCR0 = 24144;                           // PWM Period - Pin 6 and 7 - 45 Hz
  TBCCR0 = 26844;                           // PWM Period - Pin 9 and 10 - 42 Hz
  
  TACTL = TASSEL_2 + MC_1;                  // SMCLK, up mode
  TBCTL = TBSSEL_2 + MC_1;                  // SMCLK, up mode 
}
/*
void testfunction_pwm_change(int dummydata)
{

  TACCTL1 = OUTMOD_7;                       // TACCR1 reset/set
  TACCR1 = 2211;                             // TACCR1 PWM duty cycle -- Pin 6
  TACCTL2 = OUTMOD_7;                       // TACCR2 reset/set
  TACCR2 = 1435;                             // TACCR2 PWM duty cycle - Pin 7 

  TBCCTL1 = OUTMOD_7;                       // TACCR1 reset/set
  TBCCR1 = 1730;                             // TACCR1 PWM duty cycle  -- Pin 9
  TBCCTL2 = OUTMOD_7;                       // TACCR2 reset/set
  TBCCR2 = 2000;                             // TACCR2 PWM duty cycle  -- Pin 10

 
  __bis_SR_register(CPUOFF);                // Enter LPM0
}

void throttle_test(void)
{
  volatile long i;
  	
  TACCTL1 = OUTMOD_7;  			                     // TACCR1 reset/set
  TACCR1 = THROTTLE_OFF;                             // TACCR1 PWM duty cycle -- Pin 6
  
  TACCTL2 = OUTMOD_7;                       // TACCR2 reset/set
  TACCR2 = RUDDER_OFF;                             // TACCR2 PWM duty cycle - Pin 7
   
  delay(2000);
  
  TACCTL1 = OUTMOD_7;  			                     // TACCR1 reset/set
  TACCR1 = RUDDER_MILD_LEFT;                             // TACCR1 PWM duty cycle -- Pin 6
  
  TACCTL2 = OUTMOD_7;                       // TACCR2 reset/set
  TACCR2 = RUDDER_MILD_LEFT;                             // TACCR2 PWM duty cycle - Pin 7
   
  delay(2000);
  
  TACCTL1 = OUTMOD_7;  			                     // TACCR1 reset/set
  TACCR1 = THROTTLE_OFF;                             // TACCR1 PWM duty cycle -- Pin 6
  
    TACCTL2 = OUTMOD_7;                       // TACCR2 reset/set
  TACCR2 = RUDDER_OFF;                             // TACCR2 PWM duty cycle - Pin 7 
  
  	
}
*/

/* Pin 6 - Throttle
 * Pin 7 - Elevator
 * Pin 9 - Aileron
 * Pin 10 - Rudder
 */
void drift(char x, char y, char z, int current_heading, int desired_heading)
{	
	// TEST THIS TO MAKE SURE
	if(x == 'X')
	{
	  //Postive Drift - Drifting Forward
	  TACCTL2 = OUTMOD_7;                       // TACCR2 reset/set
      TACCR2 = ELEVATOR_UP;                // TACCR2 PWM duty cycle - Pin 7
      
	}
	else if(x =='x')
	{
	  //negative drift, Drifting Backward
	  TACCTL2 = OUTMOD_7;
	  TACCR2 = ELEVATOR_DOWN;

	}
	
	if(y == 'Y')
	{
	  	TBCCTL1 = OUTMOD_7;                       // TACCR1 reset/set
  		TBCCR1 = AILERON_MILD_LEFT;                   // TACCR1 PWM duty cycle  -- Pin 9
                 // TACCR1 PWM duty cycle  -- Pin 9
  		
	}
	
	else if(y == 'y')
	{
		TBCCTL1 = OUTMOD_7;                       // TACCR1 reset/set
  		TBCCR1 = AILERON_MILD_RIGHT;                     // TACCR1 PWM duty cycle  -- Pin 9
                 // TACCR1 PWM duty cycle  -- Pin 9
	}	
	
	if(z == 'Z')
	{
	  	TBCCTL2 = OUTMOD_7;                       // TACCR1 reset/set
  		TBCCR2 = 0;                   // TACCR1 PWM duty cycle  -- Pin 10
               // TACCR1 PWM duty cycle  -- Pin 10
  		
	}
	
	else if(z == 'z')
	{
		TBCCTL2 = OUTMOD_7;       
  		TBCCR2 = 0;       
              
	}
	
	delay(500);
	
	TACCTL2 = OUTMOD_7;       
	TACCR2 = ELEVATOR_NORMAL;  
	
	TBCCTL1 = OUTMOD_7;       
	TBCCR1 = AILERON_NORMAL;  
	
	TBCCTL2 = OUTMOD_7;       
	TBCCR2 = 0;
	  
}

void delay(int ms)
{
   while(ms)
   {
      __delay_cycles(8000); // set for 16Mhz change it to 1000 for 1 Mhz
      ms--;
   }
}

