#include "windowcontroller.h"

#include <esp32-hal-adc.h>
#include <esp32-hal-gpio.h>

hw_timer_t *timer;

bool buttonDownLongPressed = false;
bool buttonUpLongPressed = false;


float sampleMotorCurrent();

// -------- Window properties ----------------

bool isWindowClosed() {
    return digitalRead(PIN_WINDOW_CLOSED) == LOW;
}

bool isWindowOpening() {
    return sampleMotorCurrent() > 0;
}

bool isWindowClosing() {
    return sampleMotorCurrent() < 0;
}

// -------- Window functions ----------------

void doCloseWindow() {
    if (isWindowClosed()) {
        return;
    }
    digitalWrite(PIN_CLOSE_WINDOW, HIGH);
}

void doOpenWindow() {
    digitalWrite(PIN_OPEN_WINDOW, HIGH);
}


void doStopWindow() {
    digitalWrite(PIN_OPEN_WINDOW, LOW);
    digitalWrite(PIN_OPEN_WINDOW, LOW);
}


void doRewindWindow() {
    doStopWindow();
    doOpenWindow();
    delay(WINDOW_REWIND_TIME);
    doStopWindow();
}


// --------- Hardware functions //

void onWindowClosed() {
    FRAME frame;
    frame.id = WINDOW_MSG_CLOSED;
    frame.size = 2;
    frame.data[1] = isWindowClosed();
    sendMessageFromISR(&frame);
}


void onWindowMotorOverload() {
    bool isClosing = isWindowClosing();
    doStopWindow();
    if (isClosing && !isWindowClosed()) {
        // Window closing but not fully closed, assume its jammed
        FRAME frame;
        frame.id = WINDOW_MSG_JAMMED;
        frame.size = 1;
        sendMessageFromISR(&frame);
    }
}

// --- message functions ----


void onWindowOpenButton(FRAME *frame) {
    if (frame->size < 2) {
        return;
    }
    BUTTON_STATE state = Byte2ButtonState(frame->data[1]);

    switch (state) {
        case PRESSED: {
            // If already opening, make release stop window
            buttonDownLongPressed = isWindowOpening();
            doOpenWindow();
            break;
        }
        case RELEASED: {
            // Stop if button held down for more than x miliseconds, otherwise leave to fully open
            if (buttonDownLongPressed) {
                doStopWindow();
            }
            break;
        }

        case HELD: {
            buttonDownLongPressed = true;
        }
        case UNKNOWN:
            break;
    }
}

void onWindowCloseButton(FRAME *frame) {
    if (frame->size < 2) {
        return;
    }

    BUTTON_STATE state = Byte2ButtonState(frame->data[1]);
    switch (state) {
        case PRESSED: {
            buttonUpLongPressed = isWindowClosing();
            doCloseWindow();
            break;
        }
        case RELEASED: {
            if (buttonUpLongPressed) {
                doStopWindow();
            }
            break;
        }
        case HELD: {
            buttonUpLongPressed = true;
        }
        case UNKNOWN:
            break;
    }
}


//https://www.engineersgarage.com/acs712-current-sensor-with-arduino/
float sampleMotorCurrent() {
    float total = 0.0;
    for (int i = 0; i < CURRENT_SAMPLE_COUNT; i++) {
        total += analogRead(PIN_WINDOW_MOTOR_LOAD);
    }
    return CURRENT_REF_OFFSET - (total / CURRENT_SAMPLE_COUNT);
}

void IRAM_ATTR onTimer() {
    float current = sampleMotorCurrent();
    if (current > CURRENT_MAX || current < -CURRENT_MAX) {
        onWindowMotorOverload();
    }
}


void setupWindowController() {
    // Attch interrupt to window closed switch
    pinMode(PIN_WINDOW_CLOSED, INPUT_PULLUP);
    attachInterrupt(PIN_WINDOW_CLOSED, onWindowClosed, CHANGE);

    // Analgue read performed on timer for window motor current
    pinMode(PIN_WINDOW_MOTOR_LOAD, INPUT_PULLUP);
    timer = timerBegin(0, 0x0FFF, true);
    timerAttachInterrupt(timer, onTimer, true);
}

ERROR processWindowMessage(FRAME *frame) {
    switch (frame->id) {
        case WINDOW_MSG_CLOSED:
            doStopWindow();
            return 0;

        case WINDOW_MSG_JAMMED:
            doRewindWindow();
            return 0;

        case BUTTON_WINDOW_CLOSE:
            onWindowCloseButton(frame);
            return 0;

        case BUTTON_WINDOW_OPEN:
            onWindowOpenButton(frame);
            return 0;

        default:
            return -1;
    }
}
