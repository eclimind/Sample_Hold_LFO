/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ===============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Sample_Hold_LFOAudioProcessorEditor::Sample_Hold_LFOAudioProcessorEditor (Sample_Hold_LFOAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (740, 500);

    addAndMakeVisible(floorSlider);
    floorSlider.setName("Floor");
    floorSlider.setSliderStyle(juce::Slider::LinearVertical);
    floorSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 20);
    floorSlider.setColour(juce::Slider::trackColourId, juce::Colours::lightgrey);
    floorSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    floorSlider.setRange(0.0, 100.0, 1.0);
    floorSlider.setValue(0.0, juce::dontSendNotification);

    addAndMakeVisible(ceilingSlider);
    ceilingSlider.setName("Ceiling");
    ceilingSlider.setSliderStyle(juce::Slider::LinearVertical);
    ceilingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 20);
    ceilingSlider.setColour(juce::Slider::trackColourId, juce::Colours::lightgrey);
    ceilingSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    ceilingSlider.setRange(0.0, 100.0, 1.0);
    ceilingSlider.setValue(100.0, juce::dontSendNotification);

    addAndMakeVisible(lockFloorButton);
    lockFloorButton.setClickingTogglesState(true);
    lockFloorButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    lockFloorButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    lockFloorButton.setColour(juce::TextButton::textColourOnId, juce::Colours::yellow);
    updateLockButtons(lockFloorButton, false);

    addAndMakeVisible(lockCeilingButton);
    lockCeilingButton.setClickingTogglesState(true);
    lockCeilingButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    lockCeilingButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    lockCeilingButton.setColour(juce::TextButton::textColourOnId, juce::Colours::yellow);
    updateLockButtons(lockCeilingButton, false);

    lockFloorButton.onClick = [this] {
        floorLocked = lockFloorButton.getToggleState();

        if (floorLocked)
            floorValue = floorSlider.getValue();

        updateLockButtons(lockFloorButton, floorLocked);
        updateConstraintControls();
    };

    lockCeilingButton.onClick = [this] {
        ceilingLocked = lockCeilingButton.getToggleState();

        if (ceilingLocked)
            ceilingValue = ceilingSlider.getValue();

        updateLockButtons(lockCeilingButton, ceilingLocked);
        updateConstraintControls();
    };

    floorSlider.onValueChange = [this] {
        if (updatingControls)
            return;

        if (floorLocked) {
            floorSlider.setValue(floorValue, juce::dontSendNotification);
            return;
        }

        updateConstraintControls();
    };

    ceilingSlider.onValueChange = [this] {
        if (updatingControls)
            return;

        if (ceilingLocked) {
            ceilingSlider.setValue(ceilingValue, juce::dontSendNotification);
            return;
        }

        updateConstraintControls();
    };
    updateConstraintControls();

    addAndMakeVisible(newStartingSeedButton);
    newStartingSeedButton.onClick = [this] {
        audioProcessor.generateNewStartingSeed();
        repaint();
    };
    newStartingSeedButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);

    addAndMakeVisible(smoothingSlider);
    addAndMakeVisible(smoothingLabel);
    smoothingSlider.setSliderStyle(juce::Slider::Rotary);
    smoothingSlider.setTextBoxStyle (
        juce::Slider::TextBoxBelow,
        false,
        60,
        20);

    smoothingSlider.setRange (0.0, 1.0, 0.01);
    smoothingSlider.setValue (0.0);
    smoothingSlider.setTextValueSuffix (" smoothness");

    smoothingSlider.onValueChange = [this]
    {
        smoothingAmount = static_cast<float> (
            smoothingSlider.getValue());

        repaint();
    };

    smoothingLabel.setText (
        "Smoothness",
        juce::dontSendNotification);

    smoothingLabel.setJustificationType (
        juce::Justification::centred);
    

}

Sample_Hold_LFOAudioProcessorEditor::~Sample_Hold_LFOAudioProcessorEditor()
{
}

//==============================================================================
void Sample_Hold_LFOAudioProcessorEditor::lfoGraph(juce::Graphics& g)
{
    auto graphArea = getLocalBounds().reduced(32, 55);
    graphArea.setRight(getWidth() - 165);

    if (graphArea.getWidth() < 2 || graphArea.getHeight() < 2) {
        return;
    }

    const float graphTop = static_cast<float>(graphArea.getY());
    const float graphBottom = static_cast<float>(graphArea.getBottom() - 1);
    const float graphLeft = static_cast<float>(graphArea.getX());
    const float graphRight = static_cast<float>(graphArea.getRight() - 1);
    const float zeroYAxis = (graphTop + graphBottom) * 0.5f;

    g.setColour(juce::Colours::grey);
    g.drawHorizontalLine(juce::roundToInt(zeroYAxis), graphLeft, graphRight);
    g.drawVerticalLine(graphArea.getX(), graphTop, graphBottom);

    g.setColour(juce::Colours::lightgrey);
    g.drawText("+100", 5, graphArea.getY() - 10, 40, 20, juce::Justification::centredRight);
    g.drawText("0", 20, juce::roundToInt(zeroYAxis - 10), 25, 20, juce::Justification::centredRight);
    g.drawText("-100", 0, graphArea.getBottom() - 10, 45, 20, juce::Justification::centredRight);
    g.drawText("0", graphArea.getX() - 5, graphArea.getBottom() + 5, 30, 20, juce::Justification::centredLeft);
    g.drawText("31", graphArea.getRight() - 30, graphArea.getBottom() + 5, 35, 20, juce::Justification::centredRight);

    const auto& lfoValues = audioProcessor.getRawLfoValues();

    if (lfoValues.empty())
        return;

    juce::Path lfoPath;

    const auto drawableWidth = graphRight - graphLeft;
    const auto drawableHeight = graphBottom - graphTop;

    float previousY = 0.0f;
    for (std::size_t i = 0; i < lfoValues.size(); ++i) {
        const auto value = juce::jlimit(-100, 100, lfoValues[i]);
        const auto fraction = lfoValues.size() == 1 ? 0.0f : static_cast<float>(i) / static_cast<float>(lfoValues.size() - 1);
        const auto x = graphLeft + fraction * drawableWidth;
        const auto y = graphTop + (100.0f - static_cast<float>(value)) * (drawableHeight / 200.0f);

        if (i == 0)
            lfoPath.startNewSubPath(x, y);
        else{
            lfoPath.lineTo(x,previousY);
            lfoPath.lineTo(x, y);
        }
        previousY = y;
    }

    g.saveState();
    g.reduceClipRegion(graphArea);
    g.setColour(juce::Colours::green);
    g.strokePath(lfoPath, juce::PathStrokeType(2.0f));
    g.restoreState();
}

void Sample_Hold_LFOAudioProcessorEditor::lfoConstraints(juce::Graphics& g)
{
    auto constraintArea = juce::Rectangle<int>(getWidth() - 150, 65, 100, getHeight() - 100);

    g.setColour(juce::Colours::darkgrey);
    g.drawRect(constraintArea, 1);

    const auto lineY = [&constraintArea](double value) {
        const auto fraction = value / 100.0;
        return constraintArea.getBottom() - 1 - juce::roundToInt(fraction * (constraintArea.getHeight() - 2));
    };
    const auto getThumbY = [](juce::Slider& slider) {
        const auto sliderBounds = slider.getBounds();
        const auto range = slider.getRange();

        const auto proportion = slider.valueToProportionOfLength(slider.getValue());

        const auto trackTop = sliderBounds.getY() + 10;
        const auto trackBottom = sliderBounds.getBottom() - 30;

        return trackBottom - juce::roundToInt(proportion * (trackBottom - trackTop));
    };

    const auto floorY = getThumbY(floorSlider);
    const auto ceilingY = getThumbY(ceilingSlider);

    g.setColour(floorLocked ? juce::Colours::yellow : juce::Colours::lightgrey);
    g.drawHorizontalLine(floorY, static_cast<float>(constraintArea.getX() + 3), static_cast<float>(constraintArea.getRight() - 3));

    g.setColour(ceilingLocked ? juce::Colours::yellow : juce::Colours::lightgrey);
    g.drawHorizontalLine(ceilingY, static_cast<float>(constraintArea.getX() + 3), static_cast<float>(constraintArea.getRight() - 3));
}

void Sample_Hold_LFOAudioProcessorEditor::updateLockButtons(juce::TextButton& button, bool isLocked)
{
    button.setButtonText(isLocked ? "L" : "U");
}

void Sample_Hold_LFOAudioProcessorEditor::updateConstraintControls()
{
    if (updatingControls)
        return;

    updatingControls = true;

    double floor = floorSlider.getValue();
    double ceiling = ceilingSlider.getValue();

    floor = juce::jlimit(0.0, 100.0, floor);
    ceiling = juce::jlimit(0.0, 100.0, ceiling);

    if (floor > ceiling) {
        if (floorSlider.isMouseButtonDown() && !floorLocked)
            floor = ceiling;
        else
            ceiling = floor;
    }

    if (floorLocked)
        floor = floorValue;

    if (ceilingLocked)
        ceiling = ceilingValue;

    if (floor > ceiling) {
        if (!floorLocked)
            floor = ceiling;
        else if (!ceilingLocked)
            ceiling = floor;
        else {
            floor = juce::jmin(floor, ceiling);
        }
    }

    floorSlider.setValue(floor, juce::dontSendNotification);
    ceilingSlider.setValue(ceiling, juce::dontSendNotification);

    audioProcessor.setOutputConstraints(false, juce::roundToInt(floor), juce::roundToInt(ceiling));

    repaint();

    updatingControls = false;
}

void Sample_Hold_LFOAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    auto panel = getLocalBounds().reduced(18, 18);
    panel.removeFromTop(25);

    g.setColour(juce::Colour(0xff303d43));
    g.fillRoundedRectangle(panel.toFloat(), 8.0f);

    g.setColour(juce::Colour(0xff46545a));
    g.drawRoundedRectangle(panel.toFloat(), 8.0f, 1.0f);

    lfoGraph(g);
    lfoConstraints(g);
}

void Sample_Hold_LFOAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto panel = getLocalBounds().reduced(18, 18);
    panel.removeFromTop(25);

    const int panelPadding = 14;
    const int lockSize = 22;
    const int textboxHeight = 22;
    const int sliderWidth = 34;
    const int sliderGap = 8;

    auto content = panel.reduced(panelPadding);
    auto constraintArea = content.removeFromRight(100);

    content.removeFromRight(12);

    const int lockY = constraintArea.getY() - 5;
    const int sliderTop = lockY + lockSize + 5;
    const int sliderHeight = constraintArea.getHeight() - lockSize - textboxHeight - 10;

    lockCeilingButton.setBounds(constraintArea.getX() + sliderWidth + sliderGap, lockY, lockSize, lockSize);
    ceilingSlider.setBounds(constraintArea.getX() + sliderWidth + sliderGap, sliderTop, sliderWidth, sliderHeight);

    lockFloorButton.setBounds(constraintArea.getX(), lockY, lockSize, lockSize);
    floorSlider.setBounds(constraintArea.getX(), sliderTop, sliderWidth, sliderHeight);

    newStartingSeedButton.setBounds (20, 10, 150, 30);

    smoothingLabel.setBounds (60, 95, 120, 20);
    smoothingSlider.setBounds (200, 50, 120,60);
    //smoothingSlider.setBounds (500, 10, 240,120);



}