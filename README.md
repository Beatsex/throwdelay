# Throw Delay

Ein Delay für Gesang, das sich selbst auslöst.

Beim Mischen von Vocals macht man immer wieder dasselbe von Hand: ein Delay auf
einen Aux legen und den Send genau dann aufziehen, wenn eine Zeile zu Ende ist.
Das letzte Wort bekommt den Wurf, der Rest bleibt trocken. Throw Delay nimmt
einem das ab. Ein Look-ahead hört dem Signal voraus, erkennt am Pegelverlauf,
wo eine Phrase endet, und öffnet nur dort den Delay-Eingang.

Kein Reverb, keine Modulation, kein Preset-Browser — ein Delay und die
Erkennung, die es auslöst.

Lizenz: AGPLv3 (siehe `LICENSE`), JUCE läuft unter derselben Lizenz.

## Formate

VST3, CLAP, LV2 und eine Standalone-App unter Windows; AU und VST3 als
Universal Binary unter macOS. Kein AAX, kein VST2.

## Holen

JUCE und `clap-juce-extensions` liegen als Submodule unter `libs/`:

```bash
git clone --recurse-submodules https://github.com/Beatsex/throwdelay.git
```

Bei einem bereits geklonten Repo:

```bash
git submodule update --init --recursive
```

## Bauen (Windows)

In einer VS-Entwicklerumgebung (`VsDevCmd.bat -arch=x64`):

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

```bash
cmake --build build
```

```bash
ctest --test-dir build --output-on-failure
```

Die Artefakte liegen danach unter `build/ThrowDelay_artefacts/Release/`.
`COPY_PLUGIN_AFTER_BUILD` ist aus, das Plugin wird also nicht automatisch
installiert.

## Installer (Windows)

Braucht Inno Setup 6. Erzeugt eine Setup-Datei mit allen vier Formaten, der
HTML-Anleitung aus `installer/readme.html` und der Lizenz:

```bash
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\ThrowDelay.iss
```

Das Ergebnis landet unter `build/installer/`.

## macOS (AU + VST3 als Universal Binary)

```bash
cmake -S . -B build -G Xcode
```

```bash
cmake --build build --config Release --target ThrowDelay_AU ThrowDelay_VST3
```

Prüfung nach Installation nach `~/Library/Audio/Plug-Ins/Components`:

```bash
auval -v aufx Thrw Btsx
```

Auf einem Intel-Mac prüft `auval` nur den x86_64-Teil. Der arm64-Teil braucht
einen Apple-Silicon-Mac.

## Aufbau

- `src/dsp/` — `PhraseDetector` und `ThrowEngine`, reines C++ ohne JUCE, über
  `tests/` per CLI getestet.
- `src/plugin/` — JUCE-Adapter und Oberfläche. Keine formatspezifischen
  DSP-Zweige.
- `installer/` — Inno-Setup-Skript und die Anleitung, die mitinstalliert wird.

`processBlock` allokiert nicht, nimmt keine Locks und fasst keine Dateien an.
Die gemeldete Latenz hängt allein von der Look-ahead-Stufe und vom Live-Modus
ab, nie vom Signal.

## Verhalten, das man kennen sollte

- Sensitivity: wie weit unter dem laufenden Phrasenpegel (dB) ein Signal noch als Gesang zählt.
- Min Gap ist im Look-ahead-Betrieb auf Look-ahead minus 100 ms begrenzt.
- Ist Min Gap + Throw Length länger als der Look-ahead, öffnet die Tür später und der Throw wird kürzer.
- Look-ahead und Live sind nicht automatisierbar, weil sie die Latenz ändern.
- Live-Modus: Das Phrasenende wird aus der jüngsten Vergangenheit ins Delay geschrieben. Ist Time kürzer als Min Gap + Throw Length + 10 ms, landet das erste Echo auf einem späteren Vielfachen von Time und bleibt so im Raster.
- Nach dem Umschalten der Look-ahead-Stufe einmal Stop und Play drücken, damit die DAW die neue Latenz übernimmt.
