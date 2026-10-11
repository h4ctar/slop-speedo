#include "speed.h"

#include "screen.h"
#include "settings.h"

// Background task: count pulses from the speedo-drive hall sensor, derive the
// speed (pulse rate x 3600 / pulses-per-km), and drive the needle there.
// Skeleton until the sensor lands: with no pulses the speed is 0 and the
// needle stays parked at 0.
void speedTask() {
}

static void renderSpeed() {
  // TODO: render the measured speed once the sensor feeds speedTask.
  showString(settings_getUnit() == UNIT_KMH ? "0 km/h" : "0 mi/h");
}

static void initSpeed() {
}

static bool updateSpeed(ButtonEvent event) {
  if (event == BUTTON_LONG) {
    settings_toggleUnit(); // km/h <-> mi/h
  }
  renderSpeed();
  return event == BUTTON_SHORT;
}

const Function speedFunction = {
    .init = initSpeed,
    .update = updateSpeed,
    .name = "SPEED",
};
