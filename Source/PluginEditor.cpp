/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
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

    const float zeroYAxis = graphArea.getY() + graphArea.getHeight() / 2.0f;
    const float xAxis = graphArea.getBottom();
    const float yAxis = graphArea.getX();

    g.setColour (juce::Colours::grey);

    g.drawHorizontalLine (static_cast<int>(zeroYAxis), static_cast<float>(graphArea.getX()), static_cast<float>(graphArea.getRight()));

    g.drawVerticalLine (static_cast<int>(yAxis),static_cast<float>(graphArea.getY()),static_cast<float>(graphArea.getBottom()));


    g.setColour (juce::Colours::lightgrey);
    g.drawText ( "+100", 5, graphArea.getY() - 10, 40, 20, juce::Justification::centredRight);
    g.drawText ( "0", 20, static_cast<int>(zeroYAxis - 10), 25, 20,juce::Justification::centredRight);
    g.drawText ( "-100", 0, graphArea.getBottom() - 10, 45, 20, juce::Justification::centredRight);

    g.drawText ( "0", graphArea.getX() - 5, graphArea.getBottom() + 5, 30, 20, juce::Justification::centredLeft);\
    g.drawText ( "255", graphArea.getRight() - 30, graphArea.getBottom() + 5, 35, 20, juce::Justification::centredRight);

    const auto& lfoValues = audioProcessor.getLfoValues();

    if (lfoValues.empty())
        return;

    g.setColour (juce::Colours::green);

    juce::Path lfoPath;

    for (int i = 0; i < lfoValues.size(); i++){
        const float x = graphArea.getX()+ (static_cast<float>(i) / static_cast<float>(lfoValues.size() - 1)) * graphArea.getWidth();
        const float y = zeroYAxis - (static_cast<float>(lfoValues[i]) / 100.0f) * (graphArea.getHeight() / 2.0f);

        if (i == 0)
            lfoPath.startNewSubPath (x, y);
        else
            lfoPath.lineTo (x, y);
    }

    g.strokePath (lfoPath,juce::PathStrokeType (2.0f));

}

void Sample_Hold_LFOAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
