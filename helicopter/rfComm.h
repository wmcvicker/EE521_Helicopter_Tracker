
#ifndef RFCOMM_H
#define RFCOMM_H

#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"


#include <msp430f2274.h>
#include <signal.h>

#include "../common.h"

#define R_QUEUE_MAX 135

void linkToRep();
void sendToRep(packet_t *thePacket);
uint8_t rxCallback(linkID_t);
critical void getRepPacket(packet_t *thePacket);
void init_ReqQueue();

#endif
