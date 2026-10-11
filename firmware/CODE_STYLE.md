# Firmware code style

Applies to hand-written code in `firmware/app/` (and to hand-written `USER CODE` blocks in CubeMX files). Formatting is mechanical: run clang-format with `firmware/.clang-format` (2-space indent, braces on the same line). This guide covers what the formatter can't enforce.

## Files

- Lower snake case: `button.c`, `speed_sensor.c`.
- Headers live beside their source in `firmware/app/` (no separate include dir).

## Functions

- `module_subject_verb`, lower snake case: `screen_show_string()`, `motor_set_target_position()`, `volt_read_battery_mv()`. The subject may be dropped when there is only one obvious thing in the module: `motor_step()`, `button_poll()`.
- Module prefix on every function, including file-local `static` ones.
- One job per function; keep a function `static` unless it is the module's public API.
- CubeMX entry points follow this too: `app_setup()` / `app_loop()`.

## Variables

- Lower snake case: `function_index`, `frame_buffer`.
- Physical units are appended to the name: `timeout_ms`, `step_period_ticks`, `battery_mv`, `speed_kph`, `distance_m`. A name without a unit means the raw count of whatever it counts.
- Booleans read as predicates: `pressed`, `long_fired`.

## Types

- PascalCase, matching the HAL's own convention: `Function`, `ButtonEvent`, `SetMode`.
- Enum members and `#define`s: ALL_CAPS with underscores, units included: `DEBOUNCE_MS`, `STEP_PERIOD_TICKS`, `UNIT_KMH`.

## Defines and constants

- All `#define`s at the top of the file, right after the includes, before any code. No defines scattered through function bodies.
- No magic numbers in logic: name them, with their unit. Integer scale-factor suffixes like `X1000` are allowed where the math needs one (`MV_PER_COUNT_X1000`), with a comment saying what the value means.

## Includes

- Own header first, then project headers, then HAL/vendor headers, then libc. Alphabetical within each group; groups separated by a blank line.

## Comments

- `//` for line comments; explain **why**, not what. Mark work to do as `TODO:` (it gets copied into `TODO.md`).
- Keep big data tables inside `// clang-format off` / `// clang-format on`.

## State

- No extern globals except the peripheral handles CubeMX owns (`hi2c1`, `hrtc`, `htim1`, `hadc1`). File-scope `static`s are fine and need no module prefix (the scope makes it obvious).
- When an ISR is added, anything it shares with the main loop gets `volatile` and a comment saying which side writes it. No blocking calls in ISRs.

## Exemptions

- CubeMX-generated code (`Core/`, `Drivers/`) keeps its own style: HAL callbacks (`HAL_GPIO_EXTI_Callback`), `MX_*_Init`, pin macros (`MODE_Pin`), and its CRLF/brace conventions inside `USER CODE` blocks.
