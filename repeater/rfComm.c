/* This file contains functions used to connect to the helicopter
 * via the SimpliciTI RF protocol.
 *
 * @author William McVicker
 */

#include "rfComm.h"
#include <string.h>

static linkID_t myLinkID;
volatile uint8_t newHeliPacket = 0;

volatile uint8_t hqueue[H_QUEUE_MAX];
volatile uint8_t hqueue_front;
volatile uint8_t hqueue_back;

static uint8_t hq_pop();
static uint8_t hq_size();
static uint8_t hq_front();
static void    hq_push(uint8_t item);

extern void toggleLED(uint8_t which);

void linkToHelicopter() {

   // connect to the helicopter (MSP430)
   while (SMPL_SUCCESS != SMPL_Link(&myLinkID)) {
      toggleLED(2);  // toggle the red LED
      NWK_DELAY(100);
   }

   // turn on RX
   SMPL_Ioctl(IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);
}

void init_HeliQueue() {

   // initialize the receive queue
   uint8_t i;
   for (i = 0; i < H_QUEUE_MAX; i++)
      hqueue[i] = 0;
   hqueue_front = 0;
   hqueue_back = 0;
}

void sendToHeli(packet_t *thePacket) {
   int i;

   for (i = 0; i < 10; i++) {
      // Try to send the packet 10 times
      if (SMPL_SUCCESS == SMPL_SendOpt(myLinkID, (uint8_t *) thePacket,
               thePacket->dlen + HEADER_SIZE, SMPL_TXOPTION_ACKREQ)) {
         break;
      }
   }
   if (i == 10) {
      __disable_interrupt();
      SMPL_Unlink(myLinkID);
      linkToHelicopter();
      __enable_interrupt();
   }
   
}

// Handle received frames
uint8_t rxCallback(linkID_t port) {
   uint8_t len;
   packet_t thePacket;


   // Is the callback for the link ID we want to handle?
   if (port == myLinkID) {
      if ((SMPL_SUCCESS == SMPL_Receive(myLinkID, (uint8_t *) &thePacket,
                  &len)) && len) {
         uint8_t *packet_ptr = (uint8_t *) &thePacket;
         uint8_t plen = thePacket.dlen + HEADER_SIZE;
         if (thePacket.dev_id != HELICOPTER_ID)
            return 1;

         if (plen <= len) {
            // add packet to the queue
            uint8_t i;
            for (i = 0; i < plen; i++)
               hq_push(packet_ptr[i]);
            newHeliPacket++;
         }

         // drop frame. We're done with it
         return 1;
      }
   }

   // keep frame for later handling
   return 0;
}

critical void getHeliPacket(packet_t *myPacket) {
   uint8_t *packet_ptr = (uint8_t *) myPacket;
   uint8_t plen, i;


   plen = hq_front() + HEADER_SIZE;
   for (i = 0; i < plen; i++)
      packet_ptr[i] = hq_pop();
   newHeliPacket--;
}

/*-----------------------------------------------------------------------------
 * Receive Queue Functions
 *  - uint8_t hhq_pop()
 *  - void hq_push(char)
 *  - uint8_t hq_size()
 /----------------------------------------------------------------------------*/
static uint8_t hq_pop() {
   uint8_t val;

   // Make sure there are elements in the queue
   if (hq_size() <= 0)
      return 0;

   val = hqueue[hqueue_front];

   if (hqueue_front < (H_QUEUE_MAX - 1))
      hqueue_front++;
   else
      hqueue_front = 0;

   return val;
}

/* If the queue is full, we will wrap around and write over the data.
 *
 * @param item the item to insert into the queue
 */
static void hq_push(uint8_t item) {

   hqueue[hqueue_back] = item;

   if (hqueue_back < (H_QUEUE_MAX - 1))
      hqueue_back++;
   else
      hqueue_back = 0;

   // check if we overflowed
   if (hq_size() == 0) {
      // discard the item in the front of the queue for now
      // TODO: Need to handle this by doubling our queue size
      if (hqueue_front < (H_QUEUE_MAX - 1))
         hqueue_front++;
      else
         hqueue_front = 0;
   }
}

static uint8_t hq_size() {
   uint8_t diff;

   if (hqueue_front >= hqueue_back)
      diff = hqueue_front - hqueue_back;
   else
      diff = hqueue_back - hqueue_front;

   return diff;
}

static uint8_t hq_front() {

   return hqueue[hqueue_front];
}
