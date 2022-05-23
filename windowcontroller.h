#ifndef WINDOW_CONTROLLER
#define WINDOW_CONTROLLER

#include "canqueues.h"


// Switch closed when window fully closed
const uint8_t PIN_WINDOW_CLOSED = 1;

// Pin to sample window motor current
const uint8_t PIN_WINDOW_MOTOR_LOAD = 2;

// Control pin for Open window motor
const uint8_t PIN_OPEN_WINDOW = 3;
// Control pin for Close window motor
const uint8_t PIN_CLOSE_WINDOW = 4;

// Milliseconds to run open window motor for during rewind (post jam)
const int WINDOW_REWIND_TIME = 250;

const int CURRENT_SAMPLE_COUNT = 60;
const float CURRENT_REF_OFFSET = 2.5;
const float CURRENT_MAX = 2;

void setupWindowController();

ERROR processWindowMessage(FRAME *frame);

#endif
