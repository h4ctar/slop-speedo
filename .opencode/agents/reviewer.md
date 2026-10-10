---
description: Read-only review of firmware changes for STM32/CubeMX-specific bugs. Use after implementing a task, before committing.
mode: subagent
permissions:
  - action: edit
    resource: "*"
    effect: deny
  - action: shell
    resource: "*"
    effect: deny
  - action: shell
    resource: "git diff *"
    effect: allow
  - action: shell
    resource: "git status *"
    effect: allow
  - action: shell
    resource: "git log *"
    effect: allow
  - action: shell
    resource: "git show *"
    effect: allow
---

Review the uncommitted changes in this STM32F103 firmware repo (`git diff` and `git status`; also read new untracked files). Report findings in severity order with file:line references. Report only real problems, not style preferences. If there are none, say so.

Check specifically:

1. **CubeMX regeneration safety**: in CubeMX-generated files (`Core/Src/main.c`, `gpio.c`, `tim.c`, `adc.c`, `i2c.c`, `rtc.c`, `stm32f1xx_it.c`, `stm32f1xx_hal_msp.c`, and their headers), every change must be inside a `/* USER CODE BEGIN x */` … `/* USER CODE END x */` block. Anything else is deleted the next time CubeMX regenerates. The gap between `USER CODE END WHILE` and `USER CODE BEGIN 3` is not protected.
2. **Build wiring**: new `.c` files must be listed in the top-level `speedo-code/CMakeLists.txt` `target_sources`, not in `cmake/stm32cubemx/CMakeLists.txt`.
3. **Interrupt safety**: variables shared between ISRs/HAL callbacks and the main loop must be `volatile`. Multi-byte shared state needs to be read atomically. No blocking calls (`HAL_Delay`, `waitForMotor`, I2C transfers) inside ISRs.
4. **Main loop responsiveness**: `loop()` runs continuously and the stepper is driven from it. Flag anything that blocks long enough to stall the needle or the display.
5. **Arithmetic**: overflow and truncation in step counts, `HAL_GetTick()`/timer wraparound (use unsigned subtraction), and float use. The Cortex-M3 has no FPU, so floats are slow and pull in soft-float code.
6. **Resources**: large stack buffers, `printf`-family bloat, and anything that noticeably grows flash or RAM (64 KB flash, 20 KB RAM).
7. **Hardware assumptions**: pin and peripheral usage must match `speedo-code.ioc`. If a change needs a new peripheral or pin config, it must be done in CubeMX by the user, not hand-edited.
