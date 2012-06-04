
#include <stdlib.h>
#include <signal.h>

#include "rfComm.h"
#include "uartComm.h"
#include "../common.h"

//#define DEBUG 
#ifdef DEBUG
#define ERROR(X) TXString("\nERROR: " (X), 8+strlen(X));
#define PRINT(X) TXString("\nDEBUG: " (X), 8+strlen(X));
#else
#define ERROR(X)
#define PRINT()
#endif

extern volatile uint8_t newBSPacket;
extern volatile uint8_t newHeliPacket;

void init_device(); 
void toggleLED(uint8_t which); 

int main() {
   packet_t myPacket;

   init_device();

   while (1) {
      if (newBSPacket) {
         getBSPacket(&myPacket);
         sendToHeli(&myPacket);
         toggleLED(1); // green led
      }
      if (newHeliPacket) {
         getHeliPacket(&myPacket);
         sendToBS(&myPacket);
         toggleLED(2); // red led
      }
   }

   return 1; // shouldn't ever exit
}

void init_device() {
   BSP_Init();
   SMPL_Init(rxCallback);

   linkToHelicopter();
   init_HeliQueue();
   UART_Init();

   if (BSP_LED2_IS_ON())
      toggleLED(2);
   NWK_DELAY(200);

   __enable_interrupt();
}

void toggleLED(uint8_t which) {

   switch(which) {
   case 1:
      BSP_TOGGLE_LED1();
      break;
   case 2:
      BSP_TOGGLE_LED2();
      break;
   }
}
