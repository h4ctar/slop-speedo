#include "volt.h"

#include <stdio.h>

#include "adc.h"
#include "screen.h"

// Divider: 22k from +12V, 4.7k to GND -> V_pin = V_batt * 4.7/26.7. With a
// 3.3V reference and 12-bit counts:
//   mV_batt = counts * 3300/4095 * 26.7/4.7 = counts * 4.5786
// The constant is mV per count in thousandths; trim it if a meter disagrees.
#define MV_PER_COUNT_X1000 4579U
#define OVERSAMPLE 16

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
  uint32_t battery_mv = sum * MV_PER_COUNT_X1000 / (samples * 1000U);
  return (uint16_t)battery_mv;
}

static bool updateVolt(ButtonEvent event) {
  uint16_t battery_mv = volt_read_battery_mv();
  uint16_t km_tenths = (battery_mv + 50) / 100;
  char volt_string[8];
  snprintf(volt_string, sizeof(volt_string), "%u.%uV", km_tenths / 10, km_tenths % 10);
  screen_show_string(volt_string);
  return event == BUTTON_SHORT; // no long-press action for voltage
}

const Function voltFunction = {
    .init = volt_init,
    .update = updateVolt,
    .name = "VOLT",
};
