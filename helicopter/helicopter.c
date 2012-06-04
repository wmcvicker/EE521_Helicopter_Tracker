
#include <msp430f2274.h>
#include <signal.h>

#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"

#include "rfComm.h"
#include "../common.h"
#include "pwm.h"

extern volatile uint8_t newRepPacket;
extern volatile uint8_t newUARTPacket;


void initialize();
void toggleLED(uint8_t which);

int main (void) {
   volatile uint8_t _state = IDLE_S;
   packet_t thePacket;

   initialize();

   while (1) {
      switch (_state) {
      case IDLE_S:

         if (newRepPacket)
            _state = GET_RF_S;
         else if (newUARTPacket)
            _state = GET_UART_S;

         break;
      case GET_UART_S:
         do {
            getUARTPacket(&thePacket);
            toggleLED(2); // red led
            if (thePacket.ptype == GPS_DATA_T) {
               sendToRep(&thePacket);
            }
         } while (newUARTPacket);

         if (newRepPacket)
            _state = GET_RF_S;
         else
            _state = IDLE_S;
         break;
      case GET_RF_S:
         getRepPacket(&thePacket);
         toggleLED(1); // green led

         if (newUARTPacket)
            _state = GET_UART_S;
         else
            _state = IDLE_S;
         break;
      default:
         _state = IDLE_S;
      }
   }

   return 1; // should never return
}

void initialize() {
   BSP_Init();
   SMPL_Init(rxCallback);

   linkToRep();
   init_RepQueue();
   UART_Init();
//   init_pwm();

   // Indicate the board is up
   if (BSP_LED2_IS_ON())
      toggleLED(2);
   NWK_DELAY(200);

   __enable_interrupt();
}

void toggleLED(uint8_t which) {

   switch(which) {
   case 1 :
      BSP_TOGGLE_LED1();
      break;
   case 2 :
      BSP_TOGGLE_LED2();
      break;
   }
}
