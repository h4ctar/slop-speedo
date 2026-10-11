#include "trip.h"

#include <stdio.h>

#include "screen.h"
#include "settings.h"

// TODO: the trip accumulates from the wheel pulses (fed by speedTask) and is
// stored in flash next to the odometer.
static void initTrip() {
}

static bool updateTrip(ButtonEvent event) {
  if (event == BUTTON_LONG) {
    // TODO: trip = 0 (and persist), once the accumulation lands.
  }
  // Dummy distance until the wheel pulses and flash storage land.
  uint32_t meters = 123400; // 123.4 km
  char s[8];
  if (settings_getUnit() == UNIT_KMH) {
    uint32_t tenths = (meters + 50) / 100;
    snprintf(s, sizeof(s), "%lu.%lukm", (unsigned long)(tenths / 10),
             (unsigned long)(tenths % 10));
  } else {
    uint32_t miTenths = (uint32_t)((uint64_t)meters * 621371ULL / 100000000ULL);
    snprintf(s, sizeof(s), "%lu.%lumi", (unsigned long)(miTenths / 10),
             (unsigned long)(miTenths % 10));
  }
  showString(s);
  return event == BUTTON_SHORT;
}

const Function tripFunction = {
    .init = initTrip,
    .update = updateTrip,
    .name = "TRIP",
};
