#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "function.h"

// The CLOCK display function: shows HH:MM. Long press enters set mode -- the
// field being edited (hours first, then minutes) blinks while short presses
// increment it and long presses advance hours -> minutes -> exit; 10 s
// without a press exits too.
extern const Function clock_function;

#endif
