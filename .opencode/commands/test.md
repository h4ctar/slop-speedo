---
description: Full hardware test of every display function and subsystem on the board
---

Run a full function test on the board: build, flash, then verify everything that can be verified from this side, and finish with a checklist of what I must confirm by eye/press. Preset: $ARGUMENTS (default Debug — symbols are needed for RAM addresses).

1. **Build + flash**: run `firmware/check.sh` (must pass with no warnings), then `firmware/flash.sh $ARGUMENTS`. Wait ~5 s for boot (homing sweep ~2.3 s + OLED init + first paint).
2. **Addresses**: get the RAM addresses for `onScreen`, `functionIndex`, `currentSteps`, `targetSteps`, `hi2c1` from `arm-none-eabi-nm build/$PRESET/speedo-code.elf`. Never use gdb — it leaves FPB breakpoints that brick the session (see AGENTS.md). Inspect with openocd `-c 'init; ...; shutdown'` only; use `halt` + `mww` + `resume` for RAM injection.
3. **Automated checks** (report pass/fail each):
   - I2C healthy: `hi2c1.ErrorCode` == 0 (offset from the Debug ELF: ErrorCode sits at hi2c1+0x40).
   - Every function renders: inject `functionIndex` = 0..4 one at a time (halt/mww/resume, ~300 ms between), and read `onScreen` after each: expect "0 km/h" or "0 mi/h" (SPEED), a live "HH:MM" (CLOCK), "x.yV" (VOLT — ~2.8V is expected with no battery on J4), "ODO", "TRIP".
   - Clock ticks: two `onScreen` reads ~5 s apart in the CLOCK function; the time must be stable or advancing, never backwards.
   - Needle: inject `targetSteps` = 480, poll `currentSteps` until it reaches 480 exactly (~0.6 s, uniform rate), then inject 0 and confirm it returns to 0 exactly.
   - Name flash: after injecting a `functionIndex` change, the first `onScreen` read should show the function NAME, then its content ~1 s later.
4. **Interactive checks**: ask me to do these on the board and confirm what I see; cross-check the RAM state (`onScreen`/`functionIndex`) after each where possible:
   - Short presses cycle all 5 functions with a ~1 s name flash; rapid tapping cycles fast (no 1 s lock).
   - CLOCK: long press enters set mode ("H xx" then "M xx"), taps increment (wrap 23->0 / 59->0), long advances, long exits; 10 s idle exits too.
   - Persistence: after set mode, run openocd `reset run` and confirm the set time survives and keeps ticking.
   - SPEED long press toggles km/h <-> mi/h.
   - Needle smooth: no jitter or bursts during the boot sweep and injected sweeps.
   - VOLT vs meter: only when a battery is on J4.
5. **Report**: a pass/fail table of everything above, plus the short checklist of anything left for me to confirm by eye. Don't commit.
