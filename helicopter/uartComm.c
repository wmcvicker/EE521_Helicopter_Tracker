

#include "uartComm.h"

volatile uint8_t uart_queue[UART_QUEUE_MAX];
volatile uint8_t uart_queue_front;
volatile uint8_t uart_queue_back;
volatile uint8_t _state = IDLE_S;

// Variables used to detect a new packet
volatile uint8_t newUARTPacket = 0;

// Receive Queue Functions
static uint8_t q_pop();
static void q_push(uint8_t item);
static uint8_t q_size();
static uint8_t q_front();

void UART_Init() {
   P3SEL |= 0x30;                            // P3.4,5 = USCI_A0 TXD/RXD
   UCA0CTL1 = UCSSEL_2;                      // SMCLK
   UCA0CTL0 = 0x00;

#if (BSP_CONFIG_CLOCK_MHZ_SELECT == 1)
   UCA0BR0 = 104;                            // 9600 from 1Mhz
   UCA0BR1 = 0;
   UCA0MCTL = UCBRS_1;
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 2)
   UCA0BR0 = 0xDA;                           // 9600 from 2Mhz
   UCA0BR1 = 0x0;
   UCA0MCTL = UCBRS_6;
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 4)
   UCA0BR0 = 0xA0;                           // 9600 from 4Mhz
   UCA0BR1 = 0x1;
   UCA0MCTL = UCBRS_6;
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 6)
   UCA0BR0 = 0x7B;                           // 9600 from 6Mhz
   UCA0BR1 = 0x2;
   UCA0MCTL = UCBRS_3;
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 8) 
   UCA0BR0 = 0x41;                           // 9600 from 8Mhz
   UCA0BR1 = 0x3;
   UCA0MCTL = UCBRS_2;
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 10)
   UCA0BR0 = 0x79;                           // 9600 from 10Mhz
   UCA0BR1 = 0x4;
   UCA0MCTL = UCBRS_7;
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 12)
   UCA0BR0 = 0xE2;                           // 9600 from 12Mhz
   UCA0BR1 = 0x4;
   UCA0MCTL = 0;
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 16)
   UCA0BR0 = 0x82;                           // 9600 from 16Mhz
   UCA0BR1 = 0x6;
   UCA0MCTL = UCBRS_6;
#else
#error "ERROR: Unsupported clock speed.  Custom clock speeds are possible. " \
   "See comments in code."
#endif


   // initialize the receive queue
   uint8_t i;
   for (i = 0; i < UART_QUEUE_MAX; i++)
      uart_queue[i] = 0;
   uart_queue_front = 0;
   uart_queue_back = 0;


   UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
   IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}

/*------------------------------------------------------------------------------
 * USCIA interrupt service routine
 ------------------------------------------------------------------------------*/
interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void) {
   uint8_t rx = UCA0RXBUF;
   static volatile uint8_t psize = 0;

   switch (_state) {
   case IDLE_S:
      // Get the first part of the preamble
      if (rx == PREAMBLE_A)
         _state = PREAMBLE_S;
      break;
   case PREAMBLE_S:
      if (rx == PREAMBLE_B)
         _state = GET_SIZE_S;
      else
         _state = IDLE_S;
      break;
   case GET_SIZE_S:
      psize = rx + HEADER_SIZE;
      _state = RECV_DATA_S;
   case RECV_DATA_S:
      // save the data in the queue
      q_push(rx);
      if (--psize == 0) {
         _state = IDLE_S;
         newUARTPacket++;
      }
   }
}

critical void getUARTPacket(packet_t *myPacket) {
   uint8_t *packet_ptr = (uint8_t *) myPacket;
   uint8_t plen, i;
   
   if (_state != IDLE_S)
      return;
   
   plen = q_front() + HEADER_SIZE;
   for (i = 0; i < plen; i++)
      packet_ptr[i] = q_pop();
   newUARTPacket--;
}

/*-----------------------------------------------------------------------------
 * Receive Queue Functions
 *  - uint8_t q_pop()
 *  - void q_push(char)
 *  - uint8_t q_size()
 /----------------------------------------------------------------------------*/
static uint8_t q_pop() {
   uint8_t val;

   // Make sure there are elements in the queue
   if (q_size() <= 0)
      return 0;

   val = uart_queue[uart_queue_front];

   if (uart_queue_front < (UART_QUEUE_MAX - 1))
      uart_queue_front++;
   else
      uart_queue_front = 0;

   return val;
}

/* If the queue is full, we will wrap around and write over the data.
 *
 * @param item the item to insert into the queue
 */
static void q_push(uint8_t item) {

   uart_queue[uart_queue_back] = item;

   if (uart_queue_back < (UART_QUEUE_MAX - 1))
      uart_queue_back++;
   else
      uart_queue_back = 0;

   // check if we overflowed
   if (q_size() == 0) {
      // discard the item in the front of the queue for now
      // TODO: Need to handle this by doubling our queue size
      if (uart_queue_front < (UART_QUEUE_MAX - 1))
         uart_queue_front++;
      else
         uart_queue_front = 0;
   }
}

static uint8_t q_size() {
   uint8_t diff;

   if (uart_queue_front >= uart_queue_back)
      diff = uart_queue_front - uart_queue_back;
   else
      diff = uart_queue_back - uart_queue_front;

   return diff;
}

static uint8_t q_front() {

   return uart_queue[uart_queue_front];
}
