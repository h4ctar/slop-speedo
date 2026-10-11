#include "odo.h"

#include <stdio.h>

#include "screen.h"
#include "settings.h"

// TODO: distance accumulates from the wheel pulses (fed by speedTask) and is
// stored wear-leveled in flash so it survives power loss.
static void initOdo() {
}

static bool updateOdo(ButtonEvent event) {
  // Dummy distance until the wheel pulses and flash storage land.
  uint32_t meters = 12345678; // ~12345.7 km
  char s[8];
  if (settings_getUnit() == UNIT_KMH) {
    snprintf(s, sizeof(s), "%lukm", (unsigned long)(meters / 1000));
  } else {
    snprintf(s, sizeof(s), "%lumi", (unsigned long)(meters / 1609));
  }
  showString(s);
  return event == BUTTON_SHORT; // odometer has no button actions
}

const Function odoFunction = {
    .init = initOdo,
    .update = updateOdo,
    .name = "ODO",
};
