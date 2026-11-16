# Audio Sources

All downloads were checked on 2026-11-15. Only adapted PCM clips and `credits.txt`
are bundled as app resources. Original archives stay outside the project.

## Music

- Creator: Juhani Junkala / SubspaceAudio.
- Track: Title Screen, from Retro Game Music Pack / 5 Chiptunes (Action).
- Source: https://opengameart.org/content/5-chiptunes-action
- Licence: CC0 1.0, explicitly stated by the uploader on the source page.
- Archive: https://opengameart.org/sites/default/files/5%20Action%20Chiptunes%20By%20Juhani%20Junkala.zip
- Archive SHA-256: `ea189990c0fb9a700187ccc740eda72400341a082305a163867467d96346460d`.
- Selected WAV SHA-256: `d1590ec1b4a9e0ff948437c8e1d7f416ee382600af6e454fa0452903d8de1964`.
- Included INFO.txt also explicitly releases these tracks as CC0.
- Adaptation: complete 11.294127-second loop, mono, 8 kHz signed 16-bit PCM,
  two-pole 420 Hz high-pass, +4 dB presence EQ at 1.2 kHz, 3.4 kHz low-pass,
  peak normalization to 25600/32768, 2 ms edge fades. Runtime gain is 40%, ducked
  quickly to 12% during cues. Replaces the bass-heavy version 1.1 loop.

## Effects

- Creator: Kenney Vleugels / Kenney.nl.
- Collection: Digital SFX Set (2012), released as CC0 on the source page.
- Source: https://opengameart.org/content/63-digital-sound-effects-lasers-phasers-space-etc
- Archive: https://opengameart.org/sites/default/files/Digital_SFX_Set.zip
- Archive SHA-256: `022f0f4b73da55989567fe2b373c8af89d74f5d015257e293cab497ca606d063`.
- Included source readme permits personal/commercial use and says credit is optional.
- Mapping: pepSound3 to UI/countdown, powerUp2 to start, powerUp7 to claim,
  lowDown to hit and a slower game-over variant, highUp to clear.
- Adaptation: short excerpts, 160 Hz high-pass/3.2 kHz low-pass, normalization,
  smoothstep 6 ms fade-in and up to 40 ms fade-out, applied after resampling
  with exactly zero endpoints; four milliseconds of leading and
  32 milliseconds of trailing digital silence. The same 8 kHz signed 16-bit
  format is mixed at 50% gain for headroom. No proprietary game
  music or extracted ROM audio.

CC0: https://creativecommons.org/publicdomain/zero/1.0/
Legal text: https://creativecommons.org/publicdomain/zero/1.0/legalcode.en
Attribution is included voluntarily and does not imply endorsement.

## Rebuild

Place the selected original WAV and named MP3 files in one source directory,
then run `python3 tools/prepare_audio.py /path/to/source_dir`. The converter uses
ffmpeg plus the Python standard library. Floating point is used only off-device
for conversion; the watch mixer and streaming code are integer-only.
