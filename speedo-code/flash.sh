#!/usr/bin/env bash
# Flash a preset's ELF over ST-Link with OpenOCD. Usage: ./flash.sh [Debug|Release]
set -euo pipefail
cd "$(dirname "$0")"

preset="${1:-Debug}"
cmake --build --preset "$preset"
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
	-c "program build/$preset/speedo-code.elf verify reset exit"
