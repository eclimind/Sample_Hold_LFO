/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class Sample_Hold_LFOAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    Sample_Hold_LFOAudioProcessorEditor (Sample_Hold_LFOAudioProcessor&);
    ~Sample_Hold_LFOAudioProcessorEditor() override;

    const std::vector<int>& getLfoValues() const;
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

    void lfoGraph (juce::Graphics& g);
    void lfoConstraints (juce::Graphics& g);
    

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Sample_Hold_LFOAudioProcessor& audioProcessor;

    void updateConstraintControls();
    void updateLockButtons(juce::TextButton& lockButton, bool isLocked);

    //juce::TextButton bipolarButton {"Bipolar"};
    
    juce::Slider ceilingSlider;
    juce::Slider floorSlider;
    
    juce::TextButton lockCeilingButton {"o"};
    juce::TextButton lockFloorButton {"o"};
    //juce::TextButton lockConstraintsButton {"Lock Constraints"};

    //bool bipolarMode = false;
    bool ceilingLocked = false;
    bool floorLocked = false;
    bool updatingControls = false;
    //bool constraintsLocked = false;
    
    double ceilingValue = 100.0;
    double floorValue = 0.0;

    juce::TextButton newStartingSeedButton {"Random Start"};

    juce::Slider smoothingSlider;
    juce::Label smoothingLabel;
    float smoothingAmount = 0.0f;

    juce::Slider speedRate;
    float graphRate = 0.0f;

    juce::Slider axisSlider;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sample_Hold_LFOAudioProcessorEditor)
};
