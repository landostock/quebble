# Presentation And Store Kit 1.7

Native C emery watchapp, SDK 4.44. Prepared 2026-11-16.

## Changes

- Removed directional hints from the gameplay HUD and all touch-fork arrows
  around the marker. No steering, capture or difficulty rules were changed.
- Replaced four dense guide pages with three concise pages: goal, recommended
  touch controls, and a tabular button overview. All text remains English.
  The only guide illustration is an aligned native animated claim diagram.
- Buried historical lines now use GColorCobaltBlue, not warm-looking gray.
  Active boundaries remain white. Interior lines remain non-traversable.
- Added an original 25x25 launcher icon to the PBW.
- Prepared store-sized icons, separate original wordmarks, an illustrated banner,
  five native screenshots, copyable listing fields, privacy files and notices.

## Verification

`sh test/run_tests.sh` passes all suites, including motion (144000 wall frames,
12000 aggressive frames), runners, capture, controls, preferences, audio and input.
Guide navigation tests use QB_GUIDE_PAGES and cover the new three-page flow.
Game and route suites pass AddressSanitizer and UndefinedBehaviorSanitizer.

Native guide screenshots were inspected for text fit, row alignment, diagram
bounds and footer separation. Screenshots are emery-native 200x228 PNGs, not
mockups. The territory screenshot exercises a stopped touch fork: no HUD or
marker direction arrows remain. Its cobalt seam distinguishes a buried historic
line from a white exposed frontier. Gameplay screenshot setup is staged with
debugger data writes; no app or firmware functions are called by the debugger.
A malformed temporary debugger setup required restarting the emulator; the
final captures use the unchanged release PBW and a fresh emulator process.

Store artwork is validated against the current official submission sizes:
80x80 / 144x144 icons, 720x320 banner, up to five 200x228 emery screenshots.
The revised master is proportionally exported full-bleed, trimming excess height
without stretching or adding side bands. Edge-content assertions reject pillarboxing.
The icons and separate wordmarks use the project's existing pixel-glyph identity.
Banner illustration provenance is disclosed separately.
The archived native territory screenshot has 1354 claimed cells, score 2186 and
the marker at (63,42), an ambiguous touch fork. Its historic seam samples
RGB(22,99,141), while the exposed border samples RGB(255,255,255).
`test/test_store_package.py` independently verifies every archived file hash,
the exact release PBW, ZIP integrity, five upload images and eight gallery images.
The gallery covers title, first guide page, sound settings, two gameplay views,
HIT, Game Over and Level 2. All are distinct, unchanged 200x228 native captures.
HIT was held in its settled native transition state; Level 2 was initialized
through the game's native level-advance path, then captured while safely moving
on the outer edge so LEVEL 2 remains visible in the HUD. Game Over is staged.
The separate screenshots ZIP is checked against the exact gallery file bytes.

Build/install succeeded with the configured regular HOME; the suggested temporary
SDK HOME is absent. SDK RWX-segment and phone-permission notices remain.
No physical watch was available; the cooler hue needs the developer's hardware
check. No upload, login, account change or publication was performed.

## Release

Version 1.7.0, UUID 00d6400a-2f2c-4096-95f3-4db1cdc20f8b, emery only.
PBW: 264019 bytes.
SHA-256: d88326e305517d7fb5563642c023221e3e5710599154d09ae499990857c35bf1.
Resources: 229365 bytes / 256 KB. RAM footprint: 40906 bytes.
Heap before runtime allocations: 90166 bytes.

The kit includes public-hostable privacy copy, but no fictional privacy URL or
support address. The existing name/visual-similarity risk review is retained.
Packaging is not legal clearance and does not itself publish the app.
