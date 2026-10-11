#include "voltage.h"
#include "adc.h"

// Divider: 22k from +12V, 4.7k to GND -> V_pin = V_batt * 4.7/26.7. With a
// 3.3V reference and 12-bit counts:
//   mV_batt = counts * 3300/4095 * 26.7/4.7 = counts * 4.5786
// The constant is mV per count in thousandths; trim it if a meter disagrees.
#define MV_PER_COUNT_X1000 4579U
#define OVERSAMPLE 16

void initVoltage() {
  HAL_ADCEx_Calibration_Start(&hadc1);
}

uint16_t readBatteryMillivolts() {
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
  uint32_t millivolts = sum * MV_PER_COUNT_X1000 / (samples * 1000U);
  return (uint16_t)millivolts;
}
