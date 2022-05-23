#ifndef LOCK_BUTTONS
#define LOCK_BUTTONS

#include <esp32-hal-adc.h>
#include "canqueues.h"

/**
 * lock buttons monitors two pins linked to the physical lock buttons.
 * When those pins go LOW, it signals the button is pressed and vice versa.
 * Buttons send out a state:
 * PRESSED
 * RELEASED
 * HELD
 * Initially PRESSED will be sent.  If the button is held for more than the delay time
 * a HELD message is sent. HELD messages continue to be sent until the RELEASE message is sent, indicating the button is
 * back to its original, unpressed state.
 */

const uint8_t PIN_LOCK_DOOR_BUTTON = 1;
const uint8_t PIN_UNLOCK_DOOR_BUTTON = 2;

hw_timer_t *timerButtonLock;
hw_timer_t *timerButtonUnlock;


// Button States //

BUTTON_STATE lockDoorButtonState() {
    return stateFromPin(digitalRead(PIN_LOCK_DOOR_BUTTON));
}

BUTTON_STATE unlockDoorButtonState() {
    return stateFromPin(digitalRead(PIN_UNLOCK_DOOR_BUTTON));
}


// -- Timer induced events --

void IRAM_ATTR onLockDoorButtonHeld() {
    if (lockDoorButtonState() == PRESSED) {
        FRAME frame;
        frame.id = BUTTON_DOOR_LOCK;
        frame.size = 2;
        frame.data[1] = HELD;
        sendMessageFromISR(&frame);
    } else {
        timerEnd(timerButtonLock);
    }
}

void IRAM_ATTR onUnlockDoorButtonHeld() {
    if (unlockDoorButtonState() == PRESSED) {
        FRAME frame;
        frame.id = BUTTON_DOOR_UNLOCK;
        frame.size = 2;
        frame.data[1] = HELD;
        sendMessageFromISR(&frame);
    } else {
        timerEnd(timerButtonLock);
    }
}


// --------- Hardware functions //

BUTTON_STATE startButtonLock() {
    BUTTON_STATE state = lockDoorButtonState();
    if (state == PRESSED) {
        if (timerStarted(timerButtonLock)) {
            timerRestart(timerButtonLock);
        } else {
            timerStart(timerButtonLock);
        }
    } else { // RELEASED
        if (timerStarted(timerButtonLock)) {
            timerStop(timerButtonLock);
        }
    }
    return state;
}

BUTTON_STATE startButtonUnlock() {
    BUTTON_STATE state = unlockDoorButtonState();
    if (state == PRESSED) {
        if (timerStarted(timerButtonUnlock)) {
            timerRestart(timerButtonUnlock);
        } else {
            timerStart(timerButtonUnlock);
        }
    } else { // RELEASED
        if (timerStarted(timerButtonUnlock)) {
            timerStop(timerButtonUnlock);
        }
    }
    return state;
}

void onLockButton() {
    BUTTON_STATE state = startButtonLock();
    FRAME frame;
    frame.id = BUTTON_DOOR_LOCK;
    frame.size = 2;
    frame.data[1] = state;
    sendMessageFromISR(&frame);
}

void onUnlockButton() {
    BUTTON_STATE state = startButtonUnlock();
    FRAME frame;
    frame.id = BUTTON_DOOR_UNLOCK;
    frame.size = 2;
    frame.data[1] = state;
    sendMessageFromISR(&frame);
}


void setupLockButtons() {
    // Attach hardware interrupt
    pinMode(PIN_UNLOCK_DOOR_BUTTON, INPUT_PULLUP);
    attachInterrupt(PIN_UNLOCK_DOOR_BUTTON, onUnlockButton, CHANGE);

    pinMode(PIN_LOCK_DOOR_BUTTON, INPUT_PULLUP);
    attachInterrupt(PIN_LOCK_DOOR_BUTTON, onLockButton, CHANGE);

    timerButtonLock = timerBegin(0, 0xFF, true);
    timerStop(timerButtonLock);
    timerAttachInterrupt(timerButtonLock, onLockDoorButtonHeld, true);

    timerButtonUnlock = timerBegin(0, 0xFF, true);
    timerStop(timerButtonUnlock);
    timerAttachInterrupt(timerButtonUnlock, onUnlockDoorButtonHeld, true);
}

#endif
