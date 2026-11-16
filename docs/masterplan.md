# Quebble Masterplan

## Ziel

Quebble soll sich wie ein kompromissloses Territory-Capture-Arcade-Spiel auf der
Pebble Time 2 anfuehlen: schnelle Runden, klare Gefahr, keine unnötige
Menuearbeit, lesbare Grafik und eine Steuerung, die mit drei Seitentasten
funktioniert.

## Spielregeln

- Ziel pro Level: mindestens 75 Prozent des Feldes beanspruchen.
- Slow-Claims zaehlen doppelt und werden orange dargestellt.
- Fast-Claims bleiben blau/tuerkis und sind riskanter, aber schneller.
- Ab Level 3 gibt es zwei Innengegner. Ein Split beendet das Level sofort und
  erhoeht den Multiplikator bis maximal 9.
- Ein Split gibt keine Sofortpunkte; der Multiplikator wirkt auf spaetere
  Claims.
- Prozentpunkte ueber dem Ziel geben Bonuspunkte.

## Bewegung

- Startaufstellung: Spieler unten mittig mit Blick nach oben, zwei Sparx oben
  mittig in entgegengesetzte Richtungen.
- Vor dem Losfahren zaehlt das Spiel im Spielfeldstil von 3 auf 1 herunter;
  danach startet der Marker automatisch nach oben.
- Der Marker bewegt sich automatisch, sobald er gestartet wurde.
- Der Marker darf ausserhalb eines unfertigen Stix nur auf der aeusseren
  sicheren Kante fahren.
- Gefuellte Flaechen sind nicht befahrbar, ihre freiliegende Kante zaehlt aber
  als sichere Verbindung fuer neue Linien.
- Alte Linien bleiben sichtbar. Sie werden nicht mehr in Flaeche umgewandelt;
  befahrbar bleiben nur freiliegende sichere Kanten.
- Eine unfertige Linie darf nicht gekreuzt oder zurueckgefahren werden.
- Bleibt der Marker beim Zeichnen stehen oder steckt fest, startet die Fuse.
- Nach einem erfolgreichen Anschluss pausiert das Spiel etwa eine Sekunde,
  damit die neue Flaeche farblich/animatorisch als verbunden lesbar wird.

## PT2-Steuerung

- Obere Taste: links abbiegen.
- Untere Taste: rechts abbiegen.
- Select: stoppen oder weiterfahren.
- Select halten: Slow/Fast umschalten.
- Back: Start-/Kontrollscreen.
- Back halten: App verlassen.
- Touch: Tap fuer Stop/Go, Wisch links/rechts fuer relative Abbiegung.

## Gegner: Korrektur nach Videoreferenz

- Keine gewellte Kontur und kein starr als Ganzes gedrehtes Linienpaket.
- Sieben gerade Linien sind die Historie eines bewegten Segments. Alte Linien
  behalten ihre tatsaechlichen vorherigen Endpunkte statt neu berechnet zu werden.
- Translation, Drehung, Laenge und Geschwindigkeit variieren unabhaengig.
- Ortsbewegung und Drehstaerke besitzen getrennte, gleitende Intensitaetshuellkurven.
  Kleine Ausschlaege sind haeufiger als grosse; das Schrittlimit ist eine
  Sicherheitsgrenze, keine staendig auszuschoepfende Geschwindigkeit.
- Enge Buendel duerfen schnell wandern, breite Faecher lokal drehen. Die Laenge
  variiert unabhaengig davon mit Zielwerten von 18 bis 64 Pixeln.
- Drehung und Laengenwechsel haben eigene, unregelmaessige Timer. Kurze schnelle
  Schwenks wechseln mit ruhigen und ausdauernden Drehphasen.
- Der Drehpunkt wandert zwischen Mitte und beiden Linienenden. Dadurch entstehen
  asymmetrische Faecher statt einer staendigen symmetrischen Propellerbewegung.
  Volle Umdrehungen bleiben erlaubt.
  Ein zu grosser kombinierter Schritt wird verkleinert, nicht die Drehung verworfen.
- Keine Spielerposition und kein Zeichenzustand gehen in die Bewegungswahl ein.
- Unterschiedliche Start-Seeds verhindern identische Eroeffnungen jeder Runde.
- Maximal vier Pixel pro Achse und Endpunkt je Tick; Zwischenposen werden
  in Schritten unter einem Pixel auf Kollision geprueft.
- Darstellung und Trefferpruefung benutzen denselben Integer-Pixelrasterizer.
  Kein sichtbarer Pixel darf BORDER oder gefuellte Flaechen belegen.
- Jeder Linienpixel, auch zwischen Endpunkten und in alten Historienzeilen,
  loest bei Kontakt mit einer unfertigen Linie einen Lebensverlust aus.
- Wandvermeidung prueft voraus; ungueltige neue Posen werden nie uebernommen.
  Bei wenig Platz wird die neue Linie schrittweise kuerzer, nicht teleportiert.
  Zu langes Verweilen in einer Ecke aktiviert eine gepruefte Ausweichbewegung.
- Beim Fuellen bleiben auch von historischen Linien belegte Regionen frei.
- Randgegner koennen neuen Konturen folgen oder an ihren Einmuendungen
  geradeaus weiterfahren. Geradeaus hat Gewicht 2, Abbiegungen jeweils Gewicht 1;
  eine kurze Sperre verhindert wiederholtes Wuerfeln an derselben Verzweigung.
  Das ist ein aus der Aufnahme abgeleitetes Modell, keine bekannte Originalregel.
- Das Tempo bleibt im Normal- und Superzustand gleich: ein Rasterfeld je sechs
  Ticks, langsamer als der normale Spieler.
- Vier eigenstaendig prozedural gezeichnete, zusammenhaengende Funkenphasen
  mit stabilem Kern statt immer neuer Zufallspixel. Jede Phase dauert zwei Ticks,
  der Zyklus etwa 0,264 Sekunden. Ein weisser Kern und warme farbige Funkenarme
  setzen die spaetere Vorgabe kraeftiger Farben um.
  Vier Pixel Abstand zum Bildschirmrand halten alle Funkenpixel sichtbar.
- Normale Randgegner betreten keine unfertigen Linien.
- Super-Randgegner duerfen unfertige Linien betreten und setzen den Zeichner unter
  Druck.

## Verifikation der Bewegung

- Identischer Seed + unterschiedliche Spielerpositionen: identische Bewegung.
- 144.000 Frames auf leerem und konkavem Feld: jeden sichtbaren Pixel pruefen.
- Historienkontinuitaet, Endpunkt-Schrittgrenzen, Spielfeldabdeckung und
  maximale Stillstandsdauer als Regression pruefen.
- Rotation im freien Feld: mehr als eine volle Umdrehung in 60 Frames und
  ueber 45 Grad sichtbare Auffaecherung bei einer gehaltenen schnellen Drehphase.
- Intensitaetsverteilung: enge Buendel haeufiger als breite Faecher, kleine und
  grosse Laengen, schnelle enge Bewegung und lokale grosse Drehung pruefen.
- Das Erreichen der Schrittgrenze darf nicht der Normalfall sein.
- Randgegner: neue Konturen statt versteckter Innenlinien, keine Beschleunigung,
  normale/Super-Linienregeln und vollstaendig sichtbare Funkenpixel pruefen.
- Beide Verzweigungsentscheidungen sowie einen exakt wiederkehrenden
  vierphasigen Funkenzyklus in Normal- und Superzustand pruefen.
- Anschluss an vorhandene Flaechen einschliesslich der letzten durch TRAIL
  ersetzten EMPTY-Nachbarzelle im echten Bewegungspfad pruefen.
- emery-Build installieren, Screenshots und eine Bewegungssequenz inspizieren.

## Polish

- A separate English controls screen is available from the title.
- The compact HUD groups claim/target and score above lives, level and threat
  time, with contextual slow/stopped status instead of redundant movement icons.
- Vibration und Backlight geben klares Feedback fuer Claim, Treffer und
  Levelabschluss.

## Watch Readability Pass

- English throughout the watch UI; separate controls view instead of a crowded title.
- Original animated pixel wordmark, immediately skippable with Select or tap.
- Three-pixel cells, 64 by 60 cells; fast/slow movement every 4/7 ticks and
  constant-speed edge runners every 6 ticks compensate for the larger scale.
- Two-pixel monster strokes use exactly the same brush for rendering and collision.
  Length targets are 18-64 pixels. Bright cyan/yellow/green/magenta strokes and
  white/yellow/orange/red edge sparks replace hard-to-see thin/dark details.
- A colourful two-row HUD aligns claim/target and score above three fixed groups:
  hearts, level/contextual status and a threat clock. A thin progress track ties
  the display together. Default speed, movement icon and x1 are omitted.
- A buried marker is projected to the nearest reachable exposed boundary after
  a claim, using a non-recursive queue search. Old lines remain visible.
- Backlight is held during the round and released in menus, on focus loss and exit.

## Transition And Reference Pass

- Anchor the clock icon to the measured number width, not a separate fixed slot.
- Remove the black runner halo. Use original procedural red/yellow body morphs
  with eight unequal-duration phases and a white core; super bodies are cyan/blue.
- Share the pixel-assembly language across countdown digits, HIT, CLEAR and results.
  Center status information and remove the unrelated red corner decorations.
- Finish the final HIT before results, reset the results animation clock, keep
  results lit and wait for Retry/Menu. Focus loss and exit still release lighting.
- Keep a fixed 75% target. Implement the documented first four interior-enemy
  patterns and timed reinforcement before physical line pursuit. Runner speed
  stays capped. Later watch-specific thresholds are explicitly documented in
  `reference-derivation.md` rather than presented as recovered original constants.
- Verify native emulator frames of transitions, results persistence and the
  normal/slow/stopped/one-second/super timer variants; add host regressions for
  difficulty, reinforcement, pursuit, final hit/retry and pixel-word bounds.
