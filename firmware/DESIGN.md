# Firmware design

Speedometer/clock gauge for a motorcycle: an X27.168 air-core stepper drives the
needle, an SSD1306 OLED shows the display functions, and a single MODE button
drives the UI. Runs on an STM32F103C8T6 (64 KB flash, 20 KB RAM, Blue Pill
class) built with CubeMX + HAL + CMake. Work items live in `TODO.md`, code
style in `CODE_STYLE.md`.

## Code layout

```
firmware/
  Core/            STM32CubeMX output - do not hand-edit outside USER CODE blocks
    Src/main.c     HAL init, clock config, MX_*_Init calls,
                   app_setup() (USER CODE 2), app_loop() (USER CODE 3)
    Src/gpio.c     pin configs          Src/i2c.c   I2C1 + bus recovery
    Src/adc.c      battery divider ADC  Src/rtc.c   RTC + time-set flag
    Src/tim.c      TIM1 = 1 MHz step clock
  app/             everything hand-written (immune to CubeMX regeneration)
    app.c          function registry + background tasks + name flash
    function.h     the Function contract
    button.c       MODE button: debounce, short/long press
    speed.c        SPEED function + speedTask() background skeleton
    clock.c        CLOCK function + set mode
    volt.c         VOLT function + battery ADC (PA7 divider)
    odo.c, trip.c  stubs; accumulation + flash storage TODO
    settings.c     shared user settings (unit flag; flash persistence TODO)
    motor.c        X27.168 stepper driver (direct GPIO coil stepping)
    screen.c       SSD1306 driver + showString() change-detected flush
  Drivers/, cmake/stm32cubemx/   vendor HAL and generated build glue
```

## Runtime model

Bare-metal superloop, no RTOS, no work queues:

```
main:  HAL_Init -> SystemClock_Config (HSE 16 MHz x2 = 32 MHz)
       -> MX_GPIO/I2C1/RTC/TIM1/ADC1_Init -> app_setup() -> while(1) app_loop()

app_loop:  motor_update()      - step the needle (background, always)
           speed_task()        - pulses -> speed -> needle (background, always)
           poll button event   - time-based state machine (never blocks)
           function->update(event) - logic + display of the current function
           [on true] next function + 1 s name flash
```

`app_loop()` normally runs in well under 1 ms. It can stall for ~47 ms once
per screen change (the 4-page I2C flush) — the motor catch-up logic absorbs
that; see "Motor timing".

## The Function abstraction

`function.h` defines the contract between `app.c` and the display functions:

```c
typedef struct {
  void (*init)(void);                 // once at boot, after peripheral init
  bool (*update)(ButtonEvent event);  // every app_loop pass
  const char *name;                   // shown ~1 s when first cycled to
} Function;
```

- `app.c` holds the registry (`speedFunction`, `clockFunction`, `voltFunction`,
  `odoFunction`, `tripFunction`) and the current index. That is all it knows.
- `update(event)` does the function's logic and rendering and returns **true**
  when the event was an unconsumed short press — the signal to cycle to the
  next function. Functions that use short presses for themselves (clock set
  mode) consume them and return false.
- Long-press meaning is per function: CLOCK enters set mode, SPEED toggles
  km/h <-> mi/h, TRIP will reset; VOLT/ODO have none.
- On a switch, the new function's `name` is shown for 1 s (its `update` is
  skipped during the flash). Cycling is never blocked: a short press during
  the flash advances immediately, a long press cuts the flash short and is
  handed to the function.
- The needle is not part of any function: it always shows the speed (decided),
  driven by `speed_task()` below.

## Motor (needle)

X27.168 air-core stepper, coils driven push-pull from four GPIOs
(PA4/PA5 = M3/M4, PB4/PB5 = M1/M2) with a 6-step coil pattern
(`current_steps % 6`). Full scale = `MAX_POSITION` (16) x
`STEPS_PER_POSITION` (60) = 960 steps. One step per 1.2 ms (~833 steps/s).

- **Timing**: TIM1 free-runs at 1 MHz (32 MHz APB2 timer clock, prescaler 31),
  wrapping every 65.536 ms. Step scheduling uses 16-bit unsigned modular
  subtraction (`(uint16_t)(now - last_step_time_ticks) >= STEP_PERIOD_TICKS`)
  which is exact across wraps; 32-bit comparison was a bug (early steps at
  every wrap).
- **Catch-up**: `motor_update()` steps every due step per call, anchoring the
  schedule when idle, so a 47 ms display stall is absorbed as one burst and
  the needle never loses a step. A stall > 65.5 ms under-measures elapsed
  ticks and merely defers steps (never fires early).
- **Homing**: `motor_init()` drives to full scale and back to 0 (the known
  rest position). Position is open-loop software state: missed physical steps
  (over-driving the rotor) would lose position until the next boot.

## Screen

SSD1306 128x32 over I2C1 @ 100 kHz, address 0x3C. 512-byte frame buffer;
`screen_draw_pixel/draw_char/draw_string` render into it; `screen_update()`
flushes it as 4 pages (each page = 3 commands + 128 data bytes) and returns
HAL status. `showString()` is the UI entry point: it flushes only when the
string differs from what is on screen (the cache updates only after a
successful flush, so a failed I2C write retries next pass). The glyph cell is
16 px wide -> 8 characters per line.

**Bus recovery**: a reset mid-transfer can leave the SSD1306 holding SDA low
(it has no reset pin). `I2C1_RecoverBus()` (i2c.c, called from
`HAL_I2C_MspInit` before the pins go to I2C mode) clocks SCL until SDA is
released and sends a STOP, so flashing/halting never needs a power cycle.

## Button

MODE button on PA6 (on-board S1 and the external J2 connector, both to GND;
internal pull-up set in CubeMX — the PCB has no pull-up on this net).
`button_poll()` is a time-based state machine called every pass: 25 ms
debounce, short press fires on release, long press fires the moment the hold
crosses 800 ms (once per press), bounces mid-press keep the original press
timing. All timing uses HAL_GetTick() deltas (wrap-safe).

## Speed sensor (designed, parts ordered)

Hall-effect switch (3.3 V, open-drain, AH1815) + neodymium magnet(s) on the
stock speedo drive on the front wheel; 1 pulse per cable revolution (~28-50 Hz
at riding speeds). Bench: J7 breakout (3.3V / PA8 / GND); final: J3 "Speedo"
(3.3V / SPEEDO / GND) after D15 is desoldered (see "PCB revision notes" in
TODO.md — the LED on that net clamps it below V_IH). Needs a CubeMX change
(PA8 GPIO_EXTI8 falling + pull-up, EXTI9_5 NVIC; PA1 final). Calibration:
`speed_kph = pulse_hz * 3600 / pulses_per_km`; pulses_per_km comes from the
stock speedo's stamped ratio or a rolled measured distance. The gauge's
full-scale km/h (needle mapping) is still to be decided.

## Persistence

- **Time**: RTC on LSI (40 kHz / 40000 = 1 Hz; +-30% drift — the fitted
  32.768 kHz crystal X2 is not enabled yet; switching RTC -> LSE in CubeMX is
  a planned improvement). `rtc.c` keeps a "time set" flag in backup register
  BKP_DR1 (magic 0x5254): boots skip the 00:00 re-set while the flag survives
  in the Vbat domain; a full power loss clears it and the clock restarts at
  00:00 (a CR1220 on J5/VBAT will change that — battery test pending).
- **Settings/odometer/trip** (planned): a wear-leveled record in the F103's
  1 KB flash pages holding the odometer, trip, and the km/h-mi/h unit flag
  (shared via `settings.c`), written periodically, not per pulse.

## Memory budget

Flash ~30 KB / 64 KB (45%), RAM ~2.8 KB / 20 KB (13.5%) as of the function
registry refactor. The float-free rendering (integer `snprintf`, no
`%f`) keeps the nano.specs binary small.

## Conventions

Code style: `CODE_STYLE.md` (module_subject_verb naming, units in names,
defines at the top). Reviews before committing: `firmware-reviewer` (bugs,
CubeMX safety) and `style-reviewer` (this repo's style guide).
