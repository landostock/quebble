# Artwork And Screenshot Provenance

Prepared for Quebble 1.7.0 on 2026-11-16.

## Banner

Original AI-assisted pixel illustration created for Quebble with the built-in
image-generation tool. The user's own Brilliant Snake and Brilliant Hydration
artwork supplied composition/craft references; the native Quebble title supplied
the brand and palette reference. Those reference images are not redistributed
in this package.

Public style reference supplied by the developer:
https://apps.repebble.com/brilliant-snake_06f4a8615e144b58ad65f60c

The revised generated master extends the illustrated territory to both side edges.
The large master remains in the source repository, not the upload kit. Use the
root-level UPLOAD-BANNER-EMERY-720x320.png for submission; the inline chat preview
is not the size-constrained upload export.
The 720x320 upload version is a proportional full-bleed crop with a small vertical
trim, without padding or distortion. The complete wordmark, line fan and player
remain visible. The banner is a marketing illustration, not a screenshot or a promise
of different game graphics. No historic game's wordmarks, sprites or music were used.

Direction: a black arcade field, chunky cyan stepped territory with white contours,
cool blue historical seams, Quebble's italic white/cyan pixel title with magenta
shadow, an abstract seven-line multicolour fan, a diamond marker and yellow/red
sparks. No extra slogan, hardware mockup, fake buttons or third-party branding.

## Icons And Logos

The separate logos are deterministic exports of the original seven glyphs in
`src/c/pixel_art.c`, including the row-dependent italic offset and colour split.
The icons derive the Q glyph, stepped territory and player diamond from that
existing code-native art. Their editable exporter is `tools/prepare_store_art.py`.
They are not crops of the generated banner or reference-app art.

## Screenshots

The eight-image gallery and five-image upload selection are native 200x228 emery
emulator PNG captures of the release build, without a watch frame, compositing
or added lettering. Title, settings and guide are captured from the real menus. Gameplay scenes
are representative staged game states rendered by the native app; debugger
setup changes game data, not firmware or rendering functions. They are not
photographs of a physical watch and do not establish physical display colour
accuracy. Only the illustrated banner is AI-generated.

HIT is a held native transition; Game Over is a staged results view. Level 2
was advanced through the native level-init path and captured with LEVEL 2
visible in the HUD. These are presented as app views, not as a continuous run.
