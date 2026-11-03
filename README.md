# Throw Delay

Vocal-Delay mit Look-ahead: Echos entstehen nur an Phrasenenden. Lizenz: AGPLv3 (siehe `LICENSE`), JUCE läuft unter derselben Lizenz.

## Abhängigkeiten holen
Beide nach `libs/` klonen (nicht eingecheckt):

```bash
git clone --depth 1 --branch 9.0.5 https://github.com/juce-framework/JUCE.git libs/JUCE
```

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules https://github.com/free-audio/clap-juce-extensions.git libs/clap-juce-extensions
```

## Bauen (Windows)
In einer VS-Entwicklerumgebung:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

```bash
cmake --build build --target ThrowDelay_VST3 ThrowDelay_CLAP ThrowDelay_LV2 throw_tests
```

```bash
ctest --test-dir build --output-on-failure
```

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

Auf einem Intel-Mac prüft `auval` nur den x86_64-Teil. Der arm64-Teil braucht einen Apple-Silicon-Mac.

## Verhalten, das man kennen sollte
- Sensitivity: wie weit unter dem laufenden Phrasenpegel (dB) ein Signal noch als Gesang zählt.
- Min Gap ist im Look-ahead-Betrieb auf Look-ahead minus 100 ms begrenzt.
- Ist Min Gap + Throw Length länger als der Look-ahead, öffnet die Tür später und der Throw wird kürzer.
- Look-ahead und Live sind nicht automatisierbar, weil sie die Latenz ändern.
- Live-Modus: Das Phrasenende wird aus der jüngsten Vergangenheit ins Delay geschrieben. Ist Time kürzer als Min Gap + Throw Length + 10 ms, landet das erste Echo auf einem späteren Vielfachen von Time und bleibt so im Raster.
