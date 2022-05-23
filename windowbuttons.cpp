/**
 * window buttons monitors two pins linked to the physical window buttons.
 * When those pins go LOW, it signals the button is pressed and vice versa.
 * Buttons send out a state:
 * PRESSED
 * RELEASED
 * HELD
 * Initially PRESSED will be sent.  If the button is held for more than the delay time
 * a HELD message is sent. HELD messages continue to be sent until the RELEASE message is sent, indicating the button is
 * back to its original, unpressed state.
 */

#include "windowbuttons.h"

#include <esp32-hal-adc.h>

hw_timer_t *timerButtonOpen;
hw_timer_t *timerButtonClose;

// Button States //
BUTTON_STATE windowCloseButtonState() {
    return stateFromPin(digitalRead(PIN_WINDOW_BUTTON_CLOSE));
}

BUTTON_STATE windowOpenButtonState() {
    return stateFromPin(digitalRead(PIN_WINDOW_BUTTON_OPEN));
}


// -- Timer induced events --

void IRAM_ATTR onWindowsButtonOpenHeld() {
    if (windowOpenButtonState() == PRESSED) {
        struct FRAME frame;
        frame.id = BUTTON_WINDOW_OPEN;
        frame.size = 2;
        frame.data[1] = HELD;
        sendMessageFromISR(&frame);
    } else {
        timerEnd(timerButtonOpen);
    }
}

void IRAM_ATTR onWindowsButtonCloseHeld() {
    if (windowCloseButtonState() == PRESSED) {
        struct FRAME frame;
        frame.id = BUTTON_WINDOW_CLOSE;
        frame.size = 2;
        frame.data[1] = HELD;
        sendMessageFromISR(&frame);
    } else {
        timerEnd(timerButtonClose);
    }
}



// --------- Hardware functions //

BUTTON_STATE startButtonOpen() {
    BUTTON_STATE state = windowOpenButtonState();
    if (state == PRESSED) {
        if (timerStarted(timerButtonOpen)) {
            timerRestart(timerButtonOpen);
        } else {
            timerStart(timerButtonOpen);
        }
    } else { // RELEASED
        if (timerStarted(timerButtonOpen)) {
            timerStop(timerButtonOpen);
        }
    }
    return state;
}

BUTTON_STATE startButtonClose() {
    BUTTON_STATE state = windowCloseButtonState();
    if (state == PRESSED) {
        if (timerStarted(timerButtonClose)) {
            timerRestart(timerButtonClose);
        } else {
            timerStart(timerButtonClose);
        }
    } else { // RELEASED
        if (timerStarted(timerButtonClose)) {
            timerStop(timerButtonClose);
        }
    }
    return state;
}

void onWindowButtonClose() {
    BUTTON_STATE state = startButtonClose();
    FRAME frame;
    frame.id = BUTTON_WINDOW_CLOSE;
    frame.size = 2;
    frame.data[1] = state;
    sendMessageFromISR(&frame);
}

void onWindowButtonOpen() {
    BUTTON_STATE state = startButtonOpen();
    FRAME frame;
    frame.id = BUTTON_WINDOW_OPEN;
    frame.size = 2;
    frame.data[1] = state;
    sendMessageFromISR(&frame);
}


extern void setupWindowButtons() {
    // Attach hardware interrupt
    pinMode(PIN_WINDOW_BUTTON_OPEN, INPUT_PULLUP);
    attachInterrupt(PIN_WINDOW_BUTTON_OPEN, onWindowButtonOpen, CHANGE);

    pinMode(PIN_WINDOW_BUTTON_CLOSE, INPUT_PULLUP);
    attachInterrupt(PIN_WINDOW_BUTTON_CLOSE, onWindowButtonClose, CHANGE);

    timerButtonOpen = timerBegin(0, 0xFF, true);
    timerStop(timerButtonOpen);
    timerAttachInterrupt(timerButtonOpen, onWindowsButtonOpenHeld, true);

    timerButtonClose = timerBegin(0, 0xFF, true);
    timerStop(timerButtonClose);
    timerAttachInterrupt(timerButtonClose, onWindowsButtonCloseHeld, true);
}
