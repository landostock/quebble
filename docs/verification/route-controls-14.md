# Route Controls 1.4

Date: 2026-11-16. Native C, emery, SDK 4.44. Supersedes the movement and guide
contracts recorded for versions 1.2 and 1.3; audio and difficulty are unchanged.

## Control Contract

On a safe path, the most recent actual movement identifies the approach road.
The next preview heading is separate and cannot change that identity.

- Exclude the approach road from normal choices, including directional swipes.
- Follow the sole forward exit automatically, including corners.
- Stop at every fork, including straight-plus-side branches. Never silently
  prefer straight, preselect an arm or change the approach heading while waiting.
- Up chooses the upper/left arm; Down chooses the lower/right arm. Screen order
  is up, left, right, down, independent of approach and preview orientation.
- Select/tap deliberately reverses safe travel. A dead end waits for that command.
- Holding Up/Down starts a cut into free space. If more than one opening exists,
  the same screen ordering chooses between them. A directional swipe into free
  space also starts a cut. Ordinary path choices never start a cut accidentally.
- While drawing, Up/Down steer relative left/right; Select/tap stops or resumes
  without reversing. Even the first exposed step cannot reverse into its safe
  origin. Hold Select chooses Slow/Fast for the next cut; a started cut keeps its
  original speed/scoring classification.
- Commands during the one-second capture pause are stored as intent, not as a
  turn from a mutable preview heading, and applied only after the pause ends.

The whole game does not pause for a fork: only the marker waits. A player's
return is always subject to the frontier check; buried historical seams are
still visible but not traversable. If capture recovery projects a buried marker
onto the frontier, it clears the invalid previous-arrival record.

Rare four-way grid nodes have three forward exits. Up/Down expose the outer
choices, and a swipe can pick the middle directly. Buttons alone can reach all
arms too: enter an outer arm, return explicitly with Select and approach the
remaining arm from the new orientation. This is covered by a crossing regression.

## UI And Guide

Fork HUD: cyan Up and yellow Down, each with its actual directional arrow.
Matching arrows appear beside the marker, inset at screen borders for visibility.
Dead ends show SELECT BACK. Active drawing shows CUT / SLOW CUT using the
actual trail's classification; SLOW NEXT describes a selected next-cut setting.

The four-page English guide has separate concrete diagrams for:

1. THE GOAL: animated territory closure and the selected mode's claim target.
2. ON THE EDGE: an animated marker following a single corner automatically.
3. CHOOSE A PATH: Up above, Down below, Select separately pointing back along
   the approach road. These are no longer labels for relative turns.
4. MAKE A CUT: speed selection before starting, hold-to-cut, relative live-line
   turns, stop/go and Back/Exit.

All guide pages and the revised fork HUD were inspected at native 200x228.
The initial too-narrow DOWN label was widened; the release screenshot shows the
full word without ellipsis and keeps the wave timer separate.

## Verification

- `sh test/run_tests.sh`: passed all suites, including the existing 144000 wall
  frames plus 12000 aggressive enemy-motion frames, audio, fill and preferences.
- `test_routes.c`: exhaustively checks all 32 approach/exit-mask combinations
  with a real approach road. Tests sole exits, forks, dead ends, stable repeated
  Up/Down, rejection of return swipes and intentional Select reversal.
- Additional fixtures exercise the original left-wall case, automatic corners,
  short versus held input, a first-step live-line return rejection, capture-pause
  path/cut/return buffering and a real territory closure followed by safe return
  along the newly committed line. No movement occurs during the capture pause.
- Crossing regression proves the middle arm remains reachable with buttons.
- Current-cut speed remains unchanged when the next-cut setting is toggled.
- Game and route suites passed ASan/UBSan.
- Native emulator: controlled left-wall T junction using the actual button
  handlers, with enemies temporarily disabled only in debugger data:
  Up moved along the left wall toward the top, Down toward the bottom; neither
  entered the approach line. Select returned along it to x=6, y=20.
- A native 450 ms Up hold on a straight safe border began a rightward cut into
  free space. On release it remained rightward: no extra short-click turn and
  no implicit Slow setting. Two Select clicks then produced moving=false/true
  while preserving the drawing direction. The app-crash breakpoint did not fire.
- `route-14-left-wall-choice.png` shows the native choice state. The guide images
  are `guide-14-goal.png`, `guide-14-edge.png`, `guide-14-fork.png`, `guide-14-cut.png`.
- Debuggers were detached, all diagnostic sessions closed and the final build
  reinstalled normally. The emulator was returned to the title.

The configured regular HOME was used; the suggested temporary HOME is absent.
The SDK's RWX-segment linker warning and phone-permission notice are unchanged.
No real watch was available; physical input feel and production frame pacing
remain hardware checks.

## Artifact

- `build/quebble.pbw`: version 1.4.0, 264930 bytes, emery only.
- SHA-256: `c3ad880b949043439cee5cb4cb459f5f11336f923f9c2045fae3d0b19e13793d`.
- Resources: 229269 bytes / 256 KB.
- App RAM footprint: 42120 bytes; available heap before runtime allocations: 88952.
