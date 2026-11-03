#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr double lookaheadSeconds[] = { 0.5, 1.0, 2.0 };
// Length of each division in quarter notes, same order as the choice list below.
constexpr double divisionQuarters[] = { 0.125, 0.25 / 1.5, 0.25, 0.375, 0.5 / 1.5, 0.5, 0.75, 1.0 / 1.5,
                                        1.0, 1.5, 2.0 / 1.5, 2.0, 3.0, 4.0 };

juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
{
    using namespace juce;
    using Float = AudioParameterFloat;
    const auto unit = [] (const char* label, int decimals) {
        return AudioParameterFloatAttributes().withLabel (label)
            .withStringFromValueFunction ([label, decimals] (float v, int) { return String (v, decimals) + " " + label; })
            .withValueFromStringFunction ([] (const String& text) { return text.getFloatValue(); });
    };
    const auto ms = unit ("ms", 1);
    const auto wholeMs = unit ("ms", 0); // Min Gap and Throw Length step in whole milliseconds
    const auto db = unit ("dB", 1);
    const auto percent = AudioParameterFloatAttributes()
        .withStringFromValueFunction ([] (float v, int) { return String (juce::roundToInt (v * 100.0f)) + " %"; })
        .withValueFromStringFunction ([] (const String& text) { return text.getFloatValue() / 100.0f; });

    AudioProcessorValueTreeState::ParameterLayout layout;
    const auto add = [&layout] (auto p) { layout.add (std::move (p)); };

    add (std::make_unique<Float> (ParameterID { "time", 1 }, "Time", NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.4f), 375.0f, ms));
    add (std::make_unique<AudioParameterBool> (ParameterID { "sync", 1 }, "Sync", true));
    add (std::make_unique<AudioParameterChoice> (ParameterID { "division", 1 }, "Division",
        StringArray { "1/32", "1/16T", "1/16", "1/16.", "1/8T", "1/8", "1/8.", "1/4T", "1/4", "1/4.", "1/2T", "1/2", "1/2.", "1/1" }, 6));
    add (std::make_unique<Float> (ParameterID { "feedback", 1 }, "Feedback", NormalisableRange<float> (0.0f, 0.95f), 0.35f, percent));
    add (std::make_unique<Float> (ParameterID { "mix", 1 }, "Mix", NormalisableRange<float> (0.0f, 1.0f), 0.3f, percent));

    // Both change the reported latency, so they stay out of host automation.
    add (std::make_unique<AudioParameterChoice> (ParameterID { "lookahead", 1 }, "Look-ahead",
        StringArray { "0.5 s", "1 s", "2 s" }, 1, AudioParameterChoiceAttributes().withAutomatable (false)));
    add (std::make_unique<AudioParameterBool> (ParameterID { "live", 1 }, "Live", false, AudioParameterBoolAttributes().withAutomatable (false)));

    add (std::make_unique<Float> (ParameterID { "sensitivity", 1 }, "Sensitivity", NormalisableRange<float> (6.0f, 40.0f), 18.0f, db));
    add (std::make_unique<Float> (ParameterID { "mingap", 1 }, "Min Gap", NormalisableRange<float> (50.0f, 1500.0f, 1.0f, 0.5f), 250.0f, wholeMs));
    add (std::make_unique<Float> (ParameterID { "throw", 1 }, "Throw Length", NormalisableRange<float> (50.0f, 1500.0f, 1.0f, 0.5f), 400.0f, wholeMs));
    add (std::make_unique<Float> (ParameterID { "between", 1 }, "Between", NormalisableRange<float> (0.0f, 1.0f), 0.0f, percent));
    return layout;
}
} // namespace

ThrowDelayProcessor::ThrowDelayProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "ThrowDelay", makeLayout())
{
    const auto raw = [this] (const char* id) { return parameters.getRawParameterValue (id); };
    time = raw ("time"); sync = raw ("sync"); division = raw ("division");
    feedback = raw ("feedback"); mix = raw ("mix"); lookahead = raw ("lookahead"); live = raw ("live");
    sensitivity = raw ("sensitivity"); minGap = raw ("mingap"); throwLength = raw ("throw"); between = raw ("between");
}

ThrowDelayProcessor::~ThrowDelayProcessor() { stopTimer(); }

bool ThrowDelayProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
           && (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo());
}

int ThrowDelayProcessor::wantedLatency() const
{
    if (live->load() > 0.5f)
        return 0;
    const auto index = juce::jlimit (0, 2, (int) lookahead->load());
    return (int) std::round (lookaheadSeconds[index] * currentRate.load());
}

void ThrowDelayProcessor::prepareToPlay (double sampleRate, int)
{
    currentRate = sampleRate;
    engine.prepare (sampleRate);
    const auto latency = wantedLatency();
    setLatencySamples (latency);
    appliedLookahead = latency;
    startTimerHz (20); // not earlier: before this the sample rate is unknown and latency would be wrong
}

// setLatencySamples takes a lock inside JUCE, so it runs here and not in processBlock.
void ThrowDelayProcessor::timerCallback()
{
    const auto latency = wantedLatency();
    if (latency != getLatencySamples())
    {
        setLatencySamples (latency);
        appliedLookahead = latency;
        ++latencyChanges;
    }
}

void ThrowDelayProcessor::render (juce::AudioBuffer<float>& buffer, bool bypassed)
{
    juce::ScopedNoDenormals noDenormals;

    if (auto* head = getPlayHead())
        if (auto position = head->getPosition())
            if (auto bpm = position->getBpm(); bpm.hasValue() && *bpm > 0.0)
                hostBpm.store (*bpm, std::memory_order_relaxed);

    // The echo is measured from the delayed dry, and the host shifts both by the same latency,
    // so plain tempo-derived time lands on the grid of the compensated track.
    float delayMs = time->load();
    if (sync->load() > 0.5f)
    {
        const auto index = juce::jlimit (0, (int) std::size (divisionQuarters) - 1, (int) division->load());
        delayMs = (float) (divisionQuarters[index] * 60000.0 / hostBpm.load (std::memory_order_relaxed));
    }

    ThrowEngine::Params params;
    params.delayMs = delayMs;
    params.feedback = feedback->load();
    params.mix = mix->load();
    params.sensitivityDb = sensitivity->load();
    params.minGapMs = minGap->load();
    params.throwMs = throwLength->load();
    params.between = between->load();
    params.lookaheadSamples = appliedLookahead.load();

    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getWritePointer (buffer.getNumChannels() > 1 ? 1 : 0);
    if (getTotalNumInputChannels() == 1 && buffer.getNumChannels() > 1)
        juce::FloatVectorOperations::copy (right, left, buffer.getNumSamples());

    engine.process (left, right, buffer.getNumSamples(), params, bypassed);
}

void ThrowDelayProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) { render (buffer, false); }
void ThrowDelayProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) { render (buffer, true); }

juce::AudioProcessorEditor* ThrowDelayProcessor::createEditor() { return new ThrowDelayEditor (*this); }

void ThrowDelayProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    juce::XmlElement xml ("ThrowDelay");
    for (auto* p : getParameters())
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (p))
            xml.setAttribute (ranged->getParameterID(), (double) ranged->getValue());
    copyXmlToBinary (xml, dest);
}

void ThrowDelayProcessor::setStateInformation (const void* data, int size)
{
    const auto xml = getXmlFromBinary (data, size);
    if (xml == nullptr || ! xml->hasTagName ("ThrowDelay"))
        return;
    for (auto* p : getParameters())
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (p))
            if (xml->hasAttribute (ranged->getParameterID()))
                ranged->setValueNotifyingHost ((float) xml->getDoubleAttribute (ranged->getParameterID()));

    // An offline render may start before the next tick. Before prepareToPlay, that call sets it.
    if (isTimerRunning())
        timerCallback();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ThrowDelayProcessor(); }
