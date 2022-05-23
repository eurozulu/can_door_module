//
// Created by Rob GIlham on 18/05/2022.
//

#include "messages.h"


BUTTON_STATE Byte2ButtonState(uint8_t state) {
    switch (state) {
        case 1:
            return PRESSED;
        case 2:
            return RELEASED;
        default:
            return UNKNOWN;
    }
}
uint8_t ButtonState2Byte(BUTTON_STATE state) {
    switch (state) {
        case PRESSED:
            return 1;
        case RELEASED:
            return 2;
        default:
            return 0;
    }
}

BUTTON_STATE stateFromPin(int pinValue) {
    switch (pinValue) {
        case LOW:
            return PRESSED;
        case HIGH:
            return RELEASED;
        default:
            return UNKNOWN;
    }
}
