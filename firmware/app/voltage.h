#ifndef __VOLTAGE_H__
#define __VOLTAGE_H__

#include <stdint.h>

// Battery voltage on the VOLTS input (PA7): 22k from +12V to the pin, 4.7k to
// GND, 100nF filter, Schottky clamp to 3.3V. initVoltage() runs the ADC
// self-calibration once (MX_ADC1_Init already configured channel 7); call
// readBatteryMillivolts() when the voltage function is on screen.
void initVoltage();
uint16_t readBatteryMillivolts();

#endif
