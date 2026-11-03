# Project
Throw Delay: Vocal-Phrasen-Delay als Plugin. Look-ahead erkennt Phrasenenden und öffnet nur dort den Delay-Eingang (automatischer Delay Throw). Open Source unter AGPLv3.

# Stack
- C++20, JUCE (`libs/JUCE`), CMake + Ninja aus VS Build Tools 2022
- Formate: VST3, CLAP, LV2, Standalone (Windows), AU + VST3 Universal (macOS). Kein AAX, kein VST2.
- Keine weiteren Abhängigkeiten, keine neuronalen Netze.

# Commands
Aus einer VS-Entwicklerumgebung (`VsDevCmd.bat -arch=x64`, liegt unter `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\`):
- Configure: `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release`
- Build: `cmake --build build`
- Test: `ctest --test-dir build --output-on-failure`
- Installer: `"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\ThrowDelay.iss` → `build/installer/ThrowDelay-<version>-Windows.exe`

# Conventions
- `src/dsp/`: reines C++ ohne JUCE (`PhraseDetector`, `ThrowEngine`), per CLI getestet in `tests/`.
- `src/plugin/`: JUCE-Adapter und UI. Keine formatspezifischen DSP-Zweige.
- `processBlock`: keine Allokationen, Locks, Dateizugriffe. `setLatencySamples` nur vom Message-Thread (nimmt intern einen Lock).
- Latenz hängt nur von Look-ahead-Stufe und Live-Modus ab, nie vom Signal.
- Keine Zusatzfunktionen (Reverb, Modulation, Preset-Browser).
- Parameter-Anzeige: Einheit und Nachkommastellen kommen aus `makeLayout` (`stringFromValueFunction`), die Slider-Textbox spiegelt genau den Parametertext. Formatierung nie im Editor doppeln.
- UI-Look in `ThrowLookAndFeel` (`src/plugin/PluginEditor.h`): Akzent `#F2B8C6`, Elfenbein `#F2F1ED`, Tinte `#111`, 1,5px-Haarlinien, Mono-Labels in Großbuchstaben, Balken statt Drehregler.

# Status & open items
- Zielumgebung: Live 12.3 Suite auf Windows; Intel-Mac für AU vorhanden (arm64-Slice dort nicht mit auval prüfbar).
- Ableton-Abnahme ist durch (03.11.2026): Erkennung, UI, Parameter, Throws an Phrasenenden, Latenzkompensation (Null-Test bit-identisch), State über Speichern/Laden, kein Knacken, kein Dropout — alles OK.
- Das VST3 liegt direkt unter `C:\Program Files\Common Files\VST3\Throw Delay.vst3`, ohne Hersteller-Unterordner; `COPY_PLUGIN_AFTER_BUILD` ist FALSE, also von Hand kopieren. Live hält die DLL im laufenden Betrieb — vor dem Installieren schließen.
- Live braucht einen manuellen Plug-in-Rescan, wenn das Plugin nach dem Start installiert wurde.
- Icon: `resources/icon.png` (gezeichnet), `resources/icon.ico` für Inno Setup; in CMake als `ICON_BIG`.
- Installer-Skript: `installer/ThrowDelay.iss` mit `installer/readme.html` als Anleitung. Version steht fest auf 1.0.0, Exe ist nicht signiert (SmartScreen warnt).
- JUCE 9.0.5 hat kein natives CLAP, daher `clap-juce-extensions` (MIT).
- Offen: `clap-validator` meldet 3 Fehler bei state-reproducibility (Werte vor dem Speichern, per process gesetzt).
- Offen: Inter und JetBrains Mono sind nicht eingebettet, die UI nutzt System-Sans und -Mono. Echte Fonts bräuchten `juce_add_binary_data`.
- pluginval (Windows) läuft nicht im Vordergrund: per `Start-Process -Wait` mit Log-Umleitung starten.
- Laufwerk C: war am 03.11.2026 voll (6,5 MB frei) und hat einen Build-Schritt gekippt; Projekt und Artefakte liegen auf H:.
