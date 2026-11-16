# Fuzzy Shape Revision - 1.0.1

The reference recording and the earlier 50-frame contact sheet were reviewed
again. The previous four-arm shape and per-pixel colour noise produced a solid
star rather than the broken, stepped spark shapes in the recording. Part of
the apparent white center in that recording is the field border underneath it.

The new original procedural model uses three short asymmetric stepped curls.
Curl lengths and offsets vary across eight phases, with solid red/yellow blocks,
gaps and an open center. It retains the 10-pixel maximum body span, two-by-two
pixel elements, unequal phase durations, smooth movement and fixed runner speed.
No reference bitmaps were imported or traced. This is a watch-sized interpretation
of the observed form and motion, not a claim of pixel-identical original art.

The same coloured pixel visitor now supplies rendering and collision geometry.
Regression tests compare both masks and check palette presence, uniform colour
within each two-by-two block, a transparent center, all eight shapes, unclipped
edges, physical pursuit and constant speed.

## Verification

- `sh test/run_tests.sh`: all suites pass, including 144,000 wall frames and
  12,000 aggressive-stage frames.
- Runner tests also pass AddressSanitizer and UndefinedBehaviorSanitizer.
- `pebble build` succeeds with SDK 4.33.1; the existing SDK RWX warning remains.
- Installed and captured 12 seconds of native emery countdown/play. Returned
  the emulator to the title after inspection. No hardware testing was performed.
- PBW version label: `1.0.1`; app UUID unchanged.
- RAM footprint: 30,688 bytes; free heap: 100,384 bytes.
- PBW SHA-256: `7557d4381840a5e48cdbce38368b71092aacc24f8f2fbbee4e41b5de371c7688`.

## Visual Evidence

- `fuzzy-curl-phases.png`: all eight phases, rendered from the C sprite visitor.
- `fuzzy-curls.gif`: the full cycle enlarged with nearest-neighbor scaling.
- `fuzzy-curls-watch.png`: a native emulator frame.
- `fuzzy-curls-emery.mp4`: the full native 12-second capture at 2x scale.

To regenerate the phase sheet:

```sh
cc -std=c99 -Wall -Wextra -Werror -DQUEBBLE_GRID_TEST -Isrc/c test/render_runners.c src/c/enemies.c src/c/grid.c -o /private/tmp/quebble_render_runners
/private/tmp/quebble_render_runners /private/tmp/quebble-runner-curls.ppm
```

The earlier six-point verification remains historical evidence for the preceding
version. Store name/rights questions in `../store-readiness.md` remain unresolved;
providing this local test binary does not imply public-store legal clearance.
