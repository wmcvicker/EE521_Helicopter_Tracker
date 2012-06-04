

#include "rfComm.h"
#include <string.h>

static linkID_t myLinkID;
volatile uint8_t  newRepPacket = 0;

//extern uint8_t newUARTPACKET; // XXX DEBUGGING

volatile uint8_t rqueue[R_QUEUE_MAX];
volatile uint8_t rqueue_front;
volatile uint8_t rqueue_back;

static uint8_t rq_pop(); 
static uint8_t rq_size(); 
static uint8_t rq_front();
static void    rq_push(uint8_t item); 

extern void toggleLED(uint8_t which); 

void linkToRep() {

   // listen for link forever... 
   while (SMPL_SUCCESS != SMPL_LinkListen(&myLinkID)) {
      /* Implement fail-to-link policy here. otherwise, listen again. */
      toggleLED(2); // red led
   }

   /* turn on RX. default is RX off. */
   SMPL_Ioctl(IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);
}

void init_RepQueue() {

   // initialize the queue
   uint8_t i;
   for (i = 0; i < R_QUEUE_MAX; i++) 
      rqueue[i] = 0;
   rqueue_front = 0;
   rqueue_back = 0;
}

critical void sendToRep(packet_t *thePacket) {
   int i;

   for (i = 0; i < 10; i++) {
      if (SMPL_SUCCESS == SMPL_SendOpt(myLinkID, (uint8_t *) thePacket,
            thePacket->dlen + HEADER_SIZE, SMPL_TXOPTION_ACKREQ)) {
         break;
      }
   }
}

/* handle received messages */
uint8_t rxCallback(linkID_t port) {
   uint8_t len;
   packet_t thePacket;

   /* is the callback for the link ID we want to handle? */
   if (port == myLinkID) {
      if ((SMPL_SUCCESS == SMPL_Receive(myLinkID, (uint8_t *) &thePacket, 
            &len)) && len) {
         uint8_t plen = thePacket.dlen + HEADER_SIZE;
         uint8_t *packet_ptr = (uint8_t *) &thePacket;
         if (thePacket.dev_id != BS_ID)
            return 1;

         if (plen <= len) {
            // add to the queue
            uint8_t i;
            for (i = 0; i < plen; i++)
               rq_push(packet_ptr[i]);
            newRepPacket++;
         }

         // drop frame. we're done with it. 
         return 1;
      }
   }

   /* keep frame for later handling */
   return 0;
}

critical void getRepPacket(packet_t *thePacket) {
   uint8_t plen, i;
   uint8_t * packet_ptr = (uint8_t *) thePacket;
  
   plen = rq_front() + HEADER_SIZE;
   for (i = 0; i < plen; i++) {
      packet_ptr[i] = rq_pop();
   }
   newRepPacket--;


   // XXX DEBUGGING
   //newUARTPacket++;
}

/*-----------------------------------------------------------------------------
 * Receive Queue Functions
 *  - uint8_t rq_pop()
 *  - void rq_push(char)
 *  - uint8_t rq_size()
/----------------------------------------------------------------------------*/
static uint8_t rq_pop() {
   uint8_t val;

   // Make sure there are elements in the queue
   if (rq_size() <= 0) 
      return 0;

   val = rqueue[rqueue_front];

   if (rqueue_front < (R_QUEUE_MAX - 1)) 
      rqueue_front++;
   else
      rqueue_front = 0;

   return val;
}

/* If the queue is full, we will wrap around and write over the data.
 *
 * @param item the item to insert into the queue
 */
static void rq_push(uint8_t item) {

   rqueue[rqueue_back] = item;

   if (rqueue_back < (R_QUEUE_MAX - 1))
      rqueue_back++;
   else
      rqueue_back = 0;

   // check if we overflowed
   if (rq_size() == 0) {
      // discard the item in the front of the queue for now
      // TODO: Need to handle this by doubling our queue size
      if (rqueue_front < (R_QUEUE_MAX - 1))
         rqueue_front++;
      else
         rqueue_front = 0;
   }
}

static uint8_t rq_size() {
   uint8_t diff;

   if (rqueue_front >= rqueue_back)
      diff = rqueue_front - rqueue_back;
   else
      diff = rqueue_back - rqueue_front;
  
   return diff;
}

static uint8_t rq_front() {

   return rqueue[rqueue_front];
}
