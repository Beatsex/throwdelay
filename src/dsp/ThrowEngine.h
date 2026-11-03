#pragma once

#include "PhraseDetector.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <vector>

// Stereo delay whose input door opens around detected phrase ends.
// Dry and the door tap share one history buffer, so dry stays sample exact.
class ThrowEngine
{
public:
    // History also serves the live door tap: Min Gap + Throw Length + 10 ms, at most 3.01 s.
    static constexpr double maxHistorySeconds = 3.1, maxDelaySeconds = 8.0;

    struct Params
    {
        float delayMs = 375, feedback = 0.35f, mix = 0.3f;
        float sensitivityDb = 18, minGapMs = 250, throwMs = 400, between = 0;
        int lookaheadSamples = 0; // 0 = live mode
    };

    void prepare (double sampleRate); // allocates, never call from the audio thread
    void reset();

    // Takes stereo in place. bypassed keeps the dry delay so host compensation still lines up.
    void process (float* left, float* right, int numSamples, const Params& params, bool bypassed = false);

    uint32_t throwCount() const { return throws.load (std::memory_order_relaxed); }
    float doorLevel() const { return lastDoor.load (std::memory_order_relaxed); }
    float inputLevel() const { return lastInput.load (std::memory_order_relaxed); }
    float echoLevel() const { return lastEcho.load (std::memory_order_relaxed); }
    float detectorEnv() const { return lastEnv.load (std::memory_order_relaxed); }
    float detectorThreshold() const { return lastThreshold.load (std::memory_order_relaxed); }
    bool inPhrase() const { return phraseActive.load (std::memory_order_relaxed); }

private:
    struct Ramp
    {
        float value = 0, step = 0;
        void set (float v) { value = v; }
        float next (float target)
        {
            if (value < target) value = value + step < target ? value + step : target;
            else if (value > target) value = value - step > target ? value - step : target;
            return value;
        }
    };

    void updateTaps (const Params&);
    void schedule (const PhraseDetector::Phrase&, const Params&);

    double sr = 44100.0;
    PhraseDetector detector;

    std::array<std::vector<float>, 2> history, delay;
    int historySize = 0, delaySize = 0, writeIndex = 0, delayWrite = 0;
    int64_t t = 0;

    int dryTap = 0, doorTap = 0, pendingDryTap = 0, doorWriteOffset = 0;
    int headA = 1, headB = 1, crossfadeLength = 1, crossfadeCount = 0;
    bool targetSet = false;

    struct Window { int64_t open, close; };
    static constexpr int maxWindows = 32;
    std::array<Window, maxWindows> windows {};
    int windowRead = 0, windowCount = 0;
    bool doorWasOpen = false;

    Ramp doorGain, switchGain, feedbackGain, betweenGain, dryGain, wetGain;
    float doorCloseStep = 0, doorOpenStep = 0;
    std::atomic<uint32_t> throws { 0 };
    std::atomic<float> lastDoor { 0.0f }, lastInput { 0.0f }, lastEcho { 0.0f }, lastEnv { 0.0f }, lastThreshold { 0.001f };
    std::atomic<bool> phraseActive { false };
};
