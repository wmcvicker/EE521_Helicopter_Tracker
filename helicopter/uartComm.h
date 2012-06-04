/* This file contains functions used to connect to the base station via the
 * UART serial protocol.
 *
 * @author William McVicker
 */

#ifndef UARTCOMM_H
#define UARTCOMM_H

#include <stdlib.h>
#include <msp430f2274.h>
#include <signal.h>
#include <string.h>

#include "bsp.h"
#include "../common.h"

#define UART_QUEUE_MAX 135


/****** UART Communication States *******/
#define IDLE_S      1 
#define PREAMBLE_S  2
#define GET_SIZE_S  4 
#define RECV_DATA_S 5

#define PREAMBLE_A 0xAA
#define PREAMBLE_B 0x55

void UART_Init(void);
void TXString(char *string, uint32_t length);
void sendToRep(packet_t *thePacket); 
critical void getUARTPacket(packet_t *myPacket); 
interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void);

#endif
