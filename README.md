# Throw Delay

[![build](https://github.com/Beatsex/throwdelay/actions/workflows/build.yml/badge.svg)](https://github.com/Beatsex/throwdelay/actions/workflows/build.yml) [![release](https://img.shields.io/github/v/release/Beatsex/throwdelay?include_prereleases)](https://github.com/Beatsex/throwdelay/releases/latest) [![license: AGPL-3.0](https://img.shields.io/badge/license-AGPL--3.0-blue)](LICENSE) ![VST3 · CLAP · LV2 · AU](https://img.shields.io/badge/format-VST3%20%C2%B7%20CLAP%20%C2%B7%20LV2%20%C2%B7%20AU-pink)

**English** · [Deutsch](README.de.md)

A delay for vocals that throws by itself, by **beatsexuell**.
VST3, CLAP, LV2 and a standalone app on Windows, AU and VST3 on macOS.

> It waits for you to stop singing.

Mixing vocals, you keep doing the same thing by hand: a delay on an aux, and the send pulled up at the exact
moment a line ends. The last word gets the throw, the rest stays dry. Throw Delay does that for you. A
look-ahead hears the signal before it plays, finds the end of a phrase in the level envelope, and opens the
delay input only there.

No reverb, no modulation, no preset browser — a delay, and the detection that triggers it.

## Features

- **Phrase detection with look-ahead** — 0.5 s, 1 s or 2 s of hindsight, so the throw is already open when the last word arrives
- **Live mode** — no latency at all, for tracking; detects more coarsely in exchange
- **Sensitivity**, **Min Gap** and **Throw Length** — how far the level must drop, how long the gap must be, how long the door stays open
- **Between** — how much signal gets through outside the throws, from none to a plain delay
- **Time or Sync** — free in milliseconds, or locked to the host tempo from 1/32 to 1/1 including triplets and dotted values
- **Feedback** and **Mix**
- **Phrase-end meter** — the last seven seconds of detections, so you can see what it heard
- Values carry their units, in the plugin and in the host alike

## Install

Download the latest build from [Releases](https://github.com/Beatsex/throwdelay/releases).

| System | How |
|---|---|
| Windows 10 / 11 (64-bit) | Run `ThrowDelay-…-Windows.exe`. It is not code-signed yet: if SmartScreen appears, click *More info* → *Run anyway*. |
| macOS | Copy `Throw Delay.vst3` to `~/Library/Audio/Plug-Ins/VST3` and `Throw Delay.component` to `~/Library/Audio/Plug-Ins/Components`. |

Then restart your DAW (or rescan plugins) and insert it as an audio effect: *beatsexuell → Throw Delay*.

## Quick start

1. Put Throw Delay on a vocal track, dry and unprocessed.
2. Leave **Look-ahead** at 1 s and play the track. The meter at the bottom marks every phrase end it finds.
3. Too few marks: raise **Sensitivity**. Marks in the middle of a line: raise **Min Gap**.
4. Set **Time** or switch on **Sync** and pick a division, then dial **Mix** to taste.
5. Recording a take? Switch on **Live** — no latency, slightly coarser detection.

After changing the look-ahead stage, press stop and play once so the host picks up the new latency.

## Build from source

Requirements: CMake, a C++20 compiler, Ninja. JUCE and `clap-juce-extensions` are submodules.

```bash
git clone --recurse-submodules https://github.com/Beatsex/throwdelay.git
```

Windows, in a VS developer environment (`VsDevCmd.bat -arch=x64`):

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

```bash
cmake --build build
```

```bash
ctest --test-dir build --output-on-failure
```

The plugin ends up in `build/ThrowDelay_artefacts/Release/`. `COPY_PLUGIN_AFTER_BUILD` is off, so nothing is
installed for you.

The Windows installer needs Inno Setup 6:

```bash
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\ThrowDelay.iss
```

macOS (AU + VST3 as a universal binary):

```bash
cmake -S . -B build -G Xcode
```

```bash
cmake --build build --config Release --target ThrowDelay_AU ThrowDelay_VST3
```

On an Intel Mac `auval -v aufx Thrw Btsx` only checks the x86_64 slice; the arm64 slice needs Apple Silicon.

## How it is put together

- `src/dsp/` — `PhraseDetector` and `ThrowEngine`, plain C++ without JUCE, tested from the CLI in `tests/`.
- `src/plugin/` — the JUCE adapter and the interface. No format-specific DSP branches.
- `installer/` — the Inno Setup script and the manual that ships with it.

`processBlock` does not allocate, take locks or touch files. The reported latency depends only on the
look-ahead stage and the Live switch, never on the signal.

## Behaviour worth knowing

- Sensitivity is how far below the running phrase level (in dB) a signal still counts as singing.
- With look-ahead, Min Gap is capped at the look-ahead minus 100 ms.
- If Min Gap + Throw Length is longer than the look-ahead, the door opens later and the throw gets shorter.
- Look-ahead and Live are not automatable, because they change the latency.
- In Live mode the phrase end is written into the delay from the recent past. If Time is shorter than
  Min Gap + Throw Length + 10 ms, the first echo lands on a later multiple of Time and stays in the grid.

## Feedback

Open an [issue](https://github.com/Beatsex/throwdelay/issues) with your settings, what you sang, where the
throws landed, your DAW and sample rate. For a crash: which DAW and what you clicked last.

## Licence

AGPLv3, see [LICENSE](LICENSE). JUCE is used under the same licence.
