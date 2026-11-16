# Relative Button Controls 1.5

Date: 2026-11-16. Native emery build, version 1.5.0, SDK 4.44.

## Behaviour

Restores relative Up/Down quarter turns on both safe paths and live cuts.
Select stops/resumes, with a safe perpendicular restart when blocked (left,
then right). Long Up/Down also selects Slow; long Select toggles Slow/Fast.
Entering free space needs only a short turn command. Invalid turns preserve
the current heading and never backtrack an unfinished line.

Button movement continues straight through junctions. Automatic turns are
restricted to the four physical outer corners with one onward safe exit.
Internal corners stop. Automatic continuation cannot start a cut.

Touch keeps its 1.4 behaviour: absolute gestures, sole-safe-path following,
fork waiting, safe-path tap reversal and live-cut tap Stop/Go. The last accepted
steering/stop input chooses its movement rules. Native gesture registrations
and recognition logic are unchanged. Guide and fork HUD labels distinguish
touch from the restored buttons.

## Verification

- `sh test/run_tests.sh`: all suites pass, including the existing motion,
  runner, flood-fill, input, audio, feedback and preference regressions.
- Exhaustive safe-neighbour masks test both button straight continuation and
  the unchanged touch path/fork/return rules for all four arrival directions.
- All eight approaches to the four outer corners pass, with contrasting
  internal-corner cases, relative turns, Select stop/resume, short-press cuts,
  control-source switching, capture buffering and actual captured-line return.
- Game and route suites pass AddressSanitizer and UndefinedBehaviorSanitizer.
- Native emulator button events verified westbound arrival at the left wall:
  Up produces Down, Down produces Up. Select resumes Right without reversing,
  then stops with the same Right heading. Native top-left corner inspection
  confirmed position (0,0), arrival Up, outgoing Right and movement enabled.
  Debugger fixture setup changed data only; no app/firmware functions were called.
- Native guide screenshots inspected: `guide-15-buttons.png`,
  `guide-15-corners.png`, `guide-15-touch.png`. No clipped text or overlaps.
- Build and install succeed. Used the configured regular HOME because the
  suggested temporary SDK HOME is absent. Existing RWX linker and phone-side
  permission notices remain. No physical watch was available for testing.

## Artifact

- `build/quebble.pbw`: 265254 bytes.
- SHA-256: `a7170737e77b82485b85de1eb1239ce998dfbda53bdcf682898cf496ecf277ee`.
- Resources: 229269 bytes / 256 KB.
- RAM footprint: 42442 bytes; heap before runtime allocations: 88630 bytes.
