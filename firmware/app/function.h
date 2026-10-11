#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <stdbool.h>

#include "button.h"

// A display function: what the MODE button cycles through. app.c owns the
// registry and the current index; each function owns its logic and display.
//
//  - init: called once at boot, after the peripherals are up.
//  - update: called every loop() with the button event. Handles the logic and
//    renders the screen (via screen_show_value). Returns true when the event was NOT
//    consumed and should cycle to the next function (i.e. a short press the
//    function doesn't use for itself, like set mode does).
//  - name: shown for a second when the function is first cycled to.
typedef struct {
  void (*init)(void);
  bool (*update)(ButtonEvent event);
  const char *name;
} Function;

#endif
