# Quebble 1.7.0 - Publish Kit

Dieses Paket bereitet den Store-Upload vor. Es veroeffentlicht nichts im Pebble
Appstore. Quellcode und Downloads sind separat auf GitHub verfuegbar.

## Dateien Fuer Den Store

| Feld | Datei / Wert |
| --- | --- |
| App-Datei | `quebble-1.7.0.pbw` |
| Name | Inhalt von `copy/name.txt` |
| Beschreibung | Inhalt von `copy/description.txt` |
| Kategorie | Games |
| Preis | Kostenlos |
| Release Notes | Inhalt von `copy/release-notes.txt` |
| Source Code URL | Inhalt von `copy/source-url.txt` |
| Small Icon | `images/icon-80.png` |
| Large Icon | `images/icon-144.png` |
| Emery Banner | `images/banner-720x320.png` |
| Emery Screenshots | Die fuenf Dateien in `screenshots/`, in Nummernreihenfolge |

`images/icon-512.png` und die transparenten/schwarzen Logos sind zusaetzliche
Grafiken fuer eigene Ankuendigungen. Das 25-Pixel-Launcher-Icon ist bereits
in der PBW enthalten. Die groessere Banner-Quelldatei liegt unter `source-art/`.
Alle Store-Texte und die App sind auf Englisch.

Unter `gallery/` liegen alle acht Motive: Titel, erste Guide-Seite, Einstellungen,
zwei Spielszenen, HIT, Game Over und Level 2. `screenshots/` enthaelt die fuenf
vorgeschlagenen Upload-Bilder (Titel, Zeichnen, Gebiet, Guide, Level 2).
Die Galerie dient zur Auswahl; der Store erlaubt nicht alle acht gleichzeitig.

## Hochladen

1. Das eigene Konto im Pebble Developer Dashboard oeffnen.
2. Neue App anlegen oder eine bestehende Quebble-App mit derselben UUID oeffnen.
3. PBW hochladen, Name/Beschreibung/Kategorie uebernehmen, Icons setzen.
4. Fuer emery die fuenf Screenshots und das Banner hochladen.
5. Release Notes einsetzen und Sichtbarkeit bewusst waehlen.
6. Vor dem abschliessenden Submit die folgenden Punkte pruefen.

Die UUID bleibt `00d6400a-2f2c-4096-95f3-4db1cdc20f8b`. Version `1.7.0`
muss im eigenen Store-Eintrag noch unbenutzt sein. Website und Source Code URL
sind optional; fuer Source Code URL ist jetzt der echte GitHub-Link enthalten.
Es ist keine separate
Android- oder iOS-Companion-App erforderlich. Der normale Pebble-Begleiter
der Uhr ist davon unabhaengig.

## Noch Vom Herausgeber Zu Erledigen

- `copy/privacy-policy.html` oeffentlich hosten und die echte URL im Listing
  deutlich verlinken. Die Datei ist fertig formuliert; eine Hosting-Adresse
  oder ein Supportkontakt wurde nicht erfunden. Den Kontakt im Store setzen.
- Die bereits dokumentierten Namens-/Aehnlichkeitsfragen vor Veroeffentlichung
  klaeren. Dieses Grafik-/Build-Paket ist keine rechtliche Freigabe. Die App
  bleibt auf Wunsch Quebble. Details stehen in `legal/store-readiness.md`.
- Auf der eigenen Uhr den kuehlen Blauton und das neue Launcher-Icon pruefen.
  Hier wurden Emulator und Host-Tests verwendet, keine physische Uhr.
- Die beim Upload angezeigten Kontobedingungen lesen. Submit kann sofort
  veroeffentlichen; es wird hier kein automatischer Publish-Befehl ausgefuehrt.

## Masse Und Pruefung

Store-Icons: 80x80 und 144x144. Banner: 720x320. Emery-Screenshots: exakt
200x228, ohne Uhrenrahmen. Maximal fuenf pro Plattform. Die Dateien werden
beim Packen auf Masse, Dateigroesse, PBW-Metadaten und SHA-256 geprueft.
`manifest.json` enthaelt die Dateipruefsummen und Abmessungen.

Offizielle Vorgaben, geprueft am 16.11.2026:
https://developer.repebble.com/guides/appstore-publishing/preparing-a-submission/
https://developer.repebble.com/guides/appstore-publishing/publishing-an-app/

Datenschutz-/Rechtebedingungen:
https://developer.repebble.com/legal/distribution/
https://developer.repebble.com/legal/program-policies/

Musik-/Effektquellen und Lizenztexte liegen vollstaendig unter `legal/`.
