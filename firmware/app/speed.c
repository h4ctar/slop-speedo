#include "speed.h"

#include "screen.h"
#include "settings.h"

// Background task: count pulses from the speedo-drive hall sensor, derive the
// speed (pulse rate x 3600 / pulses-per-km), and drive the needle there.
// Skeleton until the sensor lands: with no pulses the speed is 0 and the
// needle stays parked at 0.
void speed_task() {
}

static void speed_render() {
  // TODO: render the measured speed once the sensor feeds speed_task.
  screen_show_string(settings_get_unit() == UNIT_KMH ? "0 km/h" : "0 mi/h");
}

static void speed_init() {
}

static bool speed_update(ButtonEvent event) {
  if (event == BUTTON_LONG) {
    settings_toggle_unit(); // km/h <-> mi/h
  }
  speed_render();
  return event == BUTTON_SHORT;
}

const Function speed_function = {
    .init = speed_init,
    .update = speed_update,
    .name = "SPEED",
};
