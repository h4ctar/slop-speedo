---
description: Build and flash the board over ST-Link (OpenOCD)
---

Flash the board: run `speedo-code/flash.sh $ARGUMENTS` (preset defaults to Debug). If OpenOCD can't find the ST-Link or the target, report the exact error and the likely cause, such as a USB permission/udev rule, the board not powered, SWD wiring, or the chip held in reset. Don't retry in a loop.
