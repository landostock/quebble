# Reference Derivation

## Evidence

The original manufacturer's service bulletin 81-16, page 12 of the
[archived service collection](https://arcarc.xmission.com/PDF_Misc/Taito%20Service%20Bulletins%201980-1983/Qix.pdf),
describes four frame types: one random interior enemy, one more aggressive enemy,
two random enemies and two aggressive enemies. Aggression has operator-adjustable
settings 0-3 and changes how much the enemy considers the marker's position.
It also describes a configurable time line: one expiry releases two additional
edge enemies; the next turns them blue and allows pursuit during drawing.

The [current rights-holder's arcade release description](https://www.arcadearchives.com/en/title/aca-198/)
states a 75% claim target. The original operator manual likewise describes a
prescribed claim threshold, not an automatic two-percentage-point increase.
Different home versions have different thresholds and enemy rules. Their values
must not be silently combined into a purported exact arcade rule table.

The user's 75.68-second reference recording was reviewed as a sequence and with
50 close-up frames around 1.0-1.8 seconds. The edge enemy has red/yellow body
elements around a bright center partly formed by the underlying white border.
Body elements extend, retract and shift rather
than moving as a rigid rotating four-arm symbol. The sampled warm-pixel masks
change at unequal intervals. The field border remains visible behind the sprite.
The recording does not establish the original random-number generator, exact
sprite data, routing probabilities or a complete level-by-level difficulty table.

## Implemented Model

- Native, original procedural art; no traced/captured reference sprites.
- Eight morphing runner phases with 2/3-tick dwell times, independent of movement.
- Three asymmetric short stepped curls, with gaps and an open center through
  which the white field border shows. There is no artificial solid white core.
- Whole red/yellow tiles, not per-pixel colour noise; cyan/blue in the super state.
- A two-by-two-pixel brush thickens every body element for watch visibility.
  The same footprint is used for collision; no invisible hitbox enlargement.
- The coloured sprite visitor supplies both native rendering and collision masks.
  Shapes are generated from a small rim path with independently changing curl
  lengths and offsets, not imported/traced reference frames.
- Transparent drawing over the existing grid, not a black rectangular halo.
- Fixed runner speed of one three-pixel cell every six 33ms ticks: slower than
  the normal player and faster only than slow drawing, as requested.
  Pixel interpolation avoids three-pixel visual jumps; collision and rendering
  use the same interpolated center, with at most one pixel of travel per tick.
- A super runner physically follows unfinished territory lines. Contact with
  the marker's actual sprite, not remote contact with any trail cell, costs a life.
- The first four interior-enemy configurations follow the documented pattern.
- Random stages 1/3 do not inspect player position. Aggressive stages occasionally
  blend a newly chosen random heading with the marker direction while drawing;
  they retain random translation, length, spin, pivot and intensity envelopes.
- Every visible monster pixel and swept pose still obeys the same wall and trail
  tests, with the existing maximum endpoint step; aggression bypasses no limits.
- Fixed 75% target and split-enemy progression/multiplier remain active.

## Watch-Specific Parameters

The wave duration is 30 seconds. Stages 1-4 start with two runners, add two after
one wave, and enable super pursuit after the next. Stages 5-6 start with four
normal runners; stage 7 onward starts with four super runners. Extra starting
runners are separated along the top border, not placed on top of the first pair.
Aggression settings are 0, 1, 0, 1, 2, 2, 3 (capped thereafter). On a heading
change, the chance of a player-direction blend is setting/8, only while drawing.
These concrete thresholds and probabilities are bounded watch adaptations of
the documented pressure mechanisms, not recovered original ROM constants.

## Presentation Checks

The timer's icon is anchored to measured text width, four pixels before the
number. HIT/CLEAR are centered tile-assembled words with centered status below;
there are no decorative side strokes. Each countdown digit reassembles over
the first half of its second. The last HIT runs before the animated results
screen; the results screen holds the backlight and waits for an explicit action.
Title/help, loss of focus and exit return lighting to system control.
