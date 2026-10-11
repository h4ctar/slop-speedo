#ifndef __BUTTON_H__
#define __BUTTON_H__

typedef enum {
  BUTTON_NONE,
  BUTTON_SHORT,
  BUTTON_LONG,
} ButtonEvent;

// Call pollButton() from loop(); it returns at most one event per press:
// BUTTON_SHORT on a quick press (fired when the button is released), and
// BUTTON_LONG the moment a press has been held for LONG_PRESS_MS. Requires
// the pin to idle high (pull-up) with the switch pulling it to ground.
void initButton();
ButtonEvent pollButton();

#endif
