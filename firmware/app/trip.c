#include "trip.h"

#include <stdio.h>

#include "screen.h"
#include "settings.h"

// TODO: the trip accumulates from the wheel pulses (fed by speed_task) and is
// stored in flash next to the odometer.
static void trip_init() {
}

static bool trip_update(ButtonEvent event) {
  if (event == BUTTON_LONG) {
    // TODO: trip = 0 (and persist), once the accumulation lands.
  }
  // Dummy distance until the wheel pulses and flash storage land.
  uint32_t distance_m = 123400; // 123.4 km
  char s[8];
  if (settings_get_unit() == UNIT_KMH) {
    uint32_t km_tenths = (distance_m + 50) / 100;
    snprintf(s, sizeof(s), "%lu.%lukm", (unsigned long)(km_tenths / 10),
             (unsigned long)(km_tenths % 10));
  } else {
    uint32_t mi_tenths = (uint32_t)((uint64_t)distance_m * 621371ULL / 100000000ULL);
    snprintf(s, sizeof(s), "%lu.%lumi", (unsigned long)(mi_tenths / 10),
             (unsigned long)(mi_tenths % 10));
  }
  screen_show_string(s);
  return event == BUTTON_SHORT;
}

const Function trip_function = {
    .init = trip_init,
    .update = trip_update,
    .name = "TRIP",
};
