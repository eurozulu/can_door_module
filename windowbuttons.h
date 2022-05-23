#ifndef WINDOW_BUTTONS
#define WINDOW_BUTTONS

#include "canqueues.h"


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

const uint8_t PIN_WINDOW_BUTTON_CLOSE = 1;
const uint8_t PIN_WINDOW_BUTTON_OPEN = 2;


extern void setupWindowButtons();

#endif
