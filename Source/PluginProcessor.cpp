/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

/*this portion of code is responsible for constraining the seed of the prng by declaring the initial seed, and performing a multiplication modulus operation by grabbing the current time of execution times seed modulus by a specific value*/

namespace {
    constexpr std::uint32_t mainPRNGBaseSeed = 293;
    constexpr std::uint64_t seedModulus = 4294967296ULL;

    std::uint32_t createTimeSeed()
    {
        const auto currentTime =
            static_cast<std::uint64_t> (juce::Time::currentTimeMillis());

        return static_cast<std::uint32_t>((static_cast<std::uint64_t> (mainPRNGBaseSeed) * currentTime) % seedModulus);
    }
}

//==============================================================================
Sample_Hold_LFOAudioProcessor::Sample_Hold_LFOAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                    mainPRNG(createTimeSeed()),
                    startPRNG(239)
#endif
{
    sequence.resize(500);
    for(int i = 0;i<sequence.size();i++){
        sequence[i] = mainPRNG.next();
    }

    sequenceProcessor(sequence, startPRNG, rawLfoValues);

    for (std::size_t start = 0; start < sequence.size(); ++start) {
        bool matchesWindow = true;

        for (std::size_t i = 0; i < rawLfoValues.size(); ++i) {
            if (sequence[(start + i) % sequence.size()] != rawLfoValues[i]) {
                matchesWindow = false;
                break;
            }
        }

        if (matchesWindow){
            nextSequenceIndex = (start + rawLfoValues.size()) % sequence.size();
            break;
        }
    }

    setOutputConstraints(false, 0, 100);
}

Sample_Hold_LFOAudioProcessor::~Sample_Hold_LFOAudioProcessor()
{
}

const std::vector<int>& Sample_Hold_LFOAudioProcessor::getLfoValues() const
{
    const juce::ScopedLock lock(sequenceLock);
    return lfoValues;
}

const std::vector<int>& Sample_Hold_LFOAudioProcessor::getRawLfoValues() const
{
    const juce::ScopedLock lock(sequenceLock);
    return rawLfoValues;
}

void Sample_Hold_LFOAudioProcessor::setConstantRate(double newRate)
{
    const juce::ScopedLock lock(sequenceLock);

    constantRate = juce::jmax(0.001, newRate);
    samplesUntilNextStep = 0.0;
}

void Sample_Hold_LFOAudioProcessor::generateNewStartingSeed()
{
    sequenceProcessor(sequence, startPRNG, rawLfoValues);
    setOutputConstraints(bipolarMode,lowerConstraint,upperConstraint);

}

//==============================================================================
const juce::String Sample_Hold_LFOAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

void Sample_Hold_LFOAudioProcessor::setOutputConstraints (
    bool bipolarModeIn, int lower, int upper)
{
    bipolarMode = bipolarModeIn;

    lowerConstraint = juce::jlimit (
        bipolarMode ? -100 : 0, 100, lower);

    upperConstraint = juce::jlimit (
        bipolarMode ? 0 : lowerConstraint, 100, upper);

    lfoValues.resize (rawLfoValues.size());

    for (std::size_t i = 0; i < lfoValues.size(); ++i)
    {
        const auto rawValue =
            juce::jlimit (-100, 100, rawLfoValues[i]);

        const auto mappedValue = bipolarMode
            ? rawValue
            : (rawValue + 100) / 2;

        lfoValues[i] = juce::jlimit (
            lowerConstraint, upperConstraint, mappedValue);
    }
}

bool Sample_Hold_LFOAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Sample_Hold_LFOAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Sample_Hold_LFOAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Sample_Hold_LFOAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Sample_Hold_LFOAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Sample_Hold_LFOAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Sample_Hold_LFOAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Sample_Hold_LFOAudioProcessor::getProgramName (int index)
{
    return {};
}

void Sample_Hold_LFOAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Sample_Hold_LFOAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    const juce::ScopedLock lock(sequenceLock);

    currentSampleRate = sampleRate;
    samplesUntilNextStep = 0.0;
}

void Sample_Hold_LFOAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Sample_Hold_LFOAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Sample_Hold_LFOAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
    {
        const juce::ScopedLock lock(sequenceLock);
        const double samplesPerStep = currentSampleRate / constantRate;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            samplesUntilNextStep += 1.0;

            if (samplesUntilNextStep >= samplesPerStep && !sequence.empty() && !rawLfoValues.empty())
            {
                samplesUntilNextStep -= samplesPerStep;

                const int nextValue = sequence[nextSequenceIndex % sequence.size()];
                nextSequenceIndex = (nextSequenceIndex + 1) % sequence.size();
                rawLfoValues.erase(rawLfoValues.begin());
                rawLfoValues.push_back(nextValue);

                const int mappedValue = bipolarMode ? nextValue : (nextValue + 100) / 2;
                const int constrainedValue = juce::jlimit(lowerConstraint, upperConstraint, mappedValue);

                if (!lfoValues.empty())
                {
                    lfoValues.erase(lfoValues.begin());
                    lfoValues.push_back(constrainedValue);
                }

                //plugin parameter/controller output connection
                // const float normalizedOutput = static_cast<float>(constrainedValue) / 100.0f;
                //send normalizedOutput to the selected plugin parameter here.
                //keep disabled for now; this updates the sequence/graph only.
            }
        }
    }
    juce::ignoreUnused(midiMessages);
}

//==============================================================================
bool Sample_Hold_LFOAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Sample_Hold_LFOAudioProcessor::createEditor()
{
    return new Sample_Hold_LFOAudioProcessorEditor (*this);
}

//==============================================================================
void Sample_Hold_LFOAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Sample_Hold_LFOAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Sample_Hold_LFOAudioProcessor();
}
