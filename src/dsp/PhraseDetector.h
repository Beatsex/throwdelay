#pragma once

#include <array>
#include <cstdint>

// Finds the end of vocal phrases from a mono stream, one sample at a time.
// Rules only: level relative to the running phrase level, plus a periodicity
// check, so breaths and room noise in a pause count as silence.
class PhraseDetector
{
public:
    struct Phrase
    {
        int64_t start = 0, end = 0;
    };

    void prepare (double sampleRate);
    void reset();

    // sensitivityDb: how far below the phrase level (positive dB) a signal still counts as singing.
    void setParameters (float sensitivityDb, float minGapMs);

    // Returns true when a phrase end was just confirmed; read it from lastPhrase().
    bool process (float sample);

    const Phrase& lastPhrase() const { return phrase; }
    int64_t position() const { return n; }
    float envelope() const { return env; }
    float thresholdLevel() const { return std::max (ref * ratio, floorLevel); }
    bool isVoiced() const { return inPhrase; }

private:
    void updatePeriodicity();

    static constexpr int frameLength = 240, minLag = 20, maxLag = 120, hop = 80; // at ~8 kHz

    double sr = 44100.0;
    float hpCoeff = 0, lpCoeff = 0, attack = 0, release = 0, refDecay = 0;
    float hpState = 0, lpState = 0, env = 0, ref = 0;
    float ratio = 0.125f, floorLevel = 0.001f;

    std::array<float, frameLength + maxLag> frame {};
    int decimation = 1, decimationCount = 0, hopCount = 0;
    bool periodic = false, above = false, inPhrase = false;

    int64_t n = 0, voiceRun = 0, lastVoiced = 0, phraseStart = 0;
    int64_t minGap = 0, confirm = 0, minPhrase = 0;
    Phrase phrase;
};
