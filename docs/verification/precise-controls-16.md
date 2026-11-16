# Precise Controls 1.6

Date: 2026-11-16. Native emery watchapp, SDK 4.44.

## Behaviour

- Up/Down always rotate the button heading by exactly 90 degrees left/right.
  A blocked turn updates the heading and stops instead of discarding the turn.
  Thus two turns can reverse a safe marker even through a blocked intermediate
  direction. A live trail still cannot be crossed or backtracked.
- Moving Select stops. Stopped Select proceeds straight, starting a cut if
  free space lies ahead inside the frame. If blocked, it tries the opposite
  direction, not an arbitrary perpendicular path. If that is also illegal it waits.
- A unique onward safe path auto-turns at both outer and territory corners.
  A straight safe heading continues through junctions; multiple turns without
  straight continuation wait. Automatic coasting never starts an exposed cut.
- Capture-pause quarter turns compose using the buffered heading. Select can
  request straight/back restart or cancel it with a second press during the pause.
- Touch recognition and game steering remain unchanged. The English guide
  recommends touch because swipes select screen directions directly. No new
  marker arrow is added; the previous yellow heading tip is removed.
- Active historical line borders stay white; buried, non-traversable historical
  lines use dark gray. Path/capture rules are unchanged.

## Historical-Line Research

Kevin Butler's arcade-specific guide, section 7, explicitly limits perimeter
travel to the boundary touching the playing field:
https://gamefaqs.gamespot.com/arcade/584113-qix/faqs/25178
This supports keeping buried interior lines inaccessible, while old lines
still bordering free space remain playable. It is a player-written guide,
not a claim to have inspected or run the original ROM. The cabinet service
manual's introductory gameplay section was also inspected, but does not
explicitly settle this particular boundary distinction. No original assets or
manual material are bundled in the watchapp.

## Verification

- `sh test/run_tests.sh`: all suites pass. Existing motion coverage includes
  144000 wall frames and 12000 aggressive frames.
- Exhaustive safe-neighbour masks cover straight/unique/ambiguous button routes
  and the unchanged touch path/fork/return rules in all four arrival directions.
- Regressions cover both approaches to all outer corners, an internal elbow,
  out-of-frame turns and two-turn reversals on each wall, queued quarter turns,
  straight-cut Select, safe return, capture buffering and no live-line reversal.
- A fixture reproduces the screenshot's buried vertical line: Select reverses
  onto the exposed boundary rather than entering claimed territory.
- Route and game suites pass AddressSanitizer and UndefinedBehaviorSanitizer.
- Native physical-button emulator events verified blocked Down-facing historical
  line -> Select -> Up/moving; Up-facing marker on the left wall -> Up button ->
  Left/stopped -> Up button -> Down/moving. Fixture setup changes data only,
  without debugger calls into firmware or app functions.
- Native screenshots inspected: `guide-16-goal.png`, `guide-16-buttons.png`,
  `guide-16-paths.png`, `guide-16-touch.png`, `route-16-old-line.png`.
  Pixel samples of the historical line are RGB(84,84,84), while the active line
  is RGB(255,255,255). The screenshot is an injected geometry fixture, not a
  live scoring round. No clipped guide text or overlaps observed.
- Build/install succeeded using configured regular HOME; suggested temporary
  SDK HOME is absent. Existing RWX and phone-side permissions notices remain.
  No physical watch was available.

## Artifact

- `build/quebble.pbw`, version 1.6.0: 265170 bytes.
- SHA-256: `c648a51af1343989cd552813711f473860c8bad81153461f25646781df446a91`.
- Resources: 229269 bytes / 256 KB.
- RAM footprint: 42360 bytes; heap before runtime allocations: 88712 bytes.
