
#ifndef RFCOMM_H

#include <msp430f2274.h>
#include <signal.h>

#include "../common.h"

#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"

#define H_QUEUE_MAX 135

void linkToHelicopter();
void sendToHeli(packet_t *thePacket); 
uint8_t rxCallback(linkID_t port); 
critical void getHeliPacket(packet_t *thePacket); 
void init_HeliQueue(); 

#endif
