#!/usr/bin/env bash
# Strict build of both presets, matching CI. Uses separate build dirs so -Werror
# doesn't leak into the normal build/Debug and build/Release caches.
set -euo pipefail
cd "$(dirname "$0")"

for preset in Debug Release; do
	dir="build/check-$preset"
	cmake --preset "$preset" -B "$dir" -DCMAKE_C_FLAGS=-Werror >/dev/null
	echo "== $preset"
	status=0
	output=$(cmake --build "$dir" 2>&1) || status=$?
	grep -v '^\[' <<<"$output" || true
	[ "$status" -eq 0 ] || exit "$status"
done
