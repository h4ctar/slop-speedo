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
  char s[8];
  if (settings_get_unit() == UNIT_KMH) {
    snprintf(s, sizeof(s), "%lukm", (unsigned long)(distance_m / 1000));
  } else {
    snprintf(s, sizeof(s), "%lumi", (unsigned long)(distance_m / 1609));
  }
  screen_show_string(s);
  return event == BUTTON_SHORT; // odometer has no button actions
}

const Function odoFunction = {
    .init = odo_init,
    .update = odo_update,
    .name = "ODO",
};
