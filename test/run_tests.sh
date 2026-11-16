#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
out=$(mktemp -d "${TMPDIR:-/tmp}/quebble-tests.XXXXXX")
trap 'rm -rf "$out"' EXIT
cc=${CC:-cc}
flags="-std=c99 -Wall -Wextra -Werror -O2 -DQUEBBLE_GRID_TEST -Isrc/c"
$cc $flags test/test_floodfill.c src/c/grid.c -o "$out/fill"
"$out/fill"
$cc $flags test/test_start_positions.c src/c/enemies.c src/c/grid.c -o "$out/start"
"$out/start"
$cc $flags test/test_motion.c src/c/enemies.c src/c/grid.c -o "$out/motion"
"$out/motion"
$cc $flags test/test_runners.c src/c/enemies.c src/c/grid.c -o "$out/runners"
"$out/runners"
$cc $flags -Itest/host test/test_game.c src/c/enemies.c src/c/grid.c -o "$out/game"
"$out/game"
$cc $flags -Itest/host test/test_routes.c src/c/enemies.c src/c/grid.c -o "$out/routes"
"$out/routes"
$cc $flags test/test_pixel_art.c src/c/pixel_art.c -o "$out/logo"
"$out/logo"
$cc $flags test/test_audio_mix.c src/c/audio_mix.c -o "$out/mix"
"$out/mix"
$cc $flags -DQUEBBLE_FEEDBACK_TEST -Itest/host test/test_feedback.c src/c/feedback.c src/c/audio_mix.c -o "$out/feedback"
"$out/feedback"
$cc $flags -Itest/host test/test_preferences.c src/c/enemies.c src/c/grid.c -o "$out/preferences"
"$out/preferences"
$cc $flags -Wno-unused-parameter -DQUEBBLE_INPUT_TEST -DPBL_PLATFORM_EMERY -Itest/host test/test_input.c src/c/input.c -o "$out/input"
"$out/input"
python3 test/test_audio_assets.py
