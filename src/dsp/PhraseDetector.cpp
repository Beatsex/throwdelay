#include "PhraseDetector.h"

#include <algorithm>
#include <cmath>

namespace
{
float onePole (double ms, double sr) { return (float) std::exp (-1000.0 / (ms * sr)); }
float dbToGain (float db) { return std::pow (10.0f, db / 20.0f); }
constexpr double twoPi = 6.283185307179586;
} // namespace

void PhraseDetector::prepare (double sampleRate)
{
    sr = sampleRate;
    hpCoeff = (float) std::exp (-twoPi * 80.0 / sr);
    lpCoeff = (float) std::exp (-twoPi * 1000.0 / sr);
    attack = onePole (1.0, sr);
    release = onePole (20.0, sr);
    refDecay = onePole (8000.0, sr);
    decimation = std::max (1, (int) std::lround (sr / 8000.0));
    confirm = (int64_t) (0.02 * sr);
    minPhrase = (int64_t) (0.1 * sr);
    if (minGap == 0)
        minGap = (int64_t) (0.25 * sr);
    reset();
}

void PhraseDetector::reset()
{
    hpState = lpState = env = ref = 0;
    frame.fill (0);
    decimationCount = hopCount = 0;
    periodic = above = inPhrase = false;
    n = voiceRun = lastVoiced = phraseStart = 0;
    phrase = {};
}

void PhraseDetector::setParameters (float sensitivityDb, float minGapMs)
{
    ratio = dbToGain (-sensitivityDb);
    minGap = (int64_t) (minGapMs * 0.001 * sr);
}

void PhraseDetector::updatePeriodicity()
{
    // Normalised cross-correlation of the newest frame against lagged copies.
    // Voices and held notes score high, breath noise does not.
    const float* newest = frame.data() + maxLag;
    float energy = 0;
    for (int i = 0; i < frameLength; ++i)
        energy += newest[i] * newest[i];

    float best = 0;
    if (energy > 1.0e-7f)
    {
        for (int lag = minLag; lag <= maxLag; ++lag)
        {
            float cross = 0, lagged = 0;
            for (int i = 0; i < frameLength; ++i)
            {
                cross += newest[i] * newest[i - lag];
                lagged += newest[i - lag] * newest[i - lag];
            }
            if (lagged > 1.0e-7f)
                best = std::max (best, cross / std::sqrt (energy * lagged));
        }
    }
    periodic = best > 0.7f;
}

bool PhraseDetector::process (float sample)
{
    ++n;

    hpState = sample + hpCoeff * (hpState - sample);
    const float y = sample - hpState;
    lpState = y + lpCoeff * (lpState - y);

    if (++decimationCount >= decimation)
    {
        decimationCount = 0;
        std::move (frame.begin() + 1, frame.end(), frame.begin());
        frame.back() = lpState;
        if (++hopCount >= hop)
        {
            hopCount = 0;
            updatePeriodicity();
        }
    }

    const float rectified = std::abs (y);
    env = rectified + (rectified > env ? attack : release) * (env - rectified);

    if (periodic && env > floorLevel)
        ref = std::max (env, ref * refDecay);

    const float threshold = std::max (ref * ratio, floorLevel);
    above = env > threshold * (above ? 1.0f : 1.41f);

    // Unvoiced sounds only count when they are nearly as loud as the phrase (whispered words, loud "s").
    const bool voiced = above && (periodic || (ref > 0 && env > ref * 0.5f));
    voiceRun = voiced ? voiceRun + 1 : 0;

    if (voiceRun >= confirm)
    {
        if (! inPhrase)
        {
            inPhrase = true;
            phraseStart = n - voiceRun;
        }
        lastVoiced = n;
    }

    if (inPhrase && n - lastVoiced >= minGap)
    {
        inPhrase = false;
        if (lastVoiced - phraseStart >= minPhrase)
        {
            phrase = { phraseStart, lastVoiced };
            return true;
        }
    }
    return false;
}
