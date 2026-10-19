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

    bipolarButton.setClickingTogglesState (true);
    bipolarButton.setColour (juce::TextButton::buttonColourId,
                             juce::Colours::grey);
    bipolarButton.setColour (juce::TextButton::textColourOffId,
                             juce::Colours::white);
    bipolarButton.setColour (juce::TextButton::textColourOnId,
                             juce::Colours::white);

    bipolarButton.onClick = [this]
    {
        bipolarMode = bipolarButton.getToggleState();

        bipolarButton.setButtonText (
            bipolarMode ? "Bipolar: On" : "Bipolar: Off");

        bipolarButton.setColour (
            juce::TextButton::buttonColourId,
            bipolarMode ? juce::Colours::green : juce::Colours::grey);

        floorSlider.setRange (
            bipolarMode ? -100.0 : 0.0,
            bipolarMode ? 0.0 : 100.0,
            1.0);

        ceilingSlider.setRange (0.0, 100.0, 1.0);

        if (bipolarMode)
        {
            floorSlider.setValue (
                juce::jlimit (-100.0, 0.0, floorSlider.getValue()),
                juce::dontSendNotification);

            ceilingSlider.setValue (
                juce::jlimit (0.0, 100.0, ceilingSlider.getValue()),
                juce::dontSendNotification);
        }
        else
        {
            floorSlider.setValue (
                juce::jlimit (0.0, 100.0, floorSlider.getValue()),
                juce::dontSendNotification);

            ceilingSlider.setValue (
                juce::jlimit (floorSlider.getValue(), 100.0,
                              ceilingSlider.getValue()),
                juce::dontSendNotification);
        }

        lockFloorButton.setEnabled (!bipolarMode);
        lockConstraintsButton.setEnabled (bipolarMode);

        updateConstraintControls();
    };

    for (auto* slider : { &floorSlider, &ceilingSlider })
    {
        slider->setSliderStyle (juce::Slider::LinearVertical);
        slider->setTextBoxStyle (
            juce::Slider::TextBoxBelow, false, 48, 20);

        slider->setColour (
            juce::Slider::trackColourId, juce::Colours::lightgrey);

        slider->setColour (
            juce::Slider::thumbColourId, juce::Colours::white);

        addAndMakeVisible (*slider);
    }

    floorSlider.setName ("Floor");
    ceilingSlider.setName ("Ceiling");

    floorSlider.setRange (0.0, 100.0, 1.0);
    floorSlider.setValue (0.0, juce::dontSendNotification);

    ceilingSlider.setRange (0.0, 100.0, 1.0);
    ceilingSlider.setValue (100.0, juce::dontSendNotification);

    floorSlider.onValueChange = [this]
    {
        if (floorLocked && !updatingControls)
        {
            floorSlider.setValue (
                floorValue, juce::dontSendNotification);
            return;
        }

        updateConstraintControls();
    };

    ceilingSlider.onValueChange = [this]
    {
        if (ceilingLocked && !updatingControls)
        {
            ceilingSlider.setValue (
                ceilingValue, juce::dontSendNotification);
            return;
        }

        updateConstraintControls();
    };

    for (auto* button : { &lockFloorButton, &lockCeilingButton })
    {
        button->setClickingTogglesState (true);

        button->setColour (
            juce::TextButton::buttonColourId, juce::Colours::darkgrey);

        button->setColour (
            juce::TextButton::textColourOnId, juce::Colours::yellow);

        addAndMakeVisible (*button);
    }

    lockFloorButton.onClick = [this]
    {
        floorLocked = lockFloorButton.getToggleState();
        floorValue = floorSlider.getValue();

        updateLockButtons (lockFloorButton, floorLocked);

        if (constraintsLocked && bipolarMode)
        {
            ceilingLocked = floorLocked;

            lockCeilingButton.setToggleState (
                ceilingLocked, juce::dontSendNotification);

            updateLockButtons (lockCeilingButton, ceilingLocked);
        }
    };

    lockCeilingButton.onClick = [this]
    {
        ceilingLocked = lockCeilingButton.getToggleState();
        ceilingValue = ceilingSlider.getValue();

        updateLockButtons (lockCeilingButton, ceilingLocked);

        if (constraintsLocked && bipolarMode)
        {
            floorLocked = ceilingLocked;

            lockFloorButton.setToggleState (
                floorLocked, juce::dontSendNotification);

            updateLockButtons (lockFloorButton, floorLocked);
        }
    };

    lockConstraintsButton.setClickingTogglesState (true);
    lockConstraintsButton.setColour (
        juce::TextButton::buttonColourId, juce::Colours::grey);

    lockConstraintsButton.onClick = [this]
    {
        constraintsLocked = lockConstraintsButton.getToggleState();

        lockConstraintsButton.setButtonText (
            constraintsLocked ? "Lock Constraints: On"
                               : "Lock Constraints: Off");

        if (constraintsLocked && bipolarMode)
        {
            const auto magnitude = juce::jmin (
                juce::roundToInt (
                    juce::jmax (0.0, -floorSlider.getValue())),
                juce::roundToInt (ceilingSlider.getValue()));

            floorSlider.setValue (
                -magnitude, juce::dontSendNotification);

            ceilingSlider.setValue (
                magnitude, juce::dontSendNotification);

            ceilingLocked = floorLocked;

            lockCeilingButton.setToggleState (
                ceilingLocked, juce::dontSendNotification);

            updateLockButtons (lockCeilingButton, ceilingLocked);
        }

        updateConstraintControls();
    };

    addAndMakeVisible (bipolarButton);
    addAndMakeVisible (lockConstraintsButton);

    lockFloorButton.setBounds (0, 0, 22, 22);
    lockCeilingButton.setBounds (0, 0, 22, 22);

    updateLockButtons (lockFloorButton, false);
    updateLockButtons (lockCeilingButton, false);

    lockFloorButton.setEnabled (false);
    lockConstraintsButton.setEnabled (false);
}

Sample_Hold_LFOAudioProcessorEditor::~Sample_Hold_LFOAudioProcessorEditor()
{
}

//==============================================================================



void Sample_Hold_LFOAudioProcessorEditor::lfoGraph (juce::Graphics& g)
{
    auto graphArea = getLocalBounds().reduced (28,40);
    graphArea.setRight(getWidth() - 150);

    if (graphArea.getWidth() < 2 || graphArea.getHeight() < 2){
        return;}

    const float graphTop = static_cast<float> (graphArea.getY());
    const float graphBottom = static_cast<float> (graphArea.getBottom() - 1);
    const float graphLeft = static_cast<float> (graphArea.getX());
    const float graphRight = static_cast<float> (graphArea.getRight() - 1);
    const float zeroYAxis = (graphTop + graphBottom) * 0.5f;

    g.setColour (juce::Colours::grey);

    g.drawHorizontalLine (juce::roundToInt (zeroYAxis), graphLeft, graphRight);

    g.drawVerticalLine (graphArea.getX(), graphTop, graphBottom);

    g.setColour (juce::Colours::lightgrey);

    g.drawText ("+100", 5, graphArea.getY() - 10, 40, 20, juce::Justification::centredRight);

    g.drawText ("0", 20, juce::roundToInt (zeroYAxis - 10), 25, 20, juce::Justification::centredRight);

    g.drawText ("-100", 0, graphArea.getBottom() - 10, 45, 20, juce::Justification::centredRight);

    g.drawText ("0", graphArea.getX() - 5, graphArea.getBottom() + 5, 30, 20, juce::Justification::centredLeft);

    g.drawText ("31", graphArea.getRight() - 30, graphArea.getBottom() + 5, 35, 20, juce::Justification::centredRight);

    const auto& lfoValues = audioProcessor.getRawLfoValues();

    if (lfoValues.empty())
        return;

    juce::Path lfoPath;

    const auto drawableWidth = graphRight - graphLeft;
    const auto drawableHeight = graphBottom - graphTop;

    for (std::size_t i = 0; i < lfoValues.size(); ++i)
    {
        const auto value = juce::jlimit (-100, 100, lfoValues[i]);

        const auto fraction = lfoValues.size() == 1 ? 0.0f: static_cast<float> (i) / static_cast<float> (lfoValues.size() - 1);

        const auto x = graphLeft + fraction * drawableWidth;

        const auto y = graphTop + (100.0f - static_cast<float> (value)) * (drawableHeight / 200.0f);

        if (i == 0)
            lfoPath.startNewSubPath (x, y);
        else
            lfoPath.lineTo (x, y);
    }

    g.saveState();
    g.reduceClipRegion (graphArea);

    g.setColour (juce::Colours::green);
    g.strokePath (lfoPath, juce::PathStrokeType (2.0f));

    g.restoreState();
}

void Sample_Hold_LFOAudioProcessorEditor::lfoConstraints (juce::Graphics& g)
{
    auto constraintArea = juce::Rectangle<int> (getWidth() - 125, 40, 95, getHeight() - 80);

    g.setColour (juce::Colours::darkgrey);
    g.drawRect (constraintArea, 1);

    const auto lineY = [&constraintArea] (
        double value, double minimum, double maximum)
    {
        const auto fraction = (value - minimum) / (maximum - minimum);

        return constraintArea.getBottom() - 1 - juce::roundToInt (fraction * (constraintArea.getHeight() - 2));
    };

    const auto floorValue = floorSlider.getValue();
    const auto ceilingValue = ceilingSlider.getValue();

    const auto floorMinimum = bipolarMode ? -100.0 : 0.0;
    const auto floorMaximum = 100.0;

    const auto floorY = lineY (floorValue, floorMinimum, floorMaximum);

    const auto ceilingY = lineY (ceilingValue, 0.0, 100.0);

    g.setColour (juce::Colours::lightgrey);

    g.drawHorizontalLine (ceilingY,static_cast<float> (constraintArea.getX() + 3),static_cast<float> (constraintArea.getRight() - 3));

    g.drawHorizontalLine (floorY,static_cast<float> (constraintArea.getX() + 3),static_cast<float> (constraintArea.getRight() - 3));


    g.setColour (ceilingLocked? juce::Colours::yellow: juce::Colours::white);

    g.fillEllipse (static_cast<float> (constraintArea.getRight() - 13),static_cast<float> (ceilingY - 5),10.0f, 10.0f);

    g.setColour (floorLocked? juce::Colours::yellow: juce::Colours::white);

    g.fillEllipse (static_cast<float> (constraintArea.getX() + 3),static_cast<float> (floorY - 5),10.0f, 10.0f);
}

void Sample_Hold_LFOAudioProcessorEditor::updateLockButtons (juce::TextButton& button, bool isLocked)
{
    button.setButtonText (isLocked ? "●" : "○");
}

void Sample_Hold_LFOAudioProcessorEditor::updateConstraintControls()
{
    if (updatingControls)
        return;

    updatingControls = true;

    auto floor = juce::roundToInt (floorSlider.getValue());
    auto ceiling = juce::roundToInt (ceilingSlider.getValue());

    if (bipolarMode && constraintsLocked)
    {
        if (floorSlider.isMouseButtonDown())
        {
            floor = juce::jlimit (-100, 0, floor);
            ceiling = -floor;
        }
        else
        {
            ceiling = juce::jlimit (0, 100, ceiling);
            floor = -ceiling;
        }

        floorSlider.setValue (
            floor, juce::dontSendNotification);

        ceilingSlider.setValue (
            ceiling, juce::dontSendNotification);
    }
    else if (!bipolarMode)
    {
        floor = juce::jlimit (0, 100, floor);
        ceiling = juce::jlimit (0, 100, ceiling);

        if (floor > ceiling)
        {
            if (floorSlider.isMouseButtonDown() && !ceilingLocked)
                ceiling = floor;
            else
                floor = ceiling;
        }

        if (floorLocked)
            floor = juce::roundToInt (floorSlider.getValue());

        if (ceilingLocked)
            ceiling = juce::roundToInt (ceilingSlider.getValue());

        floorSlider.setValue (
            floor, juce::dontSendNotification);

        ceilingSlider.setValue (
            ceiling, juce::dontSendNotification);
    }

    audioProcessor.setOutputConstraints (
        bipolarMode, floor, ceiling);

    repaint();

    updatingControls = false;
}

void Sample_Hold_LFOAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (
        getLookAndFeel().findColour (
            juce::ResizableWindow::backgroundColourId));

    lfoGraph (g);
    lfoConstraints (g);
}


void Sample_Hold_LFOAudioProcessorEditor::resized()
{
        // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}