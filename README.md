# slop-speedo

A custom digital speedometer/clock gauge for a motorcycle, replacing the stock
analog instrument: an X27.168 air-core stepper drives the needle, a small
OLED shows the display functions, and one MODE button drives the whole UI.
Built on an STM32F103C8T6 ("Blue Pill" class), a custom KiCad PCB, and a
hall-effect sensor off the stock speedo drive on the front wheel.

## Repo layout

- `firmware/` — STM32CubeMX + HAL project (CMake). Hand-written application
  code in `firmware/app/`; architecture in [`firmware/DESIGN.md`](firmware/DESIGN.md);
  style in [`firmware/CODE_STYLE.md`](firmware/CODE_STYLE.md).
- `hardware/` — KiCad 8 PCB project (revision 1, built and working). Generated
  outputs land in `hardware/generated/` and `hardware/JLCPCB/` via CI.

## Features

Working:

- Needle homing and positioning (X27.168, 960 steps full scale, 1.2 ms/step)
- OLED functions cycled by short MODE presses, each introduced by its name
- CLOCK: live time, long press enters set mode (set hours/minutes by button);
  the set time survives resets
- VOLT: battery voltage from the on-board divider (PA7)

In progress:

- SPEED: needle + display from the stock speedo drive (hall sensor on order);
  km/h <-> mi/h toggle
- ODOMETER / TRIP: distance from the wheel pulses; odometer persisted in
  flash, trip reset by long press

## The board

- STM32F103C8T6 (LQFP48), HSE 16 MHz, LSI RTC (32.768 kHz crystal fitted,
  LSE not yet enabled)
- Connectors: J1 debug (SWD + 3.3V), J2 mode button, J3 speedo sensor,
  J4 power (12 V -> buck 5 V -> LDO 3.3 V), J5 VBAT (backup battery),
  J7 breakout (3.3V / PA8-PA12 / GND)

## Building the firmware

Needs `arm-none-eabi-gcc`, `arm-none-eabi-newlib` (for `nano.specs`) and
`ninja` (Arch: `pacman -S arm-none-eabi-gcc arm-none-eabi-newlib ninja`).

```sh
cd firmware
cmake --preset Debug && cmake --build --preset Debug   # or Release
```

`firmware/check.sh` runs a strict (`-Werror`) build of both presets — the
same thing CI does. Output: `build/<preset>/speedo-code.elf`.

## Flashing

With an ST-Link attached:

```sh
firmware/flash.sh [Debug|Release]   # OpenOCD; defaults to Debug
```

## Using the gauge

- **Short press** cycles the display functions: SPEED -> CLOCK -> VOLT ->
  ODO -> TRIP (each function's name flashes for a second first)
- **Long press** is the current function's action: CLOCK enters set mode
  (short presses increment, long presses advance hours -> minutes -> done),
  SPEED switches km/h <-> mi/h, TRIP resets (planned)

## Development process

Work items and status live in [`TODO.md`](TODO.md). Firmware changes are
verified with `check.sh`, reviewed by the `firmware-reviewer` and
`style-reviewer` agents, and hardware-tested before being committed. CI
builds the firmware and runs KiBot (ERC/DRC + outputs) on every push.
