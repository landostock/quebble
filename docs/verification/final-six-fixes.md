# Six-Point Verification

Historical verification of the previous build. Version 1.0.1's revised runner
shapes and current binary hash are documented in `fuzzy-curls.md`.

Verified on 2026-11-15 with SDK 4.33.1 and the emery emulator.

Build: `pebble build`. PBW SHA-256:
`729233a909770872c1faa4ec0e7f332faf65f819b2567a8da6f750828b399093`.
RAM footprint: 30,720 bytes; available heap: 100,352 bytes.
The SDK's existing RWX linker warning remains; the build succeeds.

## Automated Checks

- `sh test/run_tests.sh`: all suites pass with `-Wall -Wextra -Werror`.
- 144,000 empty/concave wall frames and 12,000 aggressive-stage frames retain
  exact pixel wall/trail checks, history continuity and maximum endpoint steps.
- Game, movement, runner and pixel-art tests pass AddressSanitizer and
  UndefinedBehaviorSanitizer checks.
- Runner tests cover fixed speed, one-pixel interpolation, animation-clock wrap,
  blocked movement without jitter, eight body phases, contour routing, added
  pairs and physical super pursuit instead of remote trail kills.
  Every visible body pixel must belong to a filled two-by-two block, with at
  least 24 distinct visible body pixels in every phase; rendering and collision
  use this same thicker brush. Tests also check that edge bodies remain unclipped.
  The starting pair is separated so the thicker bodies cannot merge at countdown.
- Game tests cover all level targets through level 255, a non-wrapping level cap,
  countdown/frozen states, final HIT before results, persistent results lighting,
  retry/menu input and timer rounding.
- Pixel-art tests cover animated logo/words/digits, bounds and stable settled art.

## Native Visual Checks

- `final-hud-one-second.png`: 100% progress, six-digit score and one-second timer.
- `final-hud-super.png`: the short super-state warning remains attached to its icon.
- `final-level-seven.png`: two interior enemies, four separated blue super bodies.
- `final-countdown.png`, `final-hit.png`, `final-results.png`: centered pixel-art
  transitions, no arbitrary red side strokes and no clipped labels.
  Actors are hidden during HIT/CLEAR to avoid partly masked sprite fragments.
- A 900-frame, 30-second monitor capture covered final HIT and results. The last
  playfield frame was 83; results stayed visible for the remaining 27.2 seconds.
  There were no completely black frames. Frames at 10, 20 and 29 seconds were
  identical, with maximum brightness 255. Select then returned to a fresh
  three-life countdown (`final-retry.png`); Back returned to the title.
- `final-six-fixes-emery.mp4` is a 15-second native-renderer preview, nearest-neighbor
  scaled to 400 by 456: normal play/countdown followed by the final-HIT/results
  sequence. `final-six-fixes-overview.png` groups game/HIT/results stills.

Normal play was captured without changing game state. To exercise rare boundary
cases promptly, debugger-controlled fixtures set the final-life collision,
level-clear transition, level-seven entry and extreme HUD values. The normal
game-update/rendering code handled the subsequent transitions; no fixture code
or debugger hook is compiled into the watchapp. The emulator high score was
restored to its original value after score-layout fixtures.

Reference evidence, original-versus-watch tuning distinctions and visual
derivations are recorded in `../reference-derivation.md`.

Physical-watch performance, battery consumption and subjective comfort have not
been verified on hardware. Emulator QA is not a claim of hardware perfection.
