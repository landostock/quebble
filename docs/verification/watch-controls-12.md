# Watch Controls And Presentation 1.2

Date: 2026-11-15. Native emery build, version 1.2.0.

## Decisions

The reference uses direction-controlled joystick input, not this watch's
impulse-driven auto-movement. The published Taito instructions specify moving
up/down/left/right and a separate drawing input:
https://pixelatedarcade.s3.us-east-005.dream.io/pdf/Game/502/QIX-IBM-Instructions.pdf
This is documentation of a home-computer port, not proof of every original
arcade ROM's post-capture timing or corner arbitration. No frame-exact claim
about all original junction behaviours is made.

Quebble deliberately uses a watch-specific rule: one second of capture pause,
then safe-edge coasting. Straight wins, a unique 90-degree corner is followed,
and an ambiguous fork stops. At a left/right fork the stationary marker faces
the safe right turn, shown by a yellow tip; Select resumes that heading, or an
absolute direction gesture chooses another. Coasting cannot create a new
exposed trail. A command during the capture pause is buffered, and a tap can
cancel automatic continuation. Invalid commands never replace a valid heading.

Fast player movement is now 99 ms/cell rather than 132 ms, about 33% faster.
Slow is 165 ms rather than 231 ms. Edge enemies remain at 198 ms/cell across
all modes and their normal/super states. Their danger increases through count,
earlier reinforcement and access to unfinished trails, not greater speed.

Easy / Standard / Expert use 4/3/2 lives, 65/75/80 percent claim goals, and
45/30/22-second pressure intervals. Second interior enemies appear at levels
5/3/1; four edge enemies start at 7/5/3; super enemies start at 9/7/5. Fuse grace
is 33/21/15 ticks. Separate best scores prevent cross-mode comparisons. The
existing score key belongs to Standard. Preferences and the selected mode persist.

## Touch Implementation

SDK horizontal and vertical pan recognizers commit an absolute heading once
at Started; Updated/Completed/Cancelled cannot repeat it. The tap recognizer
handles tap commands exclusively. The SDK requires more than 10 px with a
clear dominant axis. Pan was chosen instead of swipe because no minimum flick
velocity is needed. Liftoff is not a second movement or stop command.

https://developer.repebble.com/guides/events-and-services/touch/

Native testing caught a required opt-in: attaching recognizers alone did not
subscribe this third-party app to touch dispatch on SDK 4.44. The app now enables
the touch dispatcher and disables the window's button-emulation bridge, so its
own recognizers receive touches without synthesized extra button clicks. Cleanup
removes the opt-in; buttons remain available when touch is disabled.

## Audio And Haptics

The old bass-heavy loop was replaced by Juhani Junkala's CC0 Title Screen loop.
Conversion removes bass and emphasizes presence; the resulting PCM has 4.83%
of spectral energy below 300 Hz and a spectral centroid near 1169 Hz. The music
mix gain is 40%, ducking to 12% for effects, rather than the earlier 24%/8%.
New installs default to 50% app volume. Volume supports 0/25/50/75/100%; existing
saved choices remain respected. Provenance is in resources/audio/SOURCES.md.

Each haptic event is a single pulse: countdown 10 ms, start 28 ms, claim 32 ms,
hit 110 ms and clear 65 ms. Game-over has no second pulse after the final hit.
Steering, stop/go and warnings do not buzz. System mute and Quiet Time remain
respected. The watch's own Sounds & Haptics menu controls Volume and Mute Speaker:
https://raw.githubusercontent.com/coredevices/PebbleOS/main/pebble/apps/system/settings/vibe_patterns.c

## Verification

The active toolchain changed from SDK 4.33.1 to 4.44 during work. The release was
rebuilt and installed with 4.44. The configured regular HOME was used instead of
the unconfigured temporary HOME. Build succeeds with the SDK's RWX-segment
warning and its notice about absent phone-side permission declarations; no
network or additional phone permissions are requested.

- `sh test/run_tests.sh`: all suites passed, including 144000 wall frames and
  12000 aggressive movement frames from the existing enemy tests.
- New regressions exercise the three mode progressions, absolute controls,
  blocked inputs, no exposed-trail backtracking, auto-corners, forks, Select
  resume, capture-pause buffering, preference validation and separate scores.
- Input adapter tests verify one action at Started, no action at Updated or
  Completed, tap routing, dispatcher opt-in, button fallback and allocation failure.
- Feedback tests verify single-pulse haptics, a 110 ms hit, no extra game-over
  pulse, no stop/go pulse, partial audio writes, asynchronous completion races,
  mute, focus, Quiet Time, preemption and cleanup.
- Game, preference and feedback suites passed ASan/UBSan.
- Native VNC touch events, not direct game-function calls, exercised taps and
  14-pixel up/down/right gestures. Debugger traces showed the expected absolute
  direction and no tap on liftoff. Repeated right gestures kept the player moving
  on the safe border; it subsequently rounded the outer corner automatically.
- `difficulty-emery.png`: inspected final selected-mode layout.
- `difficulty-animated.gif`: 40 native frames showing pixel assembly and the
  growing selection underline. Animated tiles are clipped to their own text band.
- `easy-12-emery.png` / `expert-12-emery.png`: native HUD, life counts, targets
  and wave timers checked. `sound-settings-12-emery.png` shows restored ON
  preferences and 50% volume after audio isolation tests.
- `hit-full-field-emery.png`: injected fast/slow capture strips behind a held HIT
  state remain fully invisible. Pixel checks confirm black interior at both strip
  columns and an intact white frame. HIT/CLEAR skip rendering the game grid and
  actors entirely; the transition paints a full opaque field inside the frame.
- `audio-12-native-emery.wav`: 35.301125 seconds of native QEMU audio at 75% app
  volume, with haptics temporarily disabled. Peak 28153/32768, zero full-scale
  clipped samples. Captures countdowns, gameplay, capture/death cues and resumed
  playback. `quebble-audio-preview.wav` is separately a deterministic software mix,
  not a physical-watch recording.

No physical watch was available. Real loudness, vibration feel, battery drain
and production-hardware frame pacing remain unverified.

## Release Artifact

- `build/quebble.pbw`: 147255 bytes.
- SHA-256: `95156c588a342f08069a787ddceac0db2ef0d66a79a7e2b71d2490ace04d8a5e`.
- Resources: 115242 bytes / 256 KB.
- RAM footprint: 37152 bytes / 128 KB, leaving 93920 bytes heap before runtime
  allocations such as recognizers and windows.
