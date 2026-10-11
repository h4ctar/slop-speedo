#ifndef __SPEED_H__
#define __SPEED_H__

#include "function.h"

// The SPEED display function: shows the speed in km/h or mi/h (MODE long
// press toggles the unit). The needle follows the speed in every function, so
// speedTask() below runs from loop() regardless of which function is shown.
extern const Function speedFunction;

// Background task: reads the speedo-drive pulses and keeps the needle on the
// speed. Skeleton until the speed sensor lands (hall on PA8, EXTI).
void speedTask();

#endif
