# Quebble Screenshot Gallery

Eight native emery PNG captures, each exactly 200x228 pixels. No watch frames,
compositing, resizing or added text. These are emulator captures, not photographs
of the physical watch.

| File | Scene |
| --- | --- |
| emery_01_title.png | Title and native pixel wordmark |
| emery_02_guide.png | First guide page: the game objective |
| emery_03_settings.png | Sound settings, toggles and app volume |
| emery_04_gameplay.png | Unfinished line and moving interior enemy |
| emery_05_territory.png | Captured land, historic seam and stopped touch fork |
| emery_06_hit.png | Full-field HIT transition |
| emery_07_game_over.png | Game Over and Retry/Menu actions |
| emery_08_level_2.png | Level 2 gameplay, identified in the HUD |

Menus are real app views. Gameplay/transition capture states are staged through
debugger data writes. HIT was held long enough to capture its settled lettering.
Level 2 is initialized by the game's native level-advance code. No debugger calls
into app or firmware functions were used, and the PBW was not modified.

The store currently accepts at most five screenshots per platform. The separate
screenshots/ directory in the Publish Kit contains a suggested five-image subset:
title, live drawing, claimed territory, first guide page and Level 2. Swap in any
gallery image if preferred; do not upload all eight at once.
