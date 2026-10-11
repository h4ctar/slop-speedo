#include "settings.h"

static SpeedUnit unit = UNIT_KMH;

void settings_init() {
  // TODO: load the saved unit (and later the odometer) from the flash record.
}

SpeedUnit settings_get_unit() {
  return unit;
}

void settings_toggle_unit() {
  unit = (unit == UNIT_KMH) ? UNIT_MPH : UNIT_KMH;
  // TODO: save to the flash record.
}
