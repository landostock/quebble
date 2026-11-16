# Audio And Guide 1.3

Date: 2026-11-16. Version 1.3.0, native emery, SDK 4.44.

## Audio Changes

The old pipeline closed its speaker stream after each isolated countdown cue.
The app now keeps a started stream open across countdown, play, claim pause,
hit and level clear, writing digital silence between effects. Turning both
Music and Effects off releases it even mid-round. Normal closes drain at least
four extra 256-sample silent blocks (128 ms). Focus loss, app exit and mute still
cancel immediately; asynchronous completion ownership remains guarded.

Resources, mixer and speaker writes now use 8 kHz signed 16-bit PCM. Resource
offsets are expressed in samples; the resource loader and speaker boundary
convert explicitly to bytes. This avoids the coarse quantization of quiet
8-bit effect tails without changing the chiptune composition. Mixer workspace
is stored in its static state, not a larger watch stack allocation.

Effects have smoothstep attack/release envelopes applied after resampling, so
resampler ringing cannot leave a nonzero final sample. Each effect has 4 ms of
leading and 32 ms of trailing silence, and the music loop also starts/ends at
exactly zero. Maximum signal normalization is unchanged in relative amplitude.
No new source assets or licences were introduced.

## Guide And Movement

Title Up or the left footer opens a three-page English guide:

1. THE GOAL: animated capture diagram, selected mode's claim target, exposed-line
   danger and edge enemies.
2. MOVEMENT: automatic travel, safe corners, stopping at forks, yellow heading
   tip and a left-facing wall diagram with relative turn labels.
3. CONTROLS: relative button turns, absolute swipes, stop/go, slow/fast and exit.

Select advances, then opens difficulty selection from the final page. Up/Down
choose previous/next; left/up swipes advance and right/down swipes go back.
Back or the Menu footer returns to the title; the guide does not change the
selected mode or draw speed. The game remains entirely English.

The user explicitly requested explanation rather than changed button semantics.
Up remains a left turn relative to the CURRENT heading; Down remains a right
turn. An automatic corner changes that heading. Thus, while facing left, Down
turns upward; once facing downward after a corner, Up turns right and can follow
an already safe approach boundary. Unfinished-trail backtracking stays forbidden.

## Verification

- `sh test/run_tests.sh`: all existing and new suites pass.
- Guide navigation, page bounds/reset, footer taps, no accidental start from
  content taps and unchanged relative turns in all four headings are tested.
- A wall fixture verifies the two distinct current-heading cases above.
- Feedback tests assert one stream for all three countdown cues, continued
  ownership with music off/effects on, release with both off, and at least
  128 ms of written silence before every graceful close. Partial writes,
  preemption, mute/focus/Quiet Time and late completion tests still pass.
- `test_audio_assets.py` checks the real 16-bit resources: zero endpoints,
  silent handles, bounded peaks and sufficient resource-budget headroom.
- Game, mixer and feedback suites passed AddressSanitizer/UndefinedBehaviorSanitizer.
- Built and installed successfully using the configured regular HOME; the
  suggested temporary HOME is absent. The SDK's RWX-segment warning and notice
  about absent phone-side permission declarations remain unchanged.
- Inspected native `guide-goal-13-emery.png`, `guide-movement-13-emery.png` and
  `guide-controls-13-emery.png`. Initially wrapping lines were shortened and
  the revised pages re-inspected without text overlap.
- `audio-13-native-emery.wav`: 56.296875-second QEMU recording at 50% app volume,
  with app haptics disabled to isolate audio. Peak 28149/32768; no full-scale
  clipped samples. Countdown, play, hits/restarts and final results ran through.
  The later countdown gaps are digitally silent. A small first-start transient
  remains measurable (peak 1024/32768 around 0.288 s); its physical-watch
  audibility/cause is not established, so this is not a claim that every hardware
  startup click is eliminated. No physical watch was available.
- `quebble-audio-13-preview.wav` is a separate deterministic software-mixer
  demonstration, not a physical-watch recording.
- Haptics were restored ON after isolation, confirmed in
  `sound-restored-13-emery.png`; music/effects ON, volume 50%.

## Artifact

- `build/quebble.pbw`: 263062 bytes.
- SHA-256: `60400ab77ed7638565fabdb0237e244e75424e24689fb47a803865bda17e03e7`.
- Resource pack: 229269 bytes / 256 KB.
- App RAM footprint: 40204 bytes, leaving 90868 bytes before runtime allocations.

Real speaker acoustics, mechanical vibration noises, battery use and production
watch frame pacing remain hardware checks. Open-stream silence during a round
can increase speaker power consumption when music is disabled.
