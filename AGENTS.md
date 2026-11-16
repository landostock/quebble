# AGENTS.md - Quebble

## Project

Native C watchapp for Pebble Time 2 (emery). It is an original
territory-capture arcade game named Quebble.

## Hard constraints

- Keep the watchapp native C.
- No floating point in watch code.
- Flood fill must stay queue-based and non-recursive.
- Keep game logic testable off-device where practical.
- Touch is primary on emery; buttons must remain fully playable.
- Do not use protected arcade names or copied assets in code, UI, package
  metadata, or store copy.

## Build and test

- Build: `HOME=/private/tmp/quebble_home pebble build`
- Emulator: `HOME=/private/tmp/quebble_home pebble install --emulator emery`
- Flood-fill test:
  `cc -std=c99 -Wall -Wextra -DQUEBBLE_GRID_TEST -Isrc/c test/test_floodfill.c src/c/grid.c -o /private/tmp/quebble_floodfill_test && /private/tmp/quebble_floodfill_test`

After UI changes, build and inspect an emery screenshot if an emulator is
available.
