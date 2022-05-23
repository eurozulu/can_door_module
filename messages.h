//
// Created by Rob GIlham on 18/05/2022.
//

#ifndef QUEUE_MESSAGES_H
#define QUEUE_MESSAGES_H

#include <stdint.h>
#include <esp32-hal-gpio.h>
#include <can.h>

enum BUTTON_STATE {
    UNKNOWN,
    PRESSED,
    RELEASED,
    HELD,
};


typedef uint8_t byte;
typedef unsigned long MESSAGE_ID;
typedef uint8_t ERROR;

const ERROR ERR_MESSAGE_READING_INBOUND_QUEUE = 4;
const ERROR ERR_MESSAGE_DATA_TOO_SHORT = 101;
const ERROR ERR_LOCK_FAILED_DOOR_OPEN = 51;
const ERROR ERR_BUTTON_STATE_UNKNOWN = 52;


BUTTON_STATE Byte2ButtonState(uint8_t state);
uint8_t ButtonState2Byte(BUTTON_STATE state);
BUTTON_STATE stateFromPin(int pinValue);

const canid_t WINDOW_MSG_JAMMED = 0x40;
const canid_t WINDOW_MSG_CLOSED = 0x41;
const canid_t WINDOW_MSG_STOPPED = 0x44;
const canid_t WINDOW_MSG_CLOSING = 0x45;
const canid_t WINDOW_MSG_OPENING = 0x46;

const canid_t BUTTON_WINDOW_OPEN = 0x4F;
const canid_t BUTTON_WINDOW_CLOSE = 0x4E;
const canid_t BUTTON_DOOR_LOCK = 0x4D;
const canid_t BUTTON_DOOR_UNLOCK = 0x4C;

const canid_t DOOR_MSG_CLOSED = 0x55;
const canid_t DOOR_MSG_LOCKED = 0x56;

#endif //QUEUE_MESSAGES_H
