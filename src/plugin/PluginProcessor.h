#pragma once

#include "ThrowEngine.h"

#include <juce_audio_processors/juce_audio_processors.h>

class ThrowDelayProcessor final : public juce::AudioProcessor, private juce::Timer
{
public:
    ThrowDelayProcessor();
    ~ThrowDelayProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Throw Delay"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 30.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;

    uint32_t throwCount() const { return engine.throwCount(); }
    uint32_t latencyChangeCount() const { return latencyChanges.load(); }
    float doorLevel() const { return engine.doorLevel(); }
    float inputLevel() const { return engine.inputLevel(); }
    float echoLevel() const { return engine.echoLevel(); }
    float detectorEnv() const { return engine.detectorEnv(); }
    float detectorThreshold() const { return engine.detectorThreshold(); }
    bool inPhrase() const { return engine.inPhrase(); }
    double currentBpm() const { return hostBpm.load (std::memory_order_relaxed); }

private:
    void timerCallback() override;
    int wantedLatency() const;
    void render (juce::AudioBuffer<float>&, bool bypassed);

    ThrowEngine engine;
    std::atomic<double> currentRate { 44100.0 };
    std::atomic<int> appliedLookahead { 0 };
    std::atomic<uint32_t> latencyChanges { 0 };
    std::atomic<double> hostBpm { 120.0 };

    std::atomic<float>* time, * sync, * division, * feedback, * mix, * lookahead, * live,
        * sensitivity, * minGap, * throwLength, * between;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThrowDelayProcessor)
};
