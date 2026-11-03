#include "PluginEditor.h"

#include <cmath>

namespace
{
const juce::Colour inkColour { ThrowLookAndFeel::ink };
const juce::Colour ivoryColour { ThrowLookAndFeel::ivory };
const juce::Colour accentColour { ThrowLookAndFeel::accent };
const juce::Colour mutedColour { ThrowLookAndFeel::muted };

constexpr double twoPi = 6.283185307179586;

juce::String caps (const char* text) { return juce::String (juce::CharPointer_UTF8 (text)).toUpperCase(); }

void drawSoftBlob (juce::Graphics& g, float cx, float cy, float radius)
{
    juce::ColourGradient grad (ivoryColour.withAlpha (0.85f), cx, cy,
                               ivoryColour.withAlpha (0.0f), cx + radius, cy, true);
    grad.addColour (0.45, ivoryColour.withAlpha (0.65f));
    grad.addColour (0.78, ivoryColour.withAlpha (0.22f));
    g.setGradientFill (grad);
    g.fillEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
}

void drawHairlineH (juce::Graphics& g, int x, int y, int w)
{
    g.setColour (inkColour);
    g.fillRect (juce::Rectangle<float> ((float) x, (float) y, (float) w, ThrowLookAndFeel::hairline));
}

void drawHairlineV (juce::Graphics& g, int x, int y, int h)
{
    g.setColour (inkColour);
    g.fillRect (juce::Rectangle<float> ((float) x, (float) y, ThrowLookAndFeel::hairline, (float) h));
}
} // namespace

juce::Font ThrowLookAndFeel::mono (float height)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), height, juce::Font::plain));
}

juce::Font ThrowLookAndFeel::sans (float height, bool semibold)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultSansSerifFontName(), height,
                                          semibold ? juce::Font::bold : juce::Font::plain));
}

ThrowLookAndFeel::ThrowLookAndFeel()
{
    setColour (juce::Slider::trackColourId, inkColour);
    setColour (juce::Slider::backgroundColourId, inkColour.withAlpha (0.15f));
    setColour (juce::Slider::textBoxTextColourId, inkColour);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxHighlightColourId, accentColour);
    setColour (juce::Label::textColourId, inkColour);
    setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    setColour (juce::ComboBox::backgroundColourId, ivoryColour);
    setColour (juce::ComboBox::textColourId, inkColour);
    setColour (juce::ComboBox::outlineColourId, inkColour);
    setColour (juce::ComboBox::arrowColourId, inkColour);
    setColour (juce::PopupMenu::backgroundColourId, ivoryColour);
    setColour (juce::PopupMenu::textColourId, inkColour);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, inkColour);
    setColour (juce::PopupMenu::highlightedTextColourId, accentColour);
}

juce::Label* ThrowLookAndFeel::createSliderTextBox (juce::Slider& slider)
{
    auto* l = LookAndFeel_V4::createSliderTextBox (slider);
    l->setJustificationType (juce::Justification::centredLeft);
    l->setBorderSize (juce::BorderSize<int> (0));
    l->setColour (juce::Label::textColourId, inkColour);
    l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    return l;
}

void ThrowLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                         float, float, juce::Slider::SliderStyle, juce::Slider& slider)
{
    const float barH = 5.0f;
    const float barY = (float) y + ((float) height - barH) * 0.5f;
    const juce::Rectangle<float> bar ((float) x, barY, (float) width, barH);
    g.setColour (slider.findColour (juce::Slider::backgroundColourId));
    g.fillRect (bar);
    g.setColour (slider.findColour (juce::Slider::trackColourId));
    const float filledRight = juce::jlimit ((float) x, (float) (x + width), sliderPos);
    g.fillRect (bar.withRight (filledRight));
}

void ThrowLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int,
                                     juce::ComboBox& box)
{
    const auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRect (bounds);
    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRect (bounds, hairline);

    const auto cx = (float) width - 14.0f, cy = (float) height * 0.5f;
    juce::Path caret;
    caret.addTriangle (cx - 4.0f, cy - 2.0f, cx + 4.0f, cy - 2.0f, cx, cy + 3.0f);
    g.setColour (box.findColour (juce::ComboBox::arrowColourId));
    g.fillPath (caret);
}

void ThrowLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool highlighted, bool)
{
    const auto bounds = button.getLocalBounds().toFloat();
    const auto on = button.getToggleState();
    g.setColour (on ? inkColour : (highlighted ? inkColour.withAlpha (0.08f) : juce::Colours::transparentBlack));
    g.fillRect (bounds);
    g.setColour (inkColour);
    g.drawRect (bounds, hairline);
    g.setFont (mono (11.0f));
    g.setColour (on ? accentColour : inkColour);
    g.drawText (button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
}

juce::Font ThrowLookAndFeel::getComboBoxFont (juce::ComboBox&) { return sans (13.0f, false); }

juce::Font ThrowLookAndFeel::getLabelFont (juce::Label& label)
{
    if (dynamic_cast<juce::Slider*> (label.getParentComponent()) != nullptr)
        return sans (20.0f, true);
    return label.getFont();
}

ThrowDelayEditor::ThrowDelayEditor (ThrowDelayProcessor& p) : AudioProcessorEditor (p), processor (p)
{
    setLookAndFeel (&look);

    setupKnob (time, "time", "Time (Verzoegerung)");
    setupKnob (feedback, "feedback", "Feedback (Wiederholungen)");
    setupKnob (mix, "mix", "Mix (Effekt-Anteil)");
    setupKnob (sensitivity, "sensitivity", "Sensitivity (Schwellwert)");
    setupKnob (minGap, "mingap", "Min Gap (Pausenlaenge)");
    setupKnob (throwLength, "throw", "Throw Length (Wurf-Dauer)");
    setupKnob (between, "between", "Between (Durchlass)");

    auto& state = processor.parameters;
    const auto setupCombo = [&] (juce::ComboBox& box, juce::Label& label, const char* id, const char* name,
                                 std::unique_ptr<ComboAttachment>& attachment) {
        box.addItemList (state.getParameter (id)->getAllValueStrings(), 1);
        attachment = std::make_unique<ComboAttachment> (state, id, box);
        label.setText (caps (name), juce::dontSendNotification);
        label.setFont (ThrowLookAndFeel::mono (10.5f));
        label.setColour (juce::Label::textColourId, mutedColour);
        label.setBorderSize (juce::BorderSize<int> (0));
        addAndMakeVisible (box);
        addAndMakeVisible (label);
    };
    setupCombo (division, divisionLabel, "division", "Division (Notenwert)", divisionAttachment);
    setupCombo (lookahead, lookaheadLabel, "lookahead", "Look-ahead (Vorlauf)", lookaheadAttachment);

    syncAttachment = std::make_unique<ButtonAttachment> (state, "sync", sync);
    liveAttachment = std::make_unique<ButtonAttachment> (state, "live", live);
    addAndMakeVisible (sync);
    addAndMakeVisible (live);

    seenThrows = processor.throwCount();
    seenLatencyChanges = processor.latencyChangeCount();
    setSize (980, 620);
    startTimerHz (30);
}

ThrowDelayEditor::~ThrowDelayEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void ThrowDelayEditor::setupKnob (Knob& knob, const char* id, const char* name)
{
    knob.slider.setSliderStyle (juce::Slider::LinearHorizontal);
    knob.slider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, 240, 26);
    knob.slider.setColour (juce::Slider::textBoxTextColourId, inkColour);
    knob.slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    knob.slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);

    if (auto* param = processor.parameters.getParameter (id))
    {
        knob.slider.textFromValueFunction = [param] (double v) {
            return param->getText (param->convertTo0to1 ((float) v), 0);
        };
        knob.slider.valueFromTextFunction = [param] (const juce::String& text) {
            return (double) param->convertFrom0to1 (param->getValueForText (text));
        };
    }

    knob.attachment = std::make_unique<SliderAttachment> (processor.parameters, id, knob.slider);
    knob.label.setText (caps (name), juce::dontSendNotification);
    knob.label.setFont (ThrowLookAndFeel::mono (10.0f));
    knob.label.setColour (juce::Label::textColourId, mutedColour);
    knob.label.setBorderSize (juce::BorderSize<int> (0));
    knob.label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (knob.slider);
    addAndMakeVisible (knob.label);
}

float ThrowDelayEditor::getParamNorm (const char* id) const
{
    if (auto* p = processor.parameters.getParameter (id))
        return p->getValue();
    return 0.5f;
}

void ThrowDelayEditor::setParamNorm (const char* id, float norm)
{
    if (auto* p = processor.parameters.getParameter (id))
        p->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, norm));
}

juce::Rectangle<float> ThrowDelayEditor::scopePlotBounds() const
{
    auto r = centerCanvas.reduced (22, 14).toFloat();
    r.removeFromTop (24.0f);
    return r.removeFromTop (r.getHeight() * 0.50f);
}

juce::Rectangle<float> ThrowDelayEditor::echoPlotBounds() const
{
    auto r = centerCanvas.reduced (22, 14).toFloat();
    r.removeFromTop (24.0f);
    r.removeFromTop (r.getHeight() * 0.50f);
    r.removeFromTop (28.0f);
    return r;
}

float ThrowDelayEditor::sensitivityLineY (juce::Rectangle<float> plot) const
{
    const float sensNorm = getParamNorm ("sensitivity");
    return plot.getY() + 22.0f + sensNorm * (plot.getHeight() - 44.0f);
}

juce::Rectangle<float> ThrowDelayEditor::throwWindowRect (juce::Rectangle<float> plot) const
{
    const float endX = plot.getX() + plot.getWidth() * 0.62f;
    const float throwNorm = getParamNorm ("throw");
    const float maxW = plot.getWidth() * 0.46f;
    const float minW = 32.0f;
    const float w = minW + throwNorm * (maxW - minW);
    return juce::Rectangle<float> (endX - w, plot.getY() + 18.0f, w, plot.getHeight() - 26.0f);
}

float ThrowDelayEditor::minGapX (juce::Rectangle<float> plot) const
{
    const float endX = plot.getX() + plot.getWidth() * 0.62f;
    const float gapNorm = getParamNorm ("mingap");
    const float minW = 28.0f;
    const float maxW = plot.getWidth() * 0.34f;
    return endX + minW + gapNorm * (maxW - minW);
}

juce::Point<float> ThrowDelayEditor::firstTapHandle (juce::Rectangle<float> echoPlot) const
{
    const char* timeId = sync.getToggleState() ? "division" : "time";
    const float tNorm = getParamNorm (timeId);
    const float fbNorm = getParamNorm ("feedback");

    const float startX = echoPlot.getX() + 44.0f;
    const float maxSpacing = (echoPlot.getWidth() - 68.0f) / 5.0f;
    const float spacing = 22.0f + tNorm * (maxSpacing - 22.0f);
    const float tapX = startX + spacing;
    const float tapH = (echoPlot.getHeight() - 28.0f) * (0.20f + 0.72f * fbNorm);
    return { tapX, echoPlot.getBottom() - 16.0f - tapH };
}

ThrowDelayEditor::DragTarget ThrowDelayEditor::hitTestVisualizer (juce::Point<float> pos) const
{
    const auto scope = scopePlotBounds();
    if (scope.expanded (0.0f, 6.0f).contains (pos))
    {
        const auto win = throwWindowRect (scope);
        const float gapMarkerX = minGapX (scope);
        const float sensY = sensitivityLineY (scope);

        if (std::abs (pos.x - win.getX()) <= 12.0f && pos.y >= win.getY() && pos.y <= win.getBottom())
            return DragTarget::throwWindow;

        if (std::abs (pos.x - gapMarkerX) <= 12.0f && pos.y >= scope.getY() && pos.y <= scope.getBottom())
            return DragTarget::minGapLine;

        if (std::abs (pos.y - sensY) <= 10.0f)
            return DragTarget::sensitivityLine;

        if (win.contains (pos))
            return DragTarget::throwWindow;
    }

    const auto echo = echoPlotBounds();
    if (echo.expanded (0.0f, 6.0f).contains (pos))
    {
        const auto h = firstTapHandle (echo);
        if (h.getDistanceFrom (pos) <= 18.0f)
            return DragTarget::echoHead;

        if (pos.y < echo.getBottom() - 20.0f)
            return DragTarget::echoHead;

        return DragTarget::mixFloor;
    }

    return DragTarget::none;
}

void ThrowDelayEditor::mouseMove (const juce::MouseEvent& e)
{
    const auto target = hitTestVisualizer (e.position);
    if (target != hoverTarget)
    {
        hoverTarget = target;
        switch (target)
        {
            case DragTarget::sensitivityLine:
            case DragTarget::mixFloor:
                setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
                break;
            case DragTarget::throwWindow:
            case DragTarget::minGapLine:
                setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
                break;
            case DragTarget::echoHead:
                setMouseCursor (juce::MouseCursor::DraggingHandCursor);
                break;
            case DragTarget::none:
                setMouseCursor (juce::MouseCursor::NormalCursor);
                break;
        }
        repaint (centerCanvas);
    }
}

void ThrowDelayEditor::mouseDown (const juce::MouseEvent& e)
{
    activeDrag = hitTestVisualizer (e.position);
    if (activeDrag != DragTarget::none)
        mouseDrag (e);
}

void ThrowDelayEditor::mouseDrag (const juce::MouseEvent& e)
{
    if (activeDrag == DragTarget::none)
        return;

    const auto scope = scopePlotBounds();
    const auto echo = echoPlotBounds();

    switch (activeDrag)
    {
        case DragTarget::sensitivityLine:
        {
            const float norm = (e.position.y - (scope.getY() + 22.0f)) / std::max (1.0f, scope.getHeight() - 44.0f);
            setParamNorm ("sensitivity", norm);
            break;
        }
        case DragTarget::throwWindow:
        {
            const float endX = scope.getX() + scope.getWidth() * 0.62f;
            const float maxW = scope.getWidth() * 0.46f;
            const float minW = 32.0f;
            const float w = endX - e.position.x;
            setParamNorm ("throw", (w - minW) / std::max (1.0f, maxW - minW));
            break;
        }
        case DragTarget::minGapLine:
        {
            const float endX = scope.getX() + scope.getWidth() * 0.62f;
            const float minW = 28.0f;
            const float maxW = scope.getWidth() * 0.34f;
            const float dist = e.position.x - endX;
            setParamNorm ("mingap", (dist - minW) / std::max (1.0f, maxW - minW));
            break;
        }
        case DragTarget::echoHead:
        {
            const char* timeId = sync.getToggleState() ? "division" : "time";
            const float startX = echo.getX() + 44.0f;
            const float maxSpacing = (echo.getWidth() - 68.0f) / 5.0f;
            const float spacing = e.position.x - startX;
            setParamNorm (timeId, (spacing - 22.0f) / std::max (1.0f, maxSpacing - 22.0f));

            const float availH = echo.getHeight() - 28.0f;
            const float tapH = (echo.getBottom() - 16.0f) - e.position.y;
            const float fbNorm = (tapH / std::max (1.0f, availH) - 0.20f) / 0.72f;
            setParamNorm ("feedback", fbNorm);
            break;
        }
        case DragTarget::mixFloor:
        {
            const float norm = (e.position.x - echo.getX()) / std::max (1.0f, echo.getWidth());
            setParamNorm ("mix", norm);
            break;
        }
        case DragTarget::none:
            break;
    }

    repaint (centerCanvas);
}

void ThrowDelayEditor::mouseUp (const juce::MouseEvent&)
{
    activeDrag = DragTarget::none;
}

void ThrowDelayEditor::timerCallback()
{
    animPhase += 1.0 / (30.0 * 14.0);
    if (animPhase >= 1.0)
        animPhase -= 1.0;

    const auto throwsNow = processor.throwCount();
    const bool newThrow = throwsNow != seenThrows;
    seenThrows = throwsNow;

    const float doorNow = processor.doorLevel();
    const float inNow = processor.inputLevel();
    const float echoNow = processor.echoLevel();

    smoothDoor = smoothDoor * 0.65f + doorNow * 0.35f;
    smoothInput = std::max (inNow, smoothInput * 0.80f);
    smoothEcho = std::max (echoNow, smoothEcho * 0.84f);

    history[(size_t) historyIndex] = {
        juce::jlimit (0.0f, 1.0f, std::sqrt (inNow)),
        doorNow,
        juce::jlimit (0.0f, 1.0f, std::sqrt (echoNow)),
        processor.inPhrase(),
        newThrow
    };
    historyIndex = (historyIndex + 1) % historyLength;

    const auto changesNow = processor.latencyChangeCount();
    if (changesNow != seenLatencyChanges)
    {
        seenLatencyChanges = changesNow;
        hintTicks = 30 * 6;
    }
    else if (hintTicks > 0)
    {
        --hintTicks;
    }

    const auto synced = sync.getToggleState();
    time.slider.setVisible (! synced);
    time.label.setVisible (! synced);
    division.setVisible (synced);
    divisionLabel.setVisible (synced);
    lookahead.setEnabled (! live.getToggleState());

    repaint();
}

void ThrowDelayEditor::paint (juce::Graphics& g)
{
    g.fillAll (accentColour);

    // Soft drifting ivory liquid blobs in the background.
    const float drift1X = 18.0f * (float) std::sin (animPhase * twoPi);
    const float drift1Y = -14.0f * (float) std::sin (animPhase * twoPi);
    const float scale1 = 1.0f + 0.08f * (float) std::sin (animPhase * twoPi);
    drawSoftBlob (g, 220.0f + drift1X, 240.0f + drift1Y, 220.0f * scale1);

    const double phase2 = animPhase + 0.42;
    const float drift2X = 20.0f * (float) std::sin (phase2 * twoPi);
    const float drift2Y = -16.0f * (float) std::cos (phase2 * twoPi);
    const float scale2 = 1.0f + 0.08f * (float) std::cos (phase2 * twoPi);
    drawSoftBlob (g, 740.0f + drift2X, 420.0f + drift2Y, 240.0f * scale2);

    // Translucent ivory panels for header, sidebars, and footer.
    const auto panelFill = ivoryColour.withAlpha (0.58f);
    g.setColour (panelFill);
    g.fillRect (headerArea);
    g.fillRect (footerArea);
    g.fillRect (leftSidebar);
    g.fillRect (rightSidebar);

    // Outer frame and major structural hairlines.
    g.setColour (inkColour);
    g.drawRect (getLocalBounds().toFloat(), ThrowLookAndFeel::hairline);
    drawHairlineH (g, 0, headerArea.getBottom(), getWidth());
    drawHairlineH (g, 0, footerArea.getY(), getWidth());
    drawHairlineV (g, leftSidebar.getRight(), 0, getHeight());
    drawHairlineV (g, rightSidebar.getX(), 0, getHeight());

    // ---------------- HEADER ----------------
    {
        auto brandBox = headerLeft.reduced (16, 0);
        g.setFont (ThrowLookAndFeel::sans (15.0f, true));
        g.setColour (inkColour);
        g.drawText ("T H R O W   D E L A Y", brandBox, juce::Justification::centredLeft);

        auto midBox = headerCenter.reduced (20, 0);
        g.setFont (ThrowLookAndFeel::mono (10.5f));
        if (hintTicks > 0)
        {
            g.setColour (inkColour);
            g.drawText ("LATENZ GEAENDERT - IN ABLETON GGF. STOP/START DRUECKEN", midBox, juce::Justification::centredLeft);
        }
        else
        {
            g.setColour (inkColour);
            g.drawText ("1. ERKENNUNG  >>  2. THROW-FENSTER  >>  3. DELAY", midBox, juce::Justification::centredLeft);
            g.setColour (mutedColour);
            g.drawText ("BEATSEXUELL", midBox, juce::Justification::centredRight);
        }
    }

    // ---------------- LEFT SIDEBAR: 1. PHRASE DETECTOR ----------------
    {
        for (size_t i = 0; i < leftCells.size() - 1; ++i)
            drawHairlineH (g, leftSidebar.getX(), leftCells[i].getBottom(), leftSidebar.getWidth());

        auto hdr = leftCells[0].reduced (16, 10);
        auto topLine = hdr.removeFromTop (16);
        g.setFont (ThrowLookAndFeel::mono (10.5f));
        g.setColour (inkColour);
        g.drawText ("01 / PHRASEN-ERKENNUNG", topLine, juce::Justification::centredLeft);
    }

    // ---------------- RIGHT SIDEBAR: 3. DELAY ENGINE ----------------
    {
        for (size_t i = 0; i < rightCells.size() - 1; ++i)
            drawHairlineH (g, rightSidebar.getX(), rightCells[i].getBottom(), rightSidebar.getWidth());

        auto hdr = rightCells[0].reduced (16, 10);
        auto topLine = hdr.removeFromTop (16);
        g.setFont (ThrowLookAndFeel::mono (10.5f));
        g.setColour (inkColour);
        g.drawText ("03 / ECHO & DELAY", topLine, juce::Justification::centredLeft);
    }

    // ---------------- CENTER: 2. INTERACTIVE PHRASE & THROW VISUALIZER ----------------
    {
        const auto scope = scopePlotBounds();
        const auto echo = echoPlotBounds();

        juce::Rectangle<float> scopeHeader (scope.getX(), (float) centerCanvas.getY() + 12.0f, scope.getWidth(), 18.0f);
        g.setFont (ThrowLookAndFeel::mono (10.5f));
        g.setColour (inkColour);
        g.drawText ("02 / THROW-FENSTER (DIREKT IM GRAPHEN ZIEHEN)", scopeHeader, juce::Justification::centredLeft);

        const juce::String stateLabel = smoothDoor > 0.05f
                                            ? "GATE OFFEN (" + juce::String (juce::roundToInt (smoothDoor * 100.0f)) + " %)"
                                            : (processor.inPhrase() ? "GESANG AKTIV" : "BEREIT");
        g.drawText (stateLabel, scopeHeader, juce::Justification::centredRight);

        // Scope box background
        g.setColour (ivoryColour.withAlpha (0.72f));
        g.fillRect (scope);
        g.setColour (inkColour);
        g.drawRect (scope, ThrowLookAndFeel::hairline);

        const float phraseEndX = scope.getX() + scope.getWidth() * 0.62f;
        const auto winRect = throwWindowRect (scope);
        const float gapX = minGapX (scope);
        const float sensY = sensitivityLineY (scope);

        // 1) Throw Window shaded region (before phrase end)
        const bool winHighlighted = (activeDrag == DragTarget::throwWindow || hoverTarget == DragTarget::throwWindow);
        g.setColour (accentColour.withAlpha (winHighlighted ? 0.85f : (0.55f + 0.35f * smoothDoor)));
        g.fillRect (winRect);
        g.setColour (inkColour);
        g.drawRect (winRect, winHighlighted ? 2.0f : 1.5f);

        // Left grab handle on Throw Window
        g.fillRect (juce::Rectangle<float> (winRect.getX() - 2.0f, winRect.getCentreY() - 14.0f, 5.0f, 28.0f));
        g.setFont (ThrowLookAndFeel::mono (9.5f));
        g.drawText ("<-> THROW: " + processor.parameters.getParameter ("throw")->getCurrentValueAsText(),
                    juce::Rectangle<float> (winRect.getX() + 6.0f, winRect.getY() + 4.0f, 150.0f, 14.0f),
                    juce::Justification::centredLeft, false);

        // 2) Min Gap shaded region (after phrase end)
        const juce::Rectangle<float> gapRect (phraseEndX, scope.getY() + 18.0f, gapX - phraseEndX, scope.getHeight() - 26.0f);
        const bool gapHighlighted = (activeDrag == DragTarget::minGapLine || hoverTarget == DragTarget::minGapLine);
        g.setColour (inkColour.withAlpha (gapHighlighted ? 0.14f : 0.07f));
        g.fillRect (gapRect);
        g.setColour (inkColour);
        g.fillRect (juce::Rectangle<float> (gapX - 1.0f, gapRect.getY(), gapHighlighted ? 3.0f : 1.5f, gapRect.getHeight()));
        g.fillRect (juce::Rectangle<float> (gapX - 2.5f, gapRect.getCentreY() - 14.0f, 5.0f, 28.0f));
        g.drawText ("PAUSE: " + processor.parameters.getParameter ("mingap")->getCurrentValueAsText() + " <->",
                    juce::Rectangle<float> (phraseEndX + 6.0f, gapRect.getBottom() - 18.0f, 150.0f, 14.0f),
                    juce::Justification::centredLeft, false);

        // Schematic Vocal Phrase envelope curve
        {
            juce::Path phraseCurve, phraseFill;
            const float startX = scope.getX() + 12.0f;
            const float baselineY = scope.getBottom() - 8.0f;
            const float peakH = scope.getHeight() * 0.64f;

            phraseCurve.startNewSubPath (startX, baselineY);
            phraseFill.startNewSubPath (startX, baselineY);

            const int steps = 80;
            for (int i = 0; i <= steps; ++i)
            {
                const float u = (float) i / (float) steps;
                const float x = startX + u * (scope.getWidth() - 24.0f);
                float envShape = 0.0f;
                if (x <= phraseEndX)
                {
                    const float p = (x - startX) / std::max (1.0f, phraseEndX - startX);
                    const float attackEnv = std::min (1.0f, p * 6.0f);
                    const float releaseEnv = std::min (1.0f, (1.0f - p) * 9.0f);
                    const float syllables = 0.78f + 0.22f * std::sin (p * 18.0f);
                    envShape = attackEnv * releaseEnv * syllables;
                }
                else
                {
                    const float d = (x - phraseEndX) / 28.0f;
                    envShape = std::exp (-d * 3.5f) * 0.12f;
                }
                const float y = baselineY - envShape * peakH;
                phraseCurve.lineTo (x, y);
                phraseFill.lineTo (x, y);
            }
            phraseFill.lineTo (scope.getRight() - 12.0f, baselineY);
            phraseFill.closeSubPath();

            g.setColour (inkColour.withAlpha (0.14f));
            g.fillPath (phraseFill);
            g.setColour (inkColour);
            g.strokePath (phraseCurve, juce::PathStrokeType (2.0f));
        }

        // Phrase End anchor marker in top margin band of scope
        g.setColour (inkColour);
        g.fillRect (juce::Rectangle<float> (phraseEndX - 1.0f, scope.getY() + 16.0f, 2.0f, scope.getHeight() - 16.0f));
        g.setFont (ThrowLookAndFeel::mono (9.5f));
        g.drawText ("PHRASENENDE",
                    juce::Rectangle<float> (phraseEndX - 50.0f, scope.getY() + 2.0f, 100.0f, 14.0f),
                    juce::Justification::centred, false);

        // 3) Horizontal Sensitivity threshold line (draggable up/down)
        const bool sensHighlighted = (activeDrag == DragTarget::sensitivityLine || hoverTarget == DragTarget::sensitivityLine);
        juce::Path sensPath;
        sensPath.startNewSubPath (scope.getX() + 4.0f, sensY);
        sensPath.lineTo (scope.getRight() - 4.0f, sensY);
        g.setColour (inkColour);
        g.strokePath (sensPath, juce::PathStrokeType (sensHighlighted ? 2.5f : 1.5f));

        auto sensBadge = juce::Rectangle<float> (scope.getX() + 10.0f, sensY - 16.0f, 152.0f, 15.0f);
        g.setColour (inkColour);
        g.fillRect (sensBadge);
        g.setColour (ivoryColour);
        g.setFont (ThrowLookAndFeel::mono (9.5f));
        g.drawText ("SCHWELLWERT: -" + processor.parameters.getParameter ("sensitivity")->getCurrentValueAsText(),
                    sensBadge, juce::Justification::centred, false);

        // ---------------- LOWER CENTER: DELAY ECHO TAPS VISUALIZER ----------------
        juce::Rectangle<float> echoHeader (echo.getX(), echo.getY() - 22.0f, echo.getWidth(), 18.0f);
        g.setFont (ThrowLookAndFeel::mono (10.5f));
        g.setColour (inkColour);
        const char* timeParamId = sync.getToggleState() ? "division" : "time";
        g.drawText ("ECHO-WIEDERHOLUNGEN  /  ABSTAND: "
                        + processor.parameters.getParameter (timeParamId)->getCurrentValueAsText().toUpperCase()
                        + "  /  ABKLINGEN: FEEDBACK",
                    echoHeader, juce::Justification::centredLeft);

        g.setColour (ivoryColour.withAlpha (0.72f));
        g.fillRect (echo);
        g.setColour (inkColour);
        g.drawRect (echo, ThrowLookAndFeel::hairline);

        const float tNorm = getParamNorm (timeParamId);
        const float mixVal = getParamNorm ("mix");
        const float betweenVal = getParamNorm ("between");

        const float floorY = echo.getBottom() - 16.0f;
        const float availH = echo.getHeight() - 28.0f;

        if (betweenVal > 0.01f)
        {
            const float bh = availH * 0.35f * betweenVal;
            const juce::Rectangle<float> betweenBand (echo.getX() + 2.0f, floorY - bh, echo.getWidth() - 4.0f, bh);
            g.setColour (inkColour.withAlpha (0.10f));
            g.fillRect (betweenBand);
            g.setFont (ThrowLookAndFeel::mono (9.0f));
            g.setColour (mutedColour);
            g.drawText ("BETWEEN (DURCHLASS " + juce::String (juce::roundToInt (betweenVal * 100.0f)) + " %)",
                        betweenBand.reduced (8.0f, 2.0f), juce::Justification::bottomRight);
        }

        const float startX = echo.getX() + 44.0f;
        g.setColour (inkColour.withAlpha (0.35f));
        g.fillRect (juce::Rectangle<float> (startX - 3.0f, floorY - availH * 0.85f, 6.0f, availH * 0.85f));
        g.setFont (ThrowLookAndFeel::mono (9.0f));
        g.setColour (mutedColour);
        g.drawText ("WURF", juce::Rectangle<float> (startX - 24.0f, floorY + 2.0f, 48.0f, 12.0f), juce::Justification::centred);

        const float maxSpacing = (echo.getWidth() - 68.0f) / 5.0f;
        const float spacing = 22.0f + tNorm * (maxSpacing - 22.0f);
        const float firstTapH = availH * (0.20f + 0.72f * getParamNorm ("feedback"));

        juce::Path decayCurve;
        decayCurve.startNewSubPath (startX, floorY - availH * 0.85f);

        for (int tap = 1; tap <= 6; ++tap)
        {
            const float tx = startX + (float) tap * spacing;
            if (tx > echo.getRight() - 14.0f)
                break;

            const float decay = std::pow (0.25f + 0.70f * getParamNorm ("feedback"), (float) (tap - 1));
            const float th = firstTapH * decay;
            const float ty = floorY - th;

            decayCurve.lineTo (tx, ty);

            const float barW = std::max (8.0f, spacing * 0.28f);
            const juce::Rectangle<float> tapBar (tx - barW * 0.5f, ty, barW, th);
            g.setColour (accentColour.interpolatedWith (inkColour, 0.25f + 0.50f * mixVal));
            g.fillRect (tapBar);
            g.setColour (inkColour);
            g.drawRect (tapBar, 1.5f);

            g.setFont (ThrowLookAndFeel::mono (9.0f));
            g.drawText (juce::String (tap) + "x",
                        juce::Rectangle<float> (tx - 16.0f, floorY + 2.0f, 32.0f, 12.0f),
                        juce::Justification::centred);

            if (tap == 1)
            {
                const bool headHi = (activeDrag == DragTarget::echoHead || hoverTarget == DragTarget::echoHead);
                const float r = headHi ? 8.5f : 6.5f;
                g.setColour (inkColour);
                g.fillEllipse (tx - r, ty - r, r * 2.0f, r * 2.0f);
                g.setColour (ivoryColour);
                g.drawEllipse (tx - r, ty - r, r * 2.0f, r * 2.0f, 1.5f);
            }
        }

        g.setColour (inkColour.withAlpha (0.45f));
        g.strokePath (decayCurve, juce::PathStrokeType (1.5f));

        g.setColour (inkColour);
        g.fillRect (juce::Rectangle<float> (echo.getX(), floorY, echo.getWidth(), 1.5f));
    }

    // ---------------- FOOTER: LIVE TIMELINE & METERS ----------------
    {
        auto fl = footerLeft.reduced (16, 10);
        g.setFont (ThrowLookAndFeel::mono (10.5f));
        g.setColour (mutedColour);
        const float inDb = smoothInput > 1.0e-4f ? 20.0f * std::log10 (smoothInput) : -60.0f;
        g.drawText (inDb <= -59.0f ? "INPUT  STILL" : "INPUT  " + juce::String (inDb, 1) + " dB",
                    fl.removeFromTop (15), juce::Justification::centredLeft);
        fl.removeFromTop (6);
        auto inBar = fl.removeFromTop (8).toFloat();
        g.setColour (inkColour.withAlpha (0.15f));
        g.fillRect (inBar);
        g.setColour (inkColour);
        g.fillRect (inBar.withWidth (inBar.getWidth() * juce::jlimit (0.0f, 1.0f, std::sqrt (smoothInput))));

        auto fc = footerCenter.reduced (16, 8);
        auto fcTop = fc.removeFromTop (14);
        g.setFont (ThrowLookAndFeel::mono (10.0f));
        g.setColour (mutedColour);
        g.drawText ("LIVE-VERLAUF (6 S)", fcTop, juce::Justification::centredLeft);
        g.setColour (inkColour);
        g.drawText ("WUERFE: " + juce::String (processor.throwCount()), fcTop, juce::Justification::centredRight);

        fc.removeFromTop (4);
        const auto strip = fc.toFloat();
        g.setColour (ivoryColour.withAlpha (0.75f));
        g.fillRect (strip);
        g.setColour (inkColour);
        g.drawRect (strip, ThrowLookAndFeel::hairline);

        const float stepW = strip.getWidth() / (float) historyLength;
        for (int i = 0; i < historyLength; ++i)
        {
            const auto& frame = history[(size_t) ((historyIndex + i) % historyLength)];
            const float x = strip.getX() + (float) i * stepW;

            if (frame.door > 0.02f || frame.triggered)
            {
                g.setColour (accentColour.withAlpha (0.4f + 0.6f * frame.door));
                g.fillRect (juce::Rectangle<float> (x, strip.getY() + 1.0f, std::max (2.0f, stepW + 0.5f), strip.getHeight() - 2.0f));
            }

            if (frame.env > 0.01f)
            {
                const float h = (strip.getHeight() - 4.0f) * frame.env;
                g.setColour (frame.door > 0.1f ? inkColour : inkColour.withAlpha (0.45f));
                g.fillRect (juce::Rectangle<float> (x, strip.getBottom() - 2.0f - h, std::max (1.5f, stepW), h));
            }

            if (frame.triggered)
            {
                g.setColour (inkColour);
                g.fillRect (juce::Rectangle<float> (x, strip.getY(), 2.5f, strip.getHeight()));
            }
        }

        auto fr = footerRight.reduced (16, 10);
        auto frTop = fr.removeFromTop (15);
        g.setFont (ThrowLookAndFeel::mono (10.5f));
        g.setColour (mutedColour);
        const float echoDb = smoothEcho > 1.0e-4f ? 20.0f * std::log10 (smoothEcho) : -60.0f;
        g.drawText (echoDb <= -59.0f ? "ECHO  STILL" : "ECHO  " + juce::String (echoDb, 1) + " dB",
                    frTop, juce::Justification::centredLeft);
        g.drawText ("BPM " + juce::String (juce::roundToInt (processor.currentBpm())),
                    frTop, juce::Justification::centredRight);

        fr.removeFromTop (6);
        auto outBar = fr.removeFromTop (8).toFloat();
        g.setColour (inkColour.withAlpha (0.15f));
        g.fillRect (outBar);
        g.setColour (inkColour);
        g.fillRect (outBar.withWidth (outBar.getWidth() * juce::jlimit (0.0f, 1.0f, std::sqrt (smoothEcho))));
    }
}

void ThrowDelayEditor::resized()
{
    auto area = getLocalBounds();

    headerArea = area.removeFromTop (48);
    footerArea = area.removeFromBottom (64);

    const int leftW = 240;
    const int rightW = 240;

    headerLeft = headerArea.withWidth (leftW);
    headerRight = headerArea.withLeft (headerArea.getRight() - rightW);
    headerCenter = headerArea.withLeft (headerLeft.getRight()).withRight (headerRight.getX());

    footerLeft = footerArea.withWidth (leftW);
    footerRight = footerArea.withLeft (footerArea.getRight() - rightW);
    footerCenter = footerArea.withLeft (footerLeft.getRight()).withRight (footerRight.getX());

    leftSidebar = area.removeFromLeft (leftW);
    rightSidebar = area.removeFromRight (rightW);
    centerCanvas = area;

    {
        auto hr = headerRight;
        const int halfW = hr.getWidth() / 2;
        live.setBounds (hr.removeFromLeft (halfW));
        sync.setBounds (hr);
    }

    const int cellH = leftSidebar.getHeight() / 4;
    for (int i = 0; i < 4; ++i)
    {
        const int y = leftSidebar.getY() + i * cellH;
        const int h = (i == 3) ? (leftSidebar.getBottom() - y) : cellH;
        leftCells[(size_t) i] = juce::Rectangle<int> (leftSidebar.getX(), y, leftSidebar.getWidth(), h);
        rightCells[(size_t) i] = juce::Rectangle<int> (rightSidebar.getX(), y, rightSidebar.getWidth(), h);
    }

    const auto placeSliderCell = [] (Knob& knob, juce::Rectangle<int> cell) {
        auto inner = cell.reduced (16, 16);
        knob.label.setBounds (inner.removeFromTop (16));
        inner.removeFromTop (4);
        knob.slider.setBounds (inner.removeFromTop (48));
    };

    // Left column (1. Phrase Detection): Look-ahead, Sensitivity, Min Gap, Throw Length
    {
        auto c0 = leftCells[0].reduced (16, 10);
        c0.removeFromTop (24);
        lookaheadLabel.setBounds (c0.removeFromTop (16));
        c0.removeFromTop (6);
        lookahead.setBounds (c0.removeFromTop (36));

        placeSliderCell (sensitivity, leftCells[1]);
        placeSliderCell (minGap, leftCells[2]);
        placeSliderCell (throwLength, leftCells[3]);
    }

    // Right column (3. Delay Engine): Time / Division, Feedback, Between, Mix
    {
        auto c0 = rightCells[0].reduced (16, 10);
        c0.removeFromTop (24);
        divisionLabel.setBounds (c0.getX(), c0.getY(), c0.getWidth(), 16);
        division.setBounds (c0.getX(), c0.getY() + 22, c0.getWidth(), 36);

        time.label.setBounds (c0.getX(), c0.getY(), c0.getWidth(), 16);
        time.slider.setBounds (c0.getX(), c0.getY() + 20, c0.getWidth(), 48);

        placeSliderCell (feedback, rightCells[1]);
        placeSliderCell (between, rightCells[2]);
        placeSliderCell (mix, rightCells[3]);
    }
}
