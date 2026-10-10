---
description: Check the repo after regenerating code from STM32CubeMX
---

I just regenerated code from `speedo-code/speedo-code.ioc` in STM32CubeMX.

!`git status --short`

!`git diff --stat`

1. Read the full `git diff`. Look for hand-written code that CubeMX deleted, especially calls to `setup()` / `loop()` in `main.c` and anything that was outside `USER CODE` blocks. Restore the lost code inside the correct `USER CODE` block.
2. Check whether CubeMX changed anything else unexpected: the top-level `CMakeLists.txt` user sources, linker script, startup file, `stm32f1xx_hal_conf.h`, or new or removed peripheral files. Summarize the intentional `.ioc` changes in a sentence or two.
3. If new HAL driver sources were added, confirm they're in `cmake/stm32cubemx/CMakeLists.txt`.
4. Run `speedo-code/check.sh` and fix any failures in hand-written code.
5. Report what changed and what you restored. Don't commit; I'll review first.
