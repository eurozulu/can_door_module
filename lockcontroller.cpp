
#include "lockcontroller.h"


// -------- Door properties ----------------

bool isDoorClosed() {
    return digitalRead(PIN_DOOR_CLOSED) == LOW;
}

bool isDoorLocked() {
    return digitalRead(PIN_DOOR_LOCKED) == LOW;
}


// -------- door functions ----------------

ERROR doLockDoor() {
    if (!isDoorClosed()) {
        return ERR_LOCK_FAILED_DOOR_OPEN;
    }
    digitalWrite(PIN_LOCK_DOOR, HIGH);
    return 0;
}

ERROR doUnlockDoor() {
    digitalWrite(PIN_UNLOCK_DOOR, HIGH);
    return 0;
}


void clickButton(MESSAGE_ID id, byte modId) {
    FRAME frame;
    frame.id = id;
    frame.size = 2;
    frame.data[0] = modId;
    frame.data[1] = PRESSED;
    sendMessage(&frame);
    frame.data[1] = RELEASED;
    sendMessage(&frame);
}

void stopButton(byte modId) {
    FRAME frame;
    frame.id = BUTTON_WINDOW_CLOSE;
    frame.size = 2;
    frame.data[0] = modId;
    frame.data[1] = RELEASED;
    sendMessage(&frame);
}


void doFullOpen() {
    clickButton(BUTTON_WINDOW_OPEN, 0);
}

void doFullClose() {
    clickButton(BUTTON_WINDOW_CLOSE, 0);
}

void doFullStop() {
    stopButton(0);
}

// --------- Hardware functions //

void onDoorClosed() {
    FRAME frame;
    frame.id = DOOR_MSG_CLOSED;
    frame.size = 2;
    frame.data[1] = isDoorClosed();
    sendMessageFromISR(&frame);
}


void onDoorLocked() {
    FRAME frame;
    frame.id = DOOR_MSG_LOCKED;
    frame.size = 2;
    frame.data[1] = isDoorLocked();
    sendMessageFromISR(&frame);
}

// --- message functions ----

ERROR onLockButton(BUTTON_STATE state) {
    switch (state) {
        case PRESSED: {
            return doLockDoor();
        }
        case RELEASED: {
            doFullStop();
            return 0;
        }
        case HELD: {
            doFullClose();
            return 0;
        }
        default: // UNKNOWN:
            return ERR_BUTTON_STATE_UNKNOWN;
    }
}

ERROR onUnlockButton(BUTTON_STATE state) {
    switch (state) {
        case PRESSED: {
            return doUnlockDoor();
        }
        case RELEASED: {
            return 0;
        }
        case HELD: {
            doFullOpen();
            return 0;
        }
        default: // UNKNOWN:
            return ERR_BUTTON_STATE_UNKNOWN;
    }
}


void setupLockController() {
    // Attch interrupt to door closed switch
    pinMode(PIN_DOOR_CLOSED, INPUT_PULLUP);
    attachInterrupt(PIN_DOOR_CLOSED, onDoorClosed, CHANGE);

    pinMode(PIN_DOOR_LOCKED, INPUT_PULLUP);
    attachInterrupt(PIN_DOOR_LOCKED, onDoorLocked, CHANGE);
}

ERROR processLockMessage(FRAME *frame) {
    switch (frame->id) {
        case BUTTON_DOOR_LOCK:
            if (frame->size < 2) {
                return ERR_MESSAGE_DATA_TOO_SHORT;
            }
            return onLockButton(Byte2ButtonState(frame->data[1]));
        case BUTTON_DOOR_UNLOCK:
            if (frame->size < 2) {
                return ERR_MESSAGE_DATA_TOO_SHORT;
            }
            return onUnlockButton(Byte2ButtonState(frame->data[1]));
        default:
            return -1;
    }
}
