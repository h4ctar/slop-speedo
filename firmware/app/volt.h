#ifndef __VOLT_H__
#define __VOLT_H__

#include <stdint.h>

#include "function.h"

// The VOLT display function: reads the battery via the VOLTS divider (PA7)
// and shows it as "x.yV". The needle is not part of this function.
extern const Function voltFunction;

// Battery voltage in battery_mv (16× oversampled).
uint16_t volt_read_battery_mv();

#endif
