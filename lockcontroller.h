#ifndef LOCK_CONTROLLER
#define LOCK_CONTROLLER

#include <esp32-hal-gpio.h>
#include "canqueues.h"

// Switch closed when door closed
const uint8_t PIN_DOOR_CLOSED = 1;

// Switch closed when door locked
const uint8_t PIN_DOOR_LOCKED = 2;

// Control pin for door lock motor
const uint8_t PIN_LOCK_DOOR = 3;
// Control pin for door unlock motor
const uint8_t PIN_UNLOCK_DOOR = 4;

void setupLockController();

ERROR processLockMessage(FRAME *frame);

#endif
