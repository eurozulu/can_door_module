//
// Created by Rob GIlham on 22/05/2022.
//

#ifndef MODULE_CANQUEUE_H
#define MODULE_CANQUEUE_H


//#include <Arduino.h>
#include <stdint.h>
#include "messages.h"

#define MAX_BUFFER_INBOUND 128
#define MAX_BUFFER_OUTBOUND 64

struct FRAME {
    MESSAGE_ID id;
    uint8_t data[8];
    uint8_t size;
};

// send the given frame out to the bus
extern void sendMessage(struct FRAME *frame);

// send the given frame out to the bus from an interrupt service routine
extern void sendMessageFromISR(struct FRAME *frame);

// receive the next message from the bus.
// Blocks until a message is available or an error occurs
extern ERROR receiveMessage(struct FRAME *frame);

extern void startCanBus(const uint8_t csPin, uint8_t moduleId);

#endif
