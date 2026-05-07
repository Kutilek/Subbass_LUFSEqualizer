/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct ChainSettings
{
    float inputGainInDecibels{ 0.f };
    float outputGainInDecibels{ 0.f };
    float subEqualizer{ 1.f };
    float crunchAmount{ 0.f };
    bool highCutEnabled{ true };
    bool normalizationEnabled{ true };
    bool limiterEnabled{ false };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class Subbass_LUFSEqualizerAudioProcessor  : public juce::AudioProcessor
{
public:

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
    // Hardcoded EQ constants
    static constexpr float LOW_SHELF_FREQ = 30.f;
    static constexpr float LOW_SHELF_GAIN = 15.2f;
    static constexpr float LOW_SHELF_QUALITY = 0.66f;

    static constexpr float PEAK0_FREQ = 51.f;
    static constexpr float PEAK0_GAIN = -0.1f;
    static constexpr float PEAK0_QUALITY = 0.24f;

    static constexpr float PEAK1_FREQ = 93.f;
    static constexpr float PEAK1_GAIN = -0.8f;
    static constexpr float PEAK1_QUALITY = 0.15f;

    static constexpr float HIGH_CUT_FREQ = 100.f;
    static constexpr float SOFT_CEILING_DB = -0.5f;
    static constexpr float HARD_CEILING_DB = -0.01f;
    static constexpr float SATURATION_DRIVE = 5.0f;
    static constexpr float LOOKAHEAD_MS = 5.0f;
    static constexpr float NORMALIZATION_CEILING_DB = -12.0f;

    juce::AudioBuffer<float> lookaheadBuffer;
    int lookaheadSamples = 0;
    juce::AudioBuffer<float> combinedBuffer;

    float limiterGainSmoothed = 1.0f;

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
    void applyEqualizationChain(juce::AudioBuffer<float>& buffer);
    void applySaturation(juce::AudioBuffer<float>& buffer);
    void applyLimiter(juce::AudioBuffer<float>& buffer);

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Subbass_LUFSEqualizerAudioProcessor)
};
