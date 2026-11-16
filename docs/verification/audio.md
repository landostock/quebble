# Speaker And Haptics Verification

Date: 2026-11-15. Version: 1.1.0. Platform: emery, SDK 4.33.1.

## Build

`pebble build` and `pebble install --emulator emery` succeeded.
The installed SDK is configured in the regular HOME, not the suggested temporary
HOME. The linker emits the SDK's RWX LOAD-segment warning.

- PBW: `build/quebble.pbw`, 182712 bytes.
- SHA-256: `dd4306fbb49f6ba1d8470225a6f08590a81356198ce8d8873bf0e9a6b88a7d01`.
- App RAM footprint: 35042 bytes; available heap: 96030 bytes.
- Resources: 152816 bytes of the 256 KB budget.

## Automated Checks

`sh test/run_tests.sh` passed, including all existing territory, movement,
runner, transition and bitmap-glyph tests. New tests cover the integer mixer,
music looping, chunk-size independence, effect priorities, ducking, fades,
partial speaker writes, bounded refill work, mute, Quiet Time, focus loss,
preemption, retries and cleanup. Sound preferences and menu navigation are
covered by game tests. Mixer and feedback tests also passed ASan/UBSan.

A native test exposed an asynchronous completion race: a late completion from
an old stream could mark its replacement unowned. New streams now wait for the
old completion event, including after an explicit stop. Regression tests defer
both Done and Stopped callbacks until after firmware reports Idle.

## Native Emulator Checks

- Inspected the English Sound and Audio credits screens at native resolution.
- `sound-settings-emery.png`: Music, Effects and Haptics enabled, Volume 25%.
- `audio-gameplay-emery.png`: active gameplay after a successful 48% capture.
- `audio-native-emery.wav`: 20.596375-second QEMU speaker recording, 8 kHz mono
  signed 16-bit. Haptics were disabled to isolate speaker output, then restored.
  This is actual emulator output, not the standalone mixer preview.
- Recording peak: 29580/32768; zero full-scale clipped samples. Countdown/start,
  background music and event cues were exercised. An earlier louder effect mix
  clipped; the final mix uses 50% effect gain instead of 75%.
- The debugger's application-crash breakpoint did not trigger during the final
  streamed playback run. Earlier exploratory audio recordings are superseded
  by this WAV and are not release evidence.

`quebble-audio-preview.wav` is a separate deterministic 20-second software-mixer
demonstration of countdown, start, music, capture, UI, hit and game-over. It does
not model physical speaker gain, enclosure acoustics or vibration.

## Limits

No physical watch was available. Speaker loudness, physical sound quality,
haptic feel, battery drain and real-device frame pacing still need hardware
testing. Emulator output cannot establish those properties. The app respects
system audio mute/Quiet Time and does not use the microphone or fetch audio at
runtime. Provenance and CC0 declarations are in `resources/audio/SOURCES.md`.
