#include "ThrowEngine.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr double doorTailSeconds = 0.06; // keeps the last consonant, detector end is voiced-only

// Linear up to 0.8, then bends towards 1.0. Bounds the feedback loop at any setting.
float limit (float x)
{
    if (! std::isfinite (x))
        return 0;
    const float a = std::abs (x);
    if (a <= 0.8f)
        return x;
    const float bent = 0.8f + 0.2f * std::tanh ((a - 0.8f) / 0.2f);
    return x < 0 ? -bent : bent;
}
} // namespace

void ThrowEngine::prepare (double sampleRate)
{
    sr = sampleRate;
    detector.prepare (sr);
    historySize = (int) (maxHistorySeconds * sr) + 64;
    delaySize = (int) (maxDelaySeconds * sr) + 64;
    for (int ch = 0; ch < 2; ++ch)
    {
        history[(size_t) ch].assign ((size_t) historySize, 0.0f);
        delay[(size_t) ch].assign ((size_t) delaySize, 0.0f);
    }

    const auto rampStep = [this] (double ms) { return (float) (1000.0 / (ms * sr)); };
    doorOpenStep = rampStep (8.0);
    doorCloseStep = rampStep (50.0);
    switchGain.step = rampStep (5.0);
    feedbackGain.step = betweenGain.step = dryGain.step = wetGain.step = rampStep (20.0);
    crossfadeLength = std::max (1, (int) (0.03 * sr));
    reset();
}

void ThrowEngine::reset()
{
    detector.reset();
    for (int ch = 0; ch < 2; ++ch)
    {
        std::fill (history[(size_t) ch].begin(), history[(size_t) ch].end(), 0.0f);
        std::fill (delay[(size_t) ch].begin(), delay[(size_t) ch].end(), 0.0f);
    }
    writeIndex = delayWrite = 0;
    t = 0;
    windowRead = windowCount = 0;
    doorWasOpen = false;
    crossfadeCount = 0;
    targetSet = false;
    doorGain.set (0);
    switchGain.set (1);
    feedbackGain.set (0);
    betweenGain.set (0);
    dryGain.set (1);
    wetGain.set (0);
    dryTap = pendingDryTap = doorTap = doorWriteOffset = 0;
    lastDoor.store (0.0f, std::memory_order_relaxed);
    lastInput.store (0.0f, std::memory_order_relaxed);
    lastEcho.store (0.0f, std::memory_order_relaxed);
    lastEnv.store (0.0f, std::memory_order_relaxed);
    lastThreshold.store (0.001f, std::memory_order_relaxed);
    phraseActive.store (false, std::memory_order_relaxed);
}

void ThrowEngine::updateTaps (const Params& p)
{
    pendingDryTap = std::clamp (p.lookaheadSamples, 0, historySize - 1);

    // Live mode has nothing buffered ahead, so the door taps the recent past instead.
    const bool live = p.lookaheadSamples == 0;
    const int wanted = live ? (int) ((p.minGapMs + p.throwMs) * 0.001 * sr) + (int) (0.01 * sr) : pendingDryTap;
    if (doorGain.value <= 0 && windowCount == 0)
        doorTap = std::clamp (wanted, 0, historySize - 1);
}

void ThrowEngine::schedule (const PhraseDetector::Phrase& phrase, const Params& p)
{
    if (windowCount == maxWindows)
        return;
    const auto throwLength = (int64_t) (p.throwMs * 0.001 * sr);
    const Window w { std::max (phrase.start, phrase.end - throwLength), phrase.end + (int64_t) (doorTailSeconds * sr) };
    windows[(size_t) ((windowRead + windowCount++) % maxWindows)] = w;
}

void ThrowEngine::process (float* left, float* right, int numSamples, const Params& p, bool bypassed)
{
    updateTaps (p);

    // Look-ahead caps Min Gap: the door must still be ahead of playback when the gap is confirmed.
    const float maxGapMs = p.lookaheadSamples > 0
                               ? std::max (20.0f, (float) (p.lookaheadSamples / sr * 1000.0) - 100.0f)
                               : p.minGapMs;
    detector.setParameters (p.sensitivityDb, std::min (p.minGapMs, maxGapMs));

    const int targetHead = std::clamp ((int) (p.delayMs * 0.001 * sr), 1, delaySize - 2);
    if (! targetSet)
    {
        headA = headB = targetHead;
        dryTap = pendingDryTap; // first block after a reset: nothing to fade
        targetSet = true;
    }

    const float mix = bypassed ? 0.0f : std::clamp (p.mix, 0.0f, 1.0f);
    const float dryTarget = std::min (1.0f, 2.0f * (1.0f - mix)), wetTarget = std::min (1.0f, 2.0f * mix);
    const float feedbackTarget = std::clamp (p.feedback, 0.0f, 0.95f);
    const float betweenTarget = std::clamp (p.between, 0.0f, 1.0f);

    auto& histL = history[0]; auto& histR = history[1];
    auto& delL = delay[0]; auto& delR = delay[1];

    const auto readHistory = [&] (const std::vector<float>& h, int tap) {
        int i = writeIndex - tap;
        if (i < 0) i += historySize;
        return h[(size_t) i];
    };
    const auto readDelay = [&] (const std::vector<float>& d, int head) {
        int i = delayWrite - head;
        if (i < 0) i += delaySize;
        return d[(size_t) i];
    };

    float peakIn = 0.0f, peakEcho = 0.0f;

    for (int s = 0; s < numSamples; ++s)
    {
        // One NaN would otherwise stick in the detector's filters for good.
        const float inL = std::isfinite (left[s]) ? left[s] : 0.0f;
        const float inR = std::isfinite (right[s]) ? right[s] : 0.0f;
        histL[(size_t) writeIndex] = inL;
        histR[(size_t) writeIndex] = inR;

        if (detector.process (0.5f * (inL + inR)) && ! bypassed)
            schedule (detector.lastPhrase(), p);
        ++t;

        // Latency changes fade the dry path out, jump, and fade back in.
        const bool switching = pendingDryTap != dryTap;
        const float sw = switchGain.next (switching ? 0.0f : 1.0f);
        if (switching && sw <= 0)
        {
            dryTap = pendingDryTap;
            windowRead = windowCount = 0;
        }

        const float dryL = readHistory (histL, dryTap), dryR = readHistory (histR, dryTap);
        const float tapL = readHistory (histL, doorTap), tapR = readHistory (histR, doorTap);

        const int64_t doorPosition = t - doorTap;
        while (windowCount > 0 && doorPosition >= windows[(size_t) windowRead].close)
        {
            windowRead = (windowRead + 1) % maxWindows;
            --windowCount;
        }
        const bool doorOpen = windowCount > 0 && doorPosition >= windows[(size_t) windowRead].open;
        if (doorOpen && ! doorWasOpen)
            throws.fetch_add (1, std::memory_order_relaxed);
        doorWasOpen = doorOpen;

        doorGain.step = doorOpen ? doorOpenStep : doorCloseStep;
        const float door = doorGain.next (doorOpen ? 1.0f : 0.0f);

        // Time changes crossfade between two read heads instead of moving one.
        if (crossfadeCount == 0 && headB != targetHead)
            { headB = targetHead; crossfadeCount = crossfadeLength; }
        float echoL = readDelay (delL, headA), echoR = readDelay (delR, headA);
        if (crossfadeCount > 0)
        {
            const float x = 1.0f - (float) crossfadeCount / (float) crossfadeLength;
            echoL += x * (readDelay (delL, headB) - echoL);
            echoR += x * (readDelay (delR, headB) - echoR);
            if (--crossfadeCount == 0)
                headA = headB;
        }

        const float between = betweenGain.next (betweenTarget);
        const float fb = feedbackGain.next (feedbackTarget);

        delL[(size_t) delayWrite] = limit (echoL * fb) + between * dryL * sw;
        delR[(size_t) delayWrite] = limit (echoR * fb) + between * dryR * sw;

        // A tap older than dry goes in early by its age modulo Time, so its echo lands on a
        // whole multiple of Time. Latched while the door is open: a jump here would splice.
        if (door <= 0)
        {
            const int age = doorTap - dryTap;
            doorWriteOffset = age > 0 ? age % std::min (headA, headB) : 0;
        }
        int doorWrite = delayWrite - doorWriteOffset;
        if (doorWrite < 0) doorWrite += delaySize;
        const float doorAmount = door * (1.0f - between) * sw;
        delL[(size_t) doorWrite] += doorAmount * tapL;
        delR[(size_t) doorWrite] += doorAmount * tapR;

        const float dg = dryGain.next (dryTarget), wg = wetGain.next (wetTarget);
        left[s] = dg * sw * dryL + wg * echoL;
        right[s] = dg * sw * dryR + wg * echoR;

        peakIn = std::max (peakIn, 0.5f * (std::abs (dryL) + std::abs (dryR)));
        peakEcho = std::max (peakEcho, 0.5f * (std::abs (echoL) + std::abs (echoR)) * wg);

        if (++writeIndex == historySize) writeIndex = 0;
        if (++delayWrite == delaySize) delayWrite = 0;
    }

    lastDoor.store (doorGain.value, std::memory_order_relaxed);
    lastInput.store (peakIn, std::memory_order_relaxed);
    lastEcho.store (peakEcho, std::memory_order_relaxed);
    lastEnv.store (detector.envelope(), std::memory_order_relaxed);
    lastThreshold.store (detector.thresholdLevel(), std::memory_order_relaxed);
    phraseActive.store (detector.isVoiced(), std::memory_order_relaxed);
}
