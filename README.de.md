# Throw Delay

[![build](https://github.com/Beatsex/throwdelay/actions/workflows/build.yml/badge.svg)](https://github.com/Beatsex/throwdelay/actions/workflows/build.yml) [![release](https://img.shields.io/github/v/release/Beatsex/throwdelay?include_prereleases)](https://github.com/Beatsex/throwdelay/releases/latest) [![license: AGPL-3.0](https://img.shields.io/badge/license-AGPL--3.0-blue)](LICENSE) ![VST3 · CLAP · LV2 · AU](https://img.shields.io/badge/format-VST3%20%C2%B7%20CLAP%20%C2%B7%20LV2%20%C2%B7%20AU-pink)

[English](README.md) · **Deutsch**

Ein Delay für Gesang, das sich selbst auslöst, von **beatsexuell**.
VST3, CLAP, LV2 und eine Standalone-App unter Windows, AU und VST3 unter macOS.

> Es wartet, bis du aufhörst zu singen.

Beim Mischen von Vocals macht man immer wieder dasselbe von Hand: ein Delay auf einen Aux legen und den Send
genau dann aufziehen, wenn eine Zeile zu Ende ist. Das letzte Wort bekommt den Wurf, der Rest bleibt trocken.
Throw Delay nimmt einem das ab. Ein Look-ahead hört dem Signal voraus, erkennt am Pegelverlauf, wo eine
Phrase endet, und öffnet nur dort den Delay-Eingang.

Kein Reverb, keine Modulation, kein Preset-Browser — ein Delay und die Erkennung, die es auslöst.

## Was drin ist

- **Phrasenerkennung mit Look-ahead** — 0,5 s, 1 s oder 2 s Vorlauf, damit der Wurf schon offen ist, wenn das letzte Wort kommt
- **Live-Modus** — ganz ohne Latenz fürs Einsingen, erkennt dafür gröber
- **Sensitivity**, **Min Gap** und **Throw Length** — wie weit der Pegel fallen muss, wie lang die Pause sein muss, wie lange die Tür offen bleibt
- **Between** — wie viel Signal außerhalb der Würfe durchkommt, von gar nichts bis zum normalen Delay
- **Time oder Sync** — frei in Millisekunden oder ans Songtempo gebunden, von 1/32 bis 1/1 mit Triolen und punktierten Werten
- **Feedback** und **Mix**
- **Phrasenenden-Meter** — die letzten sieben Sekunden an Erkennungen, damit sichtbar wird, was es gehört hat
- Werte tragen ihre Einheit, im Plugin wie im Host

## Installieren

Die neueste Fassung liegt unter [Releases](https://github.com/Beatsex/throwdelay/releases).

| System | Wie |
|---|---|
| Windows 10 / 11 (64 Bit) | `ThrowDelay-…-Windows.exe` ausführen. Noch nicht signiert: Wenn SmartScreen kommt, *Weitere Informationen* → *Trotzdem ausführen*. |
| macOS | `Throw Delay.vst3` nach `~/Library/Audio/Plug-Ins/VST3` und `Throw Delay.component` nach `~/Library/Audio/Plug-Ins/Components` kopieren. |

Danach die DAW neu starten oder die Plug-ins neu scannen lassen, dann liegt es als Audioeffekt unter
*beatsexuell → Throw Delay*.

## Schnellstart

1. Throw Delay auf eine trockene Gesangsspur legen.
2. **Look-ahead** auf 1 s lassen und abspielen. Das Meter unten markiert jedes erkannte Phrasenende.
3. Zu wenige Marken: **Sensitivity** hoch. Marken mitten in der Zeile: **Min Gap** hoch.
4. **Time** einstellen oder **Sync** einschalten und einen Notenwert wählen, dann **Mix** nach Geschmack.
5. Beim Aufnehmen **Live** einschalten — keine Latenz, etwas gröbere Erkennung.

Nach dem Umschalten der Look-ahead-Stufe einmal Stop und Play drücken, damit die DAW die neue Latenz übernimmt.

## Selbst bauen

Gebraucht werden CMake, ein C++20-Compiler und Ninja. JUCE und `clap-juce-extensions` sind Submodule.

```bash
git clone --recurse-submodules https://github.com/Beatsex/throwdelay.git
```

Windows, in einer VS-Entwicklerumgebung (`VsDevCmd.bat -arch=x64`):

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

```bash
cmake --build build
```

```bash
ctest --test-dir build --output-on-failure
```

Die Artefakte liegen danach unter `build/ThrowDelay_artefacts/Release/`. `COPY_PLUGIN_AFTER_BUILD` ist aus, es
wird also nichts automatisch installiert.

Der Windows-Installer braucht Inno Setup 6:

```bash
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\ThrowDelay.iss
```

macOS (AU + VST3 als Universal Binary):

```bash
cmake -S . -B build -G Xcode
```

```bash
cmake --build build --config Release --target ThrowDelay_AU ThrowDelay_VST3
```

Auf einem Intel-Mac prüft `auval -v aufx Thrw Btsx` nur den x86_64-Teil, der arm64-Teil braucht einen
Apple-Silicon-Mac.

## Aufbau

- `src/dsp/` — `PhraseDetector` und `ThrowEngine`, reines C++ ohne JUCE, über `tests/` per CLI getestet.
- `src/plugin/` — JUCE-Adapter und Oberfläche. Keine formatspezifischen DSP-Zweige.
- `installer/` — Inno-Setup-Skript und die Anleitung, die mitinstalliert wird.

`processBlock` allokiert nicht, nimmt keine Locks und fasst keine Dateien an. Die gemeldete Latenz hängt
allein von der Look-ahead-Stufe und vom Live-Modus ab, nie vom Signal.

## Verhalten, das man kennen sollte

- Sensitivity: wie weit unter dem laufenden Phrasenpegel (dB) ein Signal noch als Gesang zählt.
- Min Gap ist im Look-ahead-Betrieb auf Look-ahead minus 100 ms begrenzt.
- Ist Min Gap + Throw Length länger als der Look-ahead, öffnet die Tür später und der Throw wird kürzer.
- Look-ahead und Live sind nicht automatisierbar, weil sie die Latenz ändern.
- Live-Modus: Das Phrasenende wird aus der jüngsten Vergangenheit ins Delay geschrieben. Ist Time kürzer als
  Min Gap + Throw Length + 10 ms, landet das erste Echo auf einem späteren Vielfachen von Time und bleibt so
  im Raster.

## Rückmeldung

Mach ein [Issue](https://github.com/Beatsex/throwdelay/issues) auf: deine Einstellungen, was du gesungen hast,
wo die Würfe gelandet sind, DAW und Samplerate. Bei einem Absturz: welche DAW und was du zuletzt geklickt hast.

## Lizenz

AGPLv3, siehe [LICENSE](LICENSE). JUCE läuft unter derselben Lizenz.
