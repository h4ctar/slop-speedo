#include "volt.h"

#include <stdio.h>

#include "adc.h"
#include "screen.h"

// Divider: 22k from +12V, 4.7k to GND -> V_pin = V_batt * 4.7/26.7. With a
// 3.3V reference and 12-bit counts:
//   mV_batt = counts * 3300/4095 * 26.7/4.7 = counts * 4.5786
// The constant is mV per count in thousandths. Nominal 4579, trimmed to 4507
// by a meter fit at J4 (5.3/8.0/12.2 V, 2026-11-19, ST-Link unplugged).
#define MV_PER_COUNT_X1000 4507U
// The divider taps +12V after the reverse-polarity diode D13, so add its
// forward drop back to report the voltage at J4 (from the same meter fit).
#define D13_DROP_MV 298U
#define OVERSAMPLE 16
#define MV_PER_TENTH 100
// How far past the rounding boundary the reading must go before the displayed
// tenth changes, so noise around e.g. 7.75 V doesn't flick between 7.7 and 7.8.
#define HYSTERESIS_MV 40
#define NO_TENTHS_SHOWN UINT16_MAX

static uint16_t shown_tenths = NO_TENTHS_SHOWN;

void volt_init() {
  HAL_ADCEx_Calibration_Start(&hadc1);
}

uint16_t volt_read_battery_mv() {
  uint32_t sum = 0;
  uint32_t samples = 0;

  for (int i = 0; i < OVERSAMPLE; i++) {
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK) {
      continue; // skip a failed conversion rather than bias the average
    }
    sum += HAL_ADC_GetValue(&hadc1);
    samples++;
  }
  if (samples == 0) {
    return 0;
  }
  uint32_t battery_mv = sum * MV_PER_COUNT_X1000 / (samples * 1000U) + D13_DROP_MV;
  return (uint16_t)battery_mv;
}

static uint16_t volt_tenths_update(uint16_t battery_mv) {
  uint16_t nearest_tenths = (battery_mv + MV_PER_TENTH / 2) / MV_PER_TENTH;
  if (shown_tenths == NO_TENTHS_SHOWN) {
    shown_tenths = nearest_tenths;
  } else {
    int32_t offset_mv = (int32_t)battery_mv - (int32_t)shown_tenths * MV_PER_TENTH;
    int32_t limit_mv = MV_PER_TENTH / 2 + HYSTERESIS_MV;
    if (offset_mv > limit_mv || offset_mv < -limit_mv) {
      shown_tenths = nearest_tenths;
    }
  }
  return shown_tenths;
}

static bool volt_update(ButtonEvent event) {
  uint16_t volt_tenths = volt_tenths_update(volt_read_battery_mv());
  char value[8];
  snprintf(value, sizeof(value), "%u.%u", volt_tenths / 10, volt_tenths % 10);
  screen_show_value(value, "V");
  return event == BUTTON_SHORT; // no long-press action for voltage
}

const Function volt_function = {
    .init = volt_init,
    .update = volt_update,
    .name = "VOLT",
};
