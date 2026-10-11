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
  char value[8];
  const char *unit;
  if (settings_get_unit() == UNIT_KMH) {
    uint32_t km_tenths = (distance_m + 50) / 100;
    snprintf(value, sizeof(value), "%lu.%lu", (unsigned long)(km_tenths / 10),
             (unsigned long)(km_tenths % 10));
    unit = "km";
  } else {
    uint32_t mi_tenths = (uint32_t)((uint64_t)distance_m * 621371ULL / 100000000ULL);
    snprintf(value, sizeof(value), "%lu.%lu", (unsigned long)(mi_tenths / 10),
             (unsigned long)(mi_tenths % 10));
    unit = "mi";
  }
  screen_show_value(value, unit);
  return event == BUTTON_SHORT;
}

const Function trip_function = {
    .init = trip_init,
    .update = trip_update,
    .name = "TRIP",
};
