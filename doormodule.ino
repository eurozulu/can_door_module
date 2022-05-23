
#ifndef MODULE_MAIN
#define MODULE_MAIN

#include "canqueues.h"
#include <stdint.h>


#include "windowbuttons.h"
#include "windowcontroller.h"
#include "lockcontroller.h"

const uint8_t PIN_CAN_CS = 10;
const uint8_t DOOR_MODULE_ID = 4;

void setup() {
    startCanBus(PIN_CAN_CS, DOOR_MODULE_ID);
    
    setupWindowController();
    setupWindowButtons();
    setupLockController();
}

bool printError(ERROR err) {
  if (err != 0) {
    printf("error %d reading message", err);
    return true;
  }
  return false;
}

void loop() {
  struct FRAME frame;

  ERROR err = receiveMessage(&frame);
  if (printError(err)) {
    return;
  }

  err = processWindowMessage(&frame);
  if (err >= 0) {
    printError(err);
    return;    
  }
  
  err = processLockMessage(&frame);
  if (err >= 0) {
    printError(err);
    return;    
  }

  printf("Unknown message %lu being ignored", frame.id);
  
}


#endif
