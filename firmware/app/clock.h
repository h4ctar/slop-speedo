#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "function.h"

// The CLOCK display function: shows HH:MM. Long press enters set mode ("H xx"
// then "M xx"): short presses increment the field, long presses advance
// H -> M -> exit, and 10 s without a press exits too.
extern const Function clock_function;

#endif
