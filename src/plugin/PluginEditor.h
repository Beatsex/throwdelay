#pragma once

#include "PluginProcessor.h"

#include <juce_audio_utils/juce_audio_utils.h>

// Flat, hairline-ruled look: ink on ivory with a pink accent, mono micro labels.
class ThrowLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    static constexpr juce::uint32 ink = 0xff111111, ivory = 0xfff2f1ed, accent = 0xfff2b8c6, muted = 0xff333333;
    static constexpr float hairline = 1.5f;

    ThrowLookAndFeel();

    static juce::Font mono (float height);
    static juce::Font sans (float height, bool semibold = false);

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height, float sliderPos, float,
                           float, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawLinearSliderOutline (juce::Graphics&, int, int, int, int, juce::Slider::SliderStyle, juce::Slider&) override {}
    juce::Label* createSliderTextBox (juce::Slider&) override;
    int getSliderThumbRadius (juce::Slider&) override { return 0; }
    void drawComboBox (juce::Graphics&, int width, int height, bool, int, int, int, int, juce::ComboBox&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getLabelFont (juce::Label&) override;
};

class ThrowDelayEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit ThrowDelayEditor (ThrowDelayProcessor&);
    ~ThrowDelayEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    struct Knob
    {
        juce::Slider slider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxAbove };
        juce::Label label;
        std::unique_ptr<SliderAttachment> attachment;
    };

    enum class DragTarget
    {
        none,
        sensitivityLine,
        throwWindow,
        minGapLine,
        echoHead,
        mixFloor
    };

    void timerCallback() override;
    void setupKnob (Knob&, const char* id, const char* name);

    float getParamNorm (const char* id) const;
    void setParamNorm (const char* id, float norm);

    // Geometry of the central interactive Phrase & Throw Visualizer
    juce::Rectangle<float> scopePlotBounds() const;
    juce::Rectangle<float> echoPlotBounds() const;
    float sensitivityLineY (juce::Rectangle<float> plot) const;
    juce::Rectangle<float> throwWindowRect (juce::Rectangle<float> plot) const;
    float minGapX (juce::Rectangle<float> plot) const;
    juce::Point<float> firstTapHandle (juce::Rectangle<float> echoPlot) const;
    DragTarget hitTestVisualizer (juce::Point<float> pos) const;

    ThrowLookAndFeel look;
    ThrowDelayProcessor& processor;

    Knob time, feedback, mix, sensitivity, minGap, throwLength, between;
    juce::ComboBox division, lookahead;
    juce::ToggleButton sync { "SYNC" }, live { "LIVE" };
    juce::Label divisionLabel, lookaheadLabel;
    std::unique_ptr<ComboAttachment> divisionAttachment, lookaheadAttachment;
    std::unique_ptr<ButtonAttachment> syncAttachment, liveAttachment;

    static constexpr int historyLength = 180; // 6 s at 30 Hz
    struct HistoryFrame
    {
        float env = 0.0f;
        float door = 0.0f;
        float echo = 0.0f;
        bool voiced = false;
        bool triggered = false;
    };
    std::array<HistoryFrame, historyLength> history {};
    int historyIndex = 0, hintTicks = 0;
    uint32_t seenThrows = 0, seenLatencyChanges = 0;

    double animPhase = 0.0;
    float smoothDoor = 0.0f, smoothInput = 0.0f, smoothEcho = 0.0f;
    DragTarget activeDrag = DragTarget::none;
    DragTarget hoverTarget = DragTarget::none;

    juce::Rectangle<int> headerArea, footerArea, leftSidebar, centerCanvas, rightSidebar;
    juce::Rectangle<int> headerLeft, headerCenter, headerRight;
    juce::Rectangle<int> footerLeft, footerCenter, footerRight;
    std::array<juce::Rectangle<int>, 4> leftCells {};
    std::array<juce::Rectangle<int>, 4> rightCells {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThrowDelayEditor)
};
