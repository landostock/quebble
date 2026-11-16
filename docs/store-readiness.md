# Store Readiness Review

Date: 2026-11-15. Intended distribution: a free hobby watchapp, published by an
individual in the Pebble/Core Devices store. Legal discussion assumes Germany/EU;
it is an initial risk review, not legal advice or worldwide rights clearance.
No app or developer-account data was submitted to a store.

## Release Status

Technical testing is recorded in `verification/final-six-fixes.md`. Public-release
clearance is still open. A working build, free price and private developer status
do not establish permission to use another party's protected expression or name.

## Name: Action Required

The exact name is already used by a Dutch software/app provider:
[Quebble's own site](https://www.quebble.com/) and
[its product overview](https://www.quebble.com/over-ons/).
This establishes prior public use, not a confirmed registered trade mark or an
automatic finding of infringement. Registry status, relevant goods/services,
similarity and applicable commercial-use requirements were not fully cleared.
Search-engine results are not a trade-mark clearance search.

Before publication, either obtain a proper name assessment/permission or choose
a more distinctive name and check that name too. The app has not been renamed.
The [DPMA guidance](https://www.dpma.de/marken/markenrecherche/) recommends checking
German, EU and international registers, similar signs and prior commercial names.
Do not infer that a different application category alone resolves a conflict.

## Rules Versus Protected Expression

[Section 69a UrhG](https://www.gesetze-im-internet.de/urhg/__69a.html) distinguishes
software expression from underlying ideas and principles. This supports an
independently implemented territory-capture mechanic, not wholesale reuse of
another game's code, characters, artwork, audio, texts or distinctive presentation.
[Section 23 UrhG](https://www.gesetze-im-internet.de/urhg/__23.html) also requires
sufficient distance from a used work, or permission for a protected adaptation.
Public store downloads are public distribution/access, not merely private play;
see [section 19a UrhG](https://www.gesetze-im-internet.de/urhg/__19a.html).

The changes made here use procedural art and original bitmap glyphs rather than
imported ROM sprites. However, the explicit goal of reproducing the reference's
monster and runner presentation leaves a visual-similarity question. Independently
written code does not settle that question. Review the final presentation or
obtain a licence before marketing it as a faithful recreation. Do not use original
game names, logos, reference screenshots or recordings in the store listing.
The age of the reference game is not evidence that it is public domain.

## Pebble Store Obligations

The current [distribution agreement](https://developer.repebble.com/legal/distribution/)
and [program policies](https://developer.repebble.com/legal/program-policies/) apply
to free apps too. They require rights compliance, accurate metadata and no confusing
impersonation. Clause 5.3 requires a prominent privacy policy describing collection,
use and sharing. You remain responsible for the product; the agreement includes
distribution/marketing licences and indemnity provisions that deserve review.
The [publishing guide](https://developer.repebble.com/guides/appstore-publishing/publishing-an-app/)
states that apps go live without a review queue. A listing is not a legal clearance.

## Privacy And Dependencies

Update for version 1.1: the app now bundles CC0 music/effects. The downloaded
sources, hashes, licence declarations and adaptations are documented in
`../resources/audio/SOURCES.md`, with credits also included in the PBW and UI.
Playback is offline; it adds no microphone access, runtime downloads or analytics.
The app name remains unchanged at the author's request; this is not a name clearance.

Source and bundled-JavaScript inspection found no app-owned network calls,
advertising, analytics, account system or health/location access. The high score
is stored locally through the watch persistence API. Store/OS data practices are
separate and must not be described as though this app controls them.

Suggested factual privacy wording for the current version, to complete with an
appropriate developer contact and public policy location before submission:

> This game runs offline. The app does not collect or transmit personal information
> and includes no advertising or analytics. Your high score is stored locally on
> your watch. App-store and platform services are subject to their providers' own
> privacy policies.

No project-level licence was found. Do not automatically license the author's
entire project as open source. Check provenance/licences of any code carried over
from earlier example apps. The built JavaScript also contains an SDK-injected
Google copyright/Apache-2.0 bridge: retain notices and supply applicable licence
texts with distribution. System fonts are accessed through SDK APIs; no external
font/image asset is declared in `package.json`. The
[current SDK licence page](https://developer.repebble.com/legal/sdk-license/)
describes SDK/website licensing; verify conditions for the installed SDK version
and any redistributed components, not merely the game's own code.

## Before Clicking Submit

- Resolve the name and visual-similarity issues, with legal review where needed.
- Complete and prominently link the privacy policy and developer support contact.
- Audit inherited code and preserve required third-party notices/licence texts.
- Use only this app's own current screenshots and original store artwork.
- Describe it as an independent territory-capture arcade game, not an official port.
- State only verified compatibility: emery/Pebble Time 2; hardware testing remains open.
- Disclose that continuous in-game backlighting increases battery use.
- Review the actual account terms shown at submission; no blanket hobby exemption
  from IP, contract or applicable information duties is asserted here.
