/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct ChainSettings
{
	float inputGainInDecibels{ 0.f }, outputGainInDecibels{ 0.f };
	float lowShelfFreq{ 30.f }, lowShelfGainInDecibels{ 11.9f }, lowShelfQuality{ 0.52f };
    float peakFreq0{ 93.f }, peakGainInDecibels0{ -1.1f }, peakQuality0{ 0.19f };
    float peakFreq1{ 51.f }, peakGainInDecibels1{ 0.1f }, peakQuality1{ 0.24f };
    float highCutFreq{ 100.f };
    float saturationAmount{ 0.f };
    bool highCutEnabled{ true };
    bool compressorBypassed{ true };
    bool saturationEnabled{ true };
    bool normalizationEnabled{ true };
	float subEqualizer{ 1.f };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class Subbass_LUFSEqualizerAudioProcessor  : public juce::AudioProcessor
{
public:
    //float sampleRate = 44100.0f;

    float env = 0.0f;
    float gainSmoothed = 1.0f;

    // coefficients
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    // hold (sustain)
    int holdSamples = 0;
    int holdCounter = 0;

    // input gain
    float inputGain = 1.0f;

    juce::AudioBuffer<float> lookaheadBuffer;
    int lookaheadSamples = 0;
    juce::AudioBuffer<float> combinedBuffer;

    float limiterGainSmoothed = 1.0f;

    float inputRampGain = 0.0f;
    bool wasPlaying = false;


    

    //==============================================================================
    Subbass_LUFSEqualizerAudioProcessor();
    ~Subbass_LUFSEqualizerAudioProcessor() override;

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

	static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
	juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout()};

private:

    float smoothedGain = 1.0f;

	using Filter = juce::dsp::IIR::Filter<float>;

	using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

	using MonoChain = juce::dsp::ProcessorChain<Filter, Filter, Filter, CutFilter>;

    MonoChain leftChain, rightChain;

	enum ChainPositions
	{
		LowShelf,
		Peak0,
		Peak1,
		HighCut,
	};

	void applyLookaheadLimiter(juce::AudioBuffer<float>& buffer);

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Subbass_LUFSEqualizerAudioProcessor)
};
