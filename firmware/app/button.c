#include "button.h"
#include "main.h"
#include <stdbool.h>

#define DEBOUNCE_MS 25
#define LONG_PRESS_MS 800

// Time-based state machine, driven by button_poll() on every loop() pass. The
// loop usually spins fast, but it can stall for tens of ms (OLED flush) — all
// decisions use HAL_GetTick() deltas, so a stall can delay but never corrupt
// an event.
//
//   IDLE        released, stable.
//   DOWN        saw the pin low, waiting out the debounce window.
//   HELD        debounced press; short fires on release, long fires the
//               moment the hold crosses LONG_PRESS_MS.
//   RELEASE     saw the pin high, waiting out the debounce window before
//               settling back to IDLE (and firing the short, if no long).
void button_init() {
}

ButtonEvent button_poll() {
  static enum { IDLE, DOWN, HELD, HELD_LONG, RELEASE } state = IDLE;
  static uint32_t state_since_ms = 0;
  static bool long_fired = false;

  // MODE button on PA6: external switch to GND, internal pull-up (CubeMX).
  bool pressed = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET;
  uint32_t now = HAL_GetTick();

  switch (state) {
  case IDLE:
    if (pressed) {
      state = DOWN;
      state_since_ms = now;
      long_fired = false;
    }
    break;
  case DOWN:
    if (!pressed) {
      state = IDLE;
    } else if (now - state_since_ms >= DEBOUNCE_MS) {
      state = HELD;
    }
    break;
  case HELD:
    if (now - state_since_ms >= LONG_PRESS_MS) {
      long_fired = true;
      state = HELD_LONG; // long fires without waiting for release
      return BUTTON_LONG;
    }
    if (!pressed) {
      state = RELEASE;
      state_since_ms = now;
    }
    break;
  case HELD_LONG:
    if (!pressed) {
      state = RELEASE;
      state_since_ms = now;
    }
    break;
  case RELEASE:
    if (pressed) {
      if (long_fired) {
        // The long press was already delivered; this bounce (or quick re-press)
        // starts a fresh press.
        state = DOWN;
        state_since_ms = now;
        long_fired = false;
      } else {
        state = HELD; // bounce mid-press: keep the original press timing
      }
    } else if (now - state_since_ms >= DEBOUNCE_MS) {
      state = IDLE;
      if (!long_fired) {
        return BUTTON_SHORT;
      }
    }
    break;
  }
  return BUTTON_NONE;
}
