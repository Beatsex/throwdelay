// Command-line check of phrase detection and the delay engine. Exit code 0 = all passed.
#include "PhraseDetector.h"
#include "ThrowEngine.h"

#include <cmath>
#include <cstdio>
#include <limits>
#include <utility>
#include <random>
#include <vector>

namespace
{
int failures = 0;

void check (bool ok, const char* name, double value = 0)
{
    std::printf ("%s  %-58s %.4f\n", ok ? "ok  " : "FAIL", name, value);
    if (! ok)
        ++failures;
}

constexpr double pi = 3.141592653589793;
constexpr double toleranceSeconds = 0.05; // allowed error of a detected phrase end

struct Span { double start, end; };

// Sawtooth voice with syllable movement and a 10 ms decay at the end.
void addVoice (std::vector<float>& x, double sr, Span span, double level = 0.3, double hz = 180)
{
    double phase = 0;
    for (auto i = (size_t) (span.start * sr); i < (size_t) ((span.end + 0.01) * sr) && i < x.size(); ++i)
    {
        const double at = (double) i / sr;
        const double fade = std::min (std::min (1.0, (at - span.start) / 0.01), std::min (1.0, (span.end + 0.01 - at) / 0.01));
        const double syllable = 0.75 + 0.25 * std::sin (2.0 * pi * 4.0 * at);
        phase += hz / sr;
        x[i] += (float) (level * fade * syllable * (2.0 * (phase - std::floor (phase)) - 1.0));
    }
}

// Band-limited noise with a bell-shaped level: a breath. rms is measured before the bell.
void addBreath (std::vector<float>& x, double sr, Span span, double rms)
{
    std::mt19937 rng (42);
    std::normal_distribution<float> noise (0.0f, 1.0f);
    float lp = 0, lp2 = 0;
    const auto a = (float) (1.0 - std::exp (-2.0 * pi * 5000.0 / sr)), b = (float) (1.0 - std::exp (-2.0 * pi * 600.0 / sr));
    std::vector<float> band ((size_t) ((span.end - span.start) * sr));
    double energy = 0;
    for (auto& v : band)
    {
        const float white = noise (rng);
        lp += a * (white - lp);
        lp2 += b * (lp - lp2);
        v = lp - lp2;
        energy += (double) v * v;
    }
    const double gain = rms / std::sqrt (energy / (double) band.size());
    const auto first = (size_t) (span.start * sr);
    for (size_t i = 0; i < band.size() && first + i < x.size(); ++i)
        x[first + i] += (float) (gain * std::sin (pi * (double) i / (double) band.size()) * band[i]);
}

std::vector<double> detect (const std::vector<float>& x, double sr, float minGapMs = 250)
{
    PhraseDetector d;
    d.prepare (sr);
    d.setParameters (18, minGapMs);
    std::vector<double> ends;
    for (float v : x)
        if (d.process (v))
            ends.push_back ((double) d.lastPhrase().end / sr);
    return ends;
}

void testDetector (double sr)
{
    std::printf ("-- detector at %.0f Hz\n", sr);
    std::vector<float> x ((size_t) (10.0 * sr));
    const Span phrases[] = { { 0.5, 2.5 }, { 3.5, 5.0 }, { 6.0, 8.0 } };
    for (auto p : phrases)
        addVoice (x, sr, p);

    auto ends = detect (x, sr);
    bool close = ends.size() == 3;
    double worst = 0;
    for (size_t i = 0; close && i < 3; ++i)
    {
        worst = std::max (worst, std::abs (ends[i] - phrases[i].end));
        close = worst <= toleranceSeconds;
    }
    check (close, "known pauses found within tolerance (worst s)", worst);

    std::vector<float> shortGaps ((size_t) (6.0 * sr));
    addVoice (shortGaps, sr, { 0.5, 2.0 });
    addVoice (shortGaps, sr, { 2.15, 3.5 }); // 150 ms pause, Min Gap is 250 ms
    addVoice (shortGaps, sr, { 3.65, 5.0 });
    ends = detect (shortGaps, sr);
    check (ends.size() == 1 && std::abs (ends[0] - 5.0) <= toleranceSeconds, "pause below Min Gap triggers nothing", (double) ends.size());

    std::vector<float> withBreath = x;
    addBreath (withBreath, sr, { 2.75, 3.2 }, 0.03); // -20 dB against the voice peak
    addBreath (withBreath, sr, { 5.3, 5.8 }, 0.03);
    ends = detect (withBreath, sr);
    close = ends.size() == 3 && std::abs (ends[0] - 2.5) <= toleranceSeconds && std::abs (ends[1] - 5.0) <= toleranceSeconds;
    check (close, "breath in pause neither blocks nor adds a phrase end", (double) ends.size());

    std::vector<float> breathOnly ((size_t) (5.0 * sr));
    addVoice (breathOnly, sr, { 0.5, 1.5 });
    addBreath (breathOnly, sr, { 2.0, 2.4 }, 0.03);
    addBreath (breathOnly, sr, { 3.0, 3.4 }, 0.03);
    check (detect (breathOnly, sr).size() == 1, "breaths alone trigger no echo", 0);

    std::vector<float> breathFirst ((size_t) (3.0 * sr));
    addBreath (breathFirst, sr, { 0.5, 1.0 }, 0.03);
    check (detect (breathFirst, sr).empty(), "breath before any singing triggers nothing", 0);
}

struct Rendered { std::vector<float> l, r; };

Rendered render (const std::vector<float>& in, double sr, const ThrowEngine::Params& p, int block, ThrowEngine* use = nullptr)
{
    ThrowEngine local;
    auto& engine = use ? *use : local;
    engine.prepare (sr);
    Rendered o { in, in };
    for (size_t at = 0; at < in.size(); at += (size_t) block)
        engine.process (o.l.data() + at, o.r.data() + at, (int) std::min ((size_t) block, in.size() - at), p);
    return o;
}

double peak (const std::vector<float>& x, size_t from, size_t to)
{
    double m = 0;
    for (size_t i = from; i < to && i < x.size(); ++i)
        m = std::max (m, (double) std::abs (x[i]));
    return m;
}

void testEngine (double sr, int block)
{
    std::printf ("-- engine at %.0f Hz, block %d\n", sr, block);
    const auto L = (int) std::lround (1.0 * sr);
    std::vector<float> in ((size_t) (8.0 * sr));
    addVoice (in, sr, { 0.5, 3.0 });
    addVoice (in, sr, { 4.0, 5.5 });

    ThrowEngine::Params p;
    p.lookaheadSamples = L;
    p.delayMs = 250;
    p.feedback = 0;
    p.mix = 0;
    auto out = render (in, sr, p, block);
    double residual = 0;
    for (size_t i = 0; i + (size_t) L < in.size(); ++i)
        residual = std::max (residual, (double) std::abs (out.l[i + (size_t) L] - in[i]));
    check (residual == 0, "null test: dry equals input after latency", residual);

    // Wet only: with L removed, echoes must sit around phrase ends (+250 ms) and nowhere else.
    p.mix = 1;
    p.throwMs = 400;
    out = render (in, sr, p, block);
    const auto at = [&] (double s) { return (size_t) ((s + 1.0) * sr); };
    check (peak (out.l, at (0.5), at (2.8)) < 1.0e-6, "no echo during the phrase (Between 0)", peak (out.l, at (0.5), at (2.8)));
    check (peak (out.l, at (2.9), at (3.3)) > 0.1, "echo present at the phrase end", peak (out.l, at (2.9), at (3.3)));

    // Timing: echo of the phrase end lands exactly Time after dry. Output is wet only here.
    size_t firstEcho = 0;
    for (size_t i = 0; i < out.l.size(); ++i)
        if (std::abs (out.l[i]) > 1.0e-3f) { firstEcho = i; break; }
    const double opens = (double) firstEcho / sr - 1.0 - 0.25;
    check (std::abs (opens - 2.6) < 0.06, "door opens Throw Length before end, echo after Time", opens);

    p.between = 0.25f;
    out = render (in, sr, p, block);
    const auto mid = peak (out.l, at (1.0), at (2.0));
    check (mid > 0.02 && mid < 0.12, "Between gives quiet echoes during the phrase", mid);
    p.between = 0;

    // Tail: a long feedback must ring on after the door closed.
    p.feedback = 0.7f;
    out = render (in, sr, p, block);
    check (peak (out.l, at (3.8), at (3.95)) > 0.02, "tail survives the closed door", peak (out.l, at (3.8), at (3.95)));

    // Feedback bound: hottest setting on a hot signal.
    std::vector<float> hot ((size_t) (6.0 * sr));
    addVoice (hot, sr, { 0.2, 5.5 }, 1.0);
    p.feedback = 0.95f;
    p.between = 1;
    p.delayMs = 50;
    out = render (hot, sr, p, block);
    check (peak (out.l, 0, out.l.size()) < 2.0, "feedback stays bounded", peak (out.l, 0, out.l.size()));

    // Live mode: no dry delay, and an echo still arrives after a pause starts.
    ThrowEngine::Params live;
    live.lookaheadSamples = 0;
    live.delayMs = 900;
    live.feedback = 0;
    live.mix = 1;
    out = render (in, sr, live, block);
    check (peak (out.l, (size_t) (3.0 * sr), (size_t) (4.0 * sr)) > 0.1, "live mode echoes the phrase end", peak (out.l, (size_t) (3.0 * sr), (size_t) (4.0 * sr)));
    check (peak (out.l, (size_t) (0.5 * sr), (size_t) (2.5 * sr)) < 1.0e-6, "live mode stays quiet in the phrase", peak (out.l, (size_t) (0.5 * sr), (size_t) (2.5 * sr)));
    live.mix = 0;
    out = render (in, sr, live, block);
    residual = 0;
    for (size_t i = 0; i < in.size(); ++i)
        residual = std::max (residual, (double) std::abs (out.l[i] - in[i]));
    check (residual == 0, "live mode dry has no delay", residual);

    const auto firstAndLast = [] (const std::vector<float>& x, double rate) {
        size_t first = 0, last = 0;
        for (size_t i = 0; i < x.size(); ++i)
            if (std::abs (x[i]) > 1.0e-3f)
            {
                if (first == 0) first = i;
                last = i;
            }
        return std::pair<double, double> { (double) first / rate, (double) last / rate };
    };

    // Time shorter than the tap age: door opens near 2.64 s, tap age 660 ms, so with 250 ms
    // the echo sits 750 ms later (3 x Time), not 660 + 250 ms later.
    live.mix = 1;
    live.delayMs = 250;
    out = render (in, sr, live, block);
    const auto onGrid = firstAndLast (out.l, sr).first;
    check (std::abs (onGrid - 3.39) < 0.06, "live echo stays on a multiple of Time", onGrid);

    // Longest live settings must keep the whole throw, which needs more than 2 s of history.
    std::vector<float> longPhrase ((size_t) (10.0 * sr));
    addVoice (longPhrase, sr, { 0.5, 4.0 });
    live.delayMs = 3100;
    live.minGapMs = 1500;
    live.throwMs = 1500;
    out = render (longPhrase, sr, live, block);
    const auto span = firstAndLast (out.l, sr);
    check (span.second - span.first > 1.4, "live throw keeps full length at max settings", span.second - span.first);

    // A NaN in the input must not reach the output or stop detection.
    auto poisoned = in;
    poisoned[(size_t) (1.0 * sr)] = std::numeric_limits<float>::quiet_NaN();
    p = {};
    p.lookaheadSamples = L;
    p.delayMs = 250;
    p.feedback = 0;
    p.mix = 1;
    out = render (poisoned, sr, p, block);
    bool finite = true;
    for (float v : out.l)
        finite = finite && std::isfinite (v);
    check (finite && peak (out.l, at (2.9), at (3.3)) > 0.1, "NaN input is dropped, detection survives", 0);
}

// Time change and door movement on a steady sine: largest sample jump must stay near the sine's own.
void testClicks (double sr)
{
    std::printf ("-- clicks at %.0f Hz\n", sr);
    std::vector<float> in ((size_t) (6.0 * sr));
    for (size_t i = 0; i < (size_t) (5.0 * sr); ++i)
        in[i] = (float) (0.3 * std::sin (2.0 * pi * 220.0 * (double) i / sr));

    ThrowEngine engine;
    engine.prepare (sr);
    ThrowEngine::Params p;
    p.lookaheadSamples = (int) (0.5 * sr);
    p.mix = 1;
    p.between = 1;
    p.feedback = 0;
    auto l = in, r = in;
    const size_t half = l.size() / 2;
    const int block = 64;
    for (size_t at = 0; at < l.size(); at += (size_t) block)
    {
        p.delayMs = at < half ? 200.0f : 333.0f;
        engine.process (l.data() + at, r.data() + at, (int) std::min ((size_t) block, l.size() - at), p);
    }
    double jump = 0;
    for (size_t i = (size_t) (sr); i + 1 < (size_t) (4.5 * sr); ++i)
        jump = std::max (jump, (double) std::abs (l[i + 1] - l[i]));
    const double sineStep = 0.3 * 2.0 * pi * 220.0 / sr;
    check (jump < sineStep * 2.0, "time change does not click", jump);
}
} // namespace

int main()
{
    for (double sr : { 44100.0, 48000.0, 88200.0, 96000.0 })
    {
        testDetector (sr);
        testClicks (sr);
    }
    for (double sr : { 44100.0, 96000.0 })
        for (int block : { 32, 2048 })
            testEngine (sr, block);

    std::printf (failures == 0 ? "ALL PASSED\n" : "%d FAILED\n", failures);
    return failures == 0 ? 0 : 1;
}
