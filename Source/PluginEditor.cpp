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
    setSize (400, 300);
}

Sample_Hold_LFOAudioProcessorEditor::~Sample_Hold_LFOAudioProcessorEditor()
{
}

//==============================================================================
void Sample_Hold_LFOAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto graphArea = getLocalBounds().reduced (40);
    if (graphArea.getWidth() < 2 || graphArea.getHeight() < 2)
        return;

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

    const auto& lfoValues = audioProcessor.getLfoValues();
    if (lfoValues.empty())
        return;

    juce::Path lfoPath;
    const auto drawableWidth = graphRight - graphLeft;
    const auto drawableHeight = graphBottom - graphTop;

    for (std::size_t i = 0; i < lfoValues.size(); ++i)
    {
        const auto value = juce::jlimit (-100, 100, lfoValues[i]);
        const auto fraction = lfoValues.size() == 1
            ? 0.0f
            : static_cast<float> (i) / static_cast<float> (lfoValues.size() - 1);
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

void Sample_Hold_LFOAudioProcessorEditor::resized()
{
        // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}