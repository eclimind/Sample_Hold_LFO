/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginMath.h"
#include <vector>
#include "SequenceProcessor.h"

//==============================================================================
/**
*/
class Sample_Hold_LFOAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Sample_Hold_LFOAudioProcessor();
    ~Sample_Hold_LFOAudioProcessor() override;

    const std::vector<int>& getLfoValues() const;
    const std::vector<int>& getRawLfoValues() const;

    void setOutputConstraints (bool bipolarMode, int lower, int upper);

    bool isBipolarMode() const { return bipolarMode; }
    int getLowerConstraint() const { return lowerConstraint; }
    int getUpperConstraint() const { return upperConstraint; }

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;


private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sample_Hold_LFOAudioProcessor)
    MainPRNG mainPRNG;
    StartPRNG startPRNG;

    std::vector<int> sequence;
    std::vector<int> rawLfoValues;
    std::vector<int> lfoValues;

    bool bipolarMode = false;
    int lowerConstraint = 0;
    int upperConstraint = 100;

  };
