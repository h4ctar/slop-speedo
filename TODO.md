# TODO

Work through with `/next` (first unchecked item) or `/next <item>`. Every item needs a "Done when" line that can be checked on the board.

## CI

- [x] **Fix the Node.js 20 deprecation warning in GitHub Actions** (all 3 jobs): bumped `actions/checkout` and `actions/upload-artifact` from v4 to v7 (Node 24). The firmware build itself had no compiler warnings.
      Done when: the next push shows no warning annotations (`gh api repos/{owner}/{repo}/check-runs/<job-id>/annotations`). The "ubuntu-latest will migrate to Ubuntu 26 on 2026-10-19" notice is informational; just check the Firmware job still passes after that date.
      Verified 2026-10-10: all 3 jobs of the f0224da runs have zero warning annotations. Re-check the Firmware job after the 2026-10-19 Ubuntu 26 runner migration (a newer apt gcc could surface new warnings).
- [x] **KiBot warnings in the PCB job** (they don't fail CI, but they're noise). These need PCB edits, so discuss with Ben first:
  - W135: "part field mismatch" for most parts. The schematic `Value` (e.g. `0402,4.7kΩ`) differs from the PCB/LCSC part number field. Possibly fix with a KiBot field mapping in `config.kibot.yaml` rather than by editing parts.
  - W147: 6 footprints with reference `REF**` (likely mounting holes or the gauge outline at (77,77), (123,123), etc.). Give them real references (H1…) or mark them as board-only.
  - DRC: 11 warnings (6 "extra footprint", 5 "duplicate footprints"), probably the same `REF**` footprints.
      Done when: the KiBot log (`gh run view <run-id> --log | grep WARNING`) has no W135/W145/W147 lines, and the board is electrically unchanged.
      Done 2026-10-10: mounting holes renamed H1-H6 with `board_only` (kills W147/W013 and the 11 DRC warnings); scoped W135 filter in `config.kibot.yaml`. CI run 38023468174: no W135/W145/W147 lines. Remaining known noise: W004 (stale `Stock` conflicts between same parts), W020 (inductor value `FXL0420-100-M` not parseable).

## Known bugs (found in existing code)

- [x] **I2C bus can lock up (OLED stops updating).** Found on hardware 2026-10-10: I2C1 SR2 BUSY=1 while not master, and SDA (PB7) is held low with SCL high. Every transfer then times out (`hi2c1.ErrorCode` = 0x20 TIMEOUT), and `loop()` just spins in `I2C_WaitOnFlagUntilTimeout`. Confirmed cause: the MCU was reset by `/flash` in the middle of a transfer, and the SSD1306 kept holding SDA low. A full power cycle clears it, and I2C then runs with `ErrorCode` 0. Workaround until fixed: power-cycle the board after flashing. Fix: `I2C1_RecoverBus()` in `i2c.c`, called from `HAL_I2C_MspInit`, clocks SCL until the OLED releases SDA and then sends a STOP. Runtime recovery (without a reset) was dropped on purpose: power-up resets the MCU and OLED together, so only resets that keep the OLED powered (flash, debugger, NRST, watchdog, brownout) trigger this, and they all go through `MX_I2C1_Init`.
      Done when: the OLED keeps working after repeated `/flash` and debugger halts without a power cycle.
      Agent-tested 2026-10-10 on the running board (reading `hi2c1.ErrorCode` at 0x200000f4 / `.State` at 0x200000f1 via openocd, no halting): 3 consecutive `./flash.sh` resets mid-transfer, 1 `reset halt`+resume, and 1 plain halt+resume, then 4 samples over 8 s — `ErrorCode` 0 and `State` 0x21 (BUSY_TX) after every reset, no power cycle.
      Done 2026-10-10: Ben confirmed the display works and updates after flashing, no power cycle.
- [x] **RTC time resets on every boot.** The RTC counter matched uptime (33 s after flashing), so the time is lost on reset even with the CR1220 backup battery. Probably `MX_RTC_Init` sets the time unconditionally. Use a backup-register "time set" flag in a USER CODE block of `rtc.c` to skip setting it.
      Done when: the time set before a reset is still correct after the reset (with the battery fitted).
      Fix: in `rtc.c`, BKP_DR1 flag `RTC_TIME_SET_MAGIC` 0x5254 — `Check_RTC_BKUP` block returns early (skipping the generated SetTime/SetDate) when the flag is set; `RTC_Init 2` block writes the flag after SetTime/SetDate succeed. Removing all power (no Vbat fitted) clears the flag, so the clock then restarts at 00:00 — by design.
      Agent-tested 2026-10-10: after first boot writes the flag (BKP_DR1 read 0x5254 via openocd), two consecutive `/flash` resets preserved the counter — read via gdb `call HAL_RTC_GetTime` into `buffer` on the halted target: 00:02:20 → (flash reset) → 00:03:49, +89 s over ~90 s wall time. Note: RTC registers (RTC_CNTH/CNTL, BDCR) read garbage via openocd SWD even halted; BKP_DR1 reads fine; use the HAL read path (gdb call, or the OLED itself) for time. RTC runs on LSI (40 kHz ±30%), so the displayed time drifts badly — X2 (32.768 kHz) is fitted but LSE is not enabled (see VBAT answer below). Battery/power-loss persistence deferred to its own item under "Hardware tests" (no CR1220 on hand; a 3.7 V Li-ion is out of spec for VBAT).
      Done 2026-10-10: Ben confirmed on the board.

- [x] **Motor timing wraps wrongly.** TIM1's counter is 16-bit, but `motor.c` compares it to a `uint32_t lastStepTime`. Each time the counter wraps, the difference is huge, so the motor takes an early step.
      Done when: step timing uses 16-bit unsigned subtraction, and the needle sweeps smoothly without jitter.
      Fix: `lastStepTime` is now `uint16_t` and `updateMotor()` compares `(uint16_t)(currentTime - lastStepTime) >= STEP_PERIOD` — unsigned modular subtraction, exact across every 65.5 ms wrap. Firmware-reviewer: no issues (modular math well-defined, conversions safe, works on both Arch and CI gcc).
      Agent-tested 2026-10-10: clean boot (no debugger halting — gdb `detach` leaves the core halted with this openocd, which is what the earlier "stuck at 901/960" readings were): homing sweep progresses 946/960 → 556 descending → 0 at 1.2 ms/step as designed, ~2.3 s total, no hang.
      Done 2026-10-10: Ben watched the replayed boot sweep — smooth, uniform 1.2 ms/step.
- [x] **Negative positions aren't clamped.** In `setMotorTargetPosition`, `targetSteps < 0` is always false (`targetSteps` is unsigned), and converting a negative float to `uint16_t` is undefined behaviour.
      Done when: the input is clamped as a float before conversion.
      Fix: `setMotorTargetPosition` clamps the float input to [0, MAX_POSITION] before converting — low clamp is `!(position >= 0)`, which also routes NaN (fails every comparison) to 0; dropped the dead unsigned checks, kept the >MAX_STEPS guard as belt-and-braces. Firmware-reviewer: correct, and flagged the NaN case which is now closed; the only remaining callers (initMotor literals) are unaffected.
      Agent-tested 2026-10-10: gdb `call setMotorTargetPosition` on the halted board: -5 → 0, 100 → 960, NaN → 0, 0 → 0.
      Done 2026-10-10: Ben confirmed on the board (boot sweep unchanged).
- [x] **The needle never moves after homing.** Nothing calls `updateMotor()` from `loop()`, and `updateMotor` isn't declared in `motor.h`.
      Done when: `loop()` services the motor, and setting a target position moves the needle without blocking the display.
      Fix: `updateMotor()` declared in `motor.h` and called first thing in `loop()`; `updateMotor()` is now a catch-up loop (re-reads the 16-bit counter, steps every due step, anchors `lastStepTime` when idle, `lastStepTime += STEP_PERIOD` scheduling) so the needle moves at full rate even though `loop()` blocks ~50 ms per OLED redraw — reviewer confirmed termination, exact step counts across wraps/stalls, and flagged that big slews fire ~40-step bursts per frame until the OLED-redraw TODO lands (small moves are 1-2 steps/frame, invisible).
      Agent-tested 2026-10-10: commanded full-scale up/down by injecting `targetSteps` (mww) on the running board: 0→960 in ~1.5 s (391→708→960), 960→0 (590→272→0), exact to the step, no stalls. Debug-session forensics (frozen core at 901/Reset_Handler) traced to leftover FPB breakpoints + vector catch from gdb `call` — remedies documented in `AGENTS.md`.
      Done 2026-10-10: Ben confirmed on the board.

## Features (decided by Ben 2026-10-10: needle = speed from the stock speedo drive; screen cycles functions via the MODE button)

- [x] Decide what the needle shows (speed? volts? time?) and its input source.
      Decided: the needle shows the speed, from a speed sensor driven by the stock speedo drive on the front wheel (see the speed-sensor item below).
- [ ] **Speed sensor from the stock speedo drive (front wheel)**: design decided 2026-10-10 (Path A): hall-effect switch (3.3 V, open-drain, e.g. AH1815 TO-92) + neodymium magnet(s) on the rotating drive; 1 pulse/cable-rev; cable speeds ~1700-2900 rpm at 100-160 km/h → 28-50+ Hz pulse rate; internal pull-up (~40 kΩ) is fine for those rates. Parts ordered 2026-10-10 (AH1815 halls, 5×2 mm neodymium magnets, JST PH pigtails). Bench test first via the J7 breakout socket (3.3V / PA8 / GND — clean pins): hall V+ → J7 pin 1, GND → pin 7, OUT → pin 6 (PA8); J3 gets used only after D15 is desoldered (see "PCB revision notes"). CubeMX change needed for firmware (Ben): bench = PA8 → GPIO_EXTI8, Input + Pull-up, falling edge, NVIC EXTI9_5 enabled; final = PA1 → GPIO_EXTI1 the same way. Calibration: speed = pulse_rate × 3600 / pulses_per_km; get pulses_per_km from the stock speedo's stamped ratio (check the face of the original speedo or tell the agent the bike model) or empirically by counting pulses over a measured distance.
      Done when: spinning the front wheel by hand produces pulse counts on the board that map to a plausible speed.
- [x] **MODE button (PA6)**: read with debouncing; short press cycles the display functions, long press triggers the current function's secondary action.
      Fix: new hand-written `button.c`/`button.h` — time-based state machine (25 ms debounce, short fires on release, long fires the moment the hold crosses 800 ms, once per press, bounce/tick-wrap safe); Ben set PA6 Pull-up in CubeMX (the PCB has no pull-up on the MODE net — the released pin floated, which is why the button never worked). Stub UI in `app.c`: short cycles SPEED/CLOCK/VOLT/ODO/TRIP names (CLOCK renders the live time), long flashes "LONG" 1.5 s; real per-function actions land with each function's item. Firmware-reviewer: state machine traced clean. Note: the CubeMX regeneration dropped the MODE pin label, so button.c reads GPIOA/GPIO_PIN_6 directly.
      Agent-tested 2026-10-11 on the board (watching `displayedString`/`functionIndex`/GPIOA IDR via openocd while Ben pressed): shorts cycle +1 with wrap (SPEED→CLOCK→VOLT→ODO→TRIP→SPEED), CLOCK renders the live time, two 1.5 s holds each fired exactly one LONG with no short leaked and no function change, no double-fires.
      Done when: short presses cycle functions on the OLED, long presses trigger the current function's long-press action, with no double-fires or bounces.
      Done 2026-10-11: Ben confirmed on the board.
- [ ] **UI function switcher**: short press cycles Speed → Clock → Voltage → Odometer → Trip → Speed…; the needle always displays the speed no matter which function is on the OLED (decided by Ben).
      Done when: each function renders on the OLED and the cycle order wraps back to Speed, with the needle tracking the speed throughout.
- [ ] **Speed function**: display the speed in km/h on the OLED, needle shows the speed; long press switches to m/h (and back on the next long press). The km/h-m/h choice persists across power cycles (decided by Ben) — store it in flash alongside the odometer (BKP registers need the still-unfitted battery).
      Done when: the speed displays and tracks the wheel while riding; long press swaps units and the choice survives a full power cycle.
- [x] **Clock function**: display HH:MM on the OLED; long press enters set mode — the button then sets hours, then minutes (replaces the commented-out `HAL_RTC_SetTime` in `setup()`); exit set mode with a long press (or a timeout).
      Fix: `app.c` — long press in CLOCK enters set mode; the screen shows "H xx"/"M xx"; short presses increment (+1 mod 24/60) with the value written straight to the RTC (read-modify-write, BIN format), long presses advance H → M → exit, 10 s of silence exits too; the commented-out HAL_RTC_SetTime block is gone from `setup()`. GetTime failure now skips the pass (a set-mode write must never persist a bad read). Firmware-reviewer: state machine, F1 RTC specifics (no GetDate requirement, whole-counter write), and display width all clean.
      Agent-tested 2026-10-11: Ben set the clock to 14:40 via the button (screen showed "14:41" after one minute tick); a full `reset run` kept the clock ("14:41" re-rendered after boot, boot was on SPEED so the function index was flipped to CLOCK via RAM to see it) and it ticked on to "14:43" — survives reset and keeps running.
      Done when: the time can be set from the button and survives a reset.
      Done 2026-10-11: Ben confirmed on the board (set 14:40 by button).
- [ ] **Voltage function (PA7 / ADC1 ch7)**: read and scale to volts using the PCB divider (values from the `hardware` schematic); display on the OLED in the voltage function (e.g. "12.4V"). The needle is not part of this function — it always shows speed (decided by Ben); no low-battery warning.
      Done when: the displayed volts match a multimeter on the battery within ~0.1 V.
- [ ] **Odometer & trip functions**: both accumulate distance from the wheel pulses; trip resets on long press; odometer must survive power loss (needs wear-leveled storage in flash — F103C8 has 1 KB pages). The same flash record holds the km/h-m/h unit flag, which applies to the speed, odometer and trip displays (decided by Ben).
      Done when: trip accumulates and resets via long press; odometer accumulates and survives a full power cycle; the unit flag survives too.
- [x] **OLED redraw only when the displayed value changes** (per function: clock = minute tick, speed = every few hundred ms, etc.) instead of every loop iteration; this also shrinks loop latency so the motor steps smoothly instead of bursting (see the ticked "needle never moves" item).
      Done when: the OLED content is unchanged but the loop period drops, and the display still updates.
      Fix: `app.c` renders the clock into a string and repaints only when it differs from a static `displayedString` cache (starts empty → first loop paints). Future display functions reuse the same pattern. Reviewer's follow-up built in: `writeCommand`/`writeData`/`updateScreen` now propagate `HAL_StatusTypeDef` and the cache is only updated after a successful flush, so a failed I2C write retries next loop() instead of leaving a stale screen for a minute.
      Agent-tested 2026-10-10: OLED cache in RAM read back as the live "01:08" clock string; injected a half-scale needle command (480 steps) — step progress uniform at ~84 steps/100 ms (840 steps/s, no bursts) vs the old ~40-step bursts per 50 ms frame, and it stopped exactly on target; commanded back to 0.
      Done 2026-10-10: Ben confirmed on the board.

## Hardware tests

- [ ] **Test the Vbat battery keeps the clock across full power loss.** Needs a CR1220 in a holder wired to J5 (JST PH 2-pin: pin 1 = VBAT = +, pin 2 = GND). The board has no series diode or charger, so check the holder's polarity with a meter before plugging in, and do NOT use a 3.7 V Li-ion directly — VBAT is rated 1.8-3.6 V and a full Li-ion is 4.2 V. If the Li-ion must be used, put a 1N4148/1N4007 in series with its + wire (drops 4.2 V to ~3.55 V, in spec).
      Done when: with the battery fitted, the time read via gdb `call HAL_RTC_GetTime` carries forward over a full power-off (USB unplugged for a minute or two) instead of restarting at 00:00, and restarting at 00:00 happens with the battery removed.

## PCB revision notes

Record board problems found during hardware testing here. Only edit the PCB after discussion.

- OLED lockup after an MCU-only reset is handled in software (`I2C1_RecoverBus()`). The OLED module has no reset pin, so the only hardware fix would be switching its power from a GPIO (P-MOSFET or load switch). Decided 2026-10-10 that this isn't worth a revision on its own; reconsider only if the board is being revised anyway.
- **SPEEDO input conditioning is broken as built**: J3's SPEEDO net (PA1+PA2+PA3) carries D15 (red LED) → R1 (51Ω) → GND. The LED clamps the line at ~Vf (1.8 V) for any pull-up-fed sensor — under the F103's V_IH (2.3 V); no R1 value fixes it (the node can only pass 2.3 V with ~10 mA of pull-up current). Decision 2026-10-10 (Path A): **desolder D15 on the built board** before using J3; R1 becomes irrelevant. Open-drain hall + internal pull-up (~40 kΩ) is fine for 28-90 Hz pulses. Do NOT change the PCB design yet — if a rev-2 happens, prefer either dropping D15 and adding an external 10k pull-up, or R1 → 470-1000 Ω plus a push-pull hall (DRV5032FA) so the LED works as a speed-blinker (sensor sources ~1.5-3 mA, node ≈ 3.2 V).
