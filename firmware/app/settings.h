#ifndef __SETTINGS_H__
#define __SETTINGS_H__

typedef enum {
  UNIT_KMH,
  UNIT_MPH,
} SpeedUnit;

// User settings shared by the speed, odometer and trip functions. Values live
// in RAM for now; persistence in flash lands with the odometer storage TODO.
void settings_init();
SpeedUnit settings_get_unit();
void settings_toggle_unit();

#endif
