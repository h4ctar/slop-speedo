#include "odo.h"

#include <stdio.h>

#include "screen.h"
#include "settings.h"

// TODO: distance accumulates from the wheel pulses (fed by speed_task) and is
// stored wear-leveled in flash so it survives power loss.
static void odo_init() {
}

static bool odo_update(ButtonEvent event) {
  // Dummy distance until the wheel pulses and flash storage land.
  uint32_t distance_m = 12345678; // ~12345.7 km
  char value[8];
  const char *unit;
  if (settings_get_unit() == UNIT_KMH) {
    snprintf(value, sizeof(value), "%lu", (unsigned long)(distance_m / 1000));
    unit = "km";
  } else {
    snprintf(value, sizeof(value), "%lu", (unsigned long)(distance_m / 1609));
    unit = "mi";
  }
  screen_show_value(value, unit);
  return event == BUTTON_SHORT; // odometer has no button actions
}

const Function odo_function = {
    .init = odo_init,
    .update = odo_update,
    .name = "ODO",
};
