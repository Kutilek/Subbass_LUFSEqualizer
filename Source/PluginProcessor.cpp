/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Subbass_LUFSEqualizerAudioProcessor::Subbass_LUFSEqualizerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

Subbass_LUFSEqualizerAudioProcessor::~Subbass_LUFSEqualizerAudioProcessor()
{
}

//==============================================================================
const juce::String Subbass_LUFSEqualizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Subbass_LUFSEqualizerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Subbass_LUFSEqualizerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Subbass_LUFSEqualizerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Subbass_LUFSEqualizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Subbass_LUFSEqualizerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Subbass_LUFSEqualizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Subbass_LUFSEqualizerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Subbass_LUFSEqualizerAudioProcessor::getProgramName (int index)
{
    return {};
}

void Subbass_LUFSEqualizerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Subbass_LUFSEqualizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

    // This is my initialization, this should be here
    juce::dsp::ProcessSpec spec;

    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

	auto chainSettings = getChainSettings(apvts);

    // Calculate lookahead samples once
    lookaheadSamples = static_cast<int>(LOOKAHEAD_MS * 0.001f * sampleRate);

    // Pre-allocate lookahead buffer - do this in prepareToPlay, not processBlock
    lookaheadBuffer.setSize(2, lookaheadSamples);
    lookaheadBuffer.clear();
    combinedBuffer.setSize(2, samplesPerBlock + lookaheadSamples);
    combinedBuffer.clear();

    // ... rest of your prepareToPlay code

    leftChain.setBypassed<ChainPositions::HighCut>(!chainSettings.highCutEnabled);
    rightChain.setBypassed<ChainPositions::HighCut>(!chainSettings.highCutEnabled);

    float lowShelfEqualize = juce::jmap(chainSettings.subEqualizer, 0.0f, 1.0f, .0f, LOW_SHELF_GAIN);
    float peak0Equalize = juce::jmap(chainSettings.subEqualizer, 0.0f, 1.0f, .0f, PEAK0_GAIN);
    float peak1Equalize = juce::jmap(chainSettings.subEqualizer, 0.0f, 1.0f, .0f, PEAK1_GAIN);

    auto lowShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), LOW_SHELF_FREQ, LOW_SHELF_QUALITY, juce::Decibels::decibelsToGain(lowShelfEqualize));
    auto peak0Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), PEAK0_FREQ, PEAK0_QUALITY, juce::Decibels::decibelsToGain(peak0Equalize));
    auto peak1Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), PEAK1_FREQ, PEAK1_QUALITY, juce::Decibels::decibelsToGain(peak1Equalize));
    auto highCut = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(100.f, sampleRate, 8);

	*leftChain.get<ChainPositions::LowShelf>().coefficients = *lowShelfCoefficients;
    *leftChain.get<ChainPositions::Peak0>().coefficients = *peak0Coefficients;
    *leftChain.get<ChainPositions::Peak1>().coefficients = *peak1Coefficients;

    *rightChain.get<ChainPositions::LowShelf>().coefficients = *lowShelfCoefficients;
    *rightChain.get<ChainPositions::Peak0>().coefficients = *peak0Coefficients;
    *rightChain.get<ChainPositions::Peak1>().coefficients = *peak1Coefficients;

    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();

    *leftHighCut.get<0>().coefficients = *highCut[0];
    *leftHighCut.get<1>().coefficients = *highCut[1];
    *leftHighCut.get<2>().coefficients = *highCut[2];
    *leftHighCut.get<3>().coefficients = *highCut[3];

    *rightHighCut.get<0>().coefficients = *highCut[0];
    *rightHighCut.get<1>().coefficients = *highCut[1];
    *rightHighCut.get<2>().coefficients = *highCut[2];
    *rightHighCut.get<3>().coefficients = *highCut[3];
}

void Subbass_LUFSEqualizerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Subbass_LUFSEqualizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

// Lookahead peak limiter: normalizes signal to -12dBFS with zero transient distortion
void Subbass_LUFSEqualizerAudioProcessor::applyLookaheadLimiter(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    const float ceiling = juce::Decibels::decibelsToGain(NORMALIZATION_CEILING_DB);

    // Build combined buffer: [lookahead history | current buffer]
    for (int ch = 0; ch < numChannels; ++ch)
    {
        combinedBuffer.copyFrom(ch, 0, lookaheadBuffer, ch, 0, lookaheadSamples);
        combinedBuffer.copyFrom(ch, lookaheadSamples, buffer, ch, 0, numSamples);
    }

    // Process each sample
    for (int i = 0; i < numSamples; ++i)
    {
        float peak = 0.0f;
        for (int j = 0; j < lookaheadSamples; ++j)
        {
            for (int ch = 0; ch < numChannels; ++ch)
                peak = std::max(peak, std::abs(combinedBuffer.getSample(ch, i + j)));
        }

        float targetGain = (peak > ceiling && peak > 1e-6f) ? (ceiling / peak) : 1.0f;

        // ---- THIS IS THE NEW PART ----
        // Attack faster than release to catch peaks before they happen
        if (targetGain < limiterGainSmoothed)
            limiterGainSmoothed = limiterGainSmoothed * 0.999f + targetGain * 0.001f;
        else
            limiterGainSmoothed = limiterGainSmoothed * 0.9999f + targetGain * 0.0001f;
        // ---- END NEW PART ----

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float sample = combinedBuffer.getSample(ch, i);
            buffer.setSample(ch, i, sample * limiterGainSmoothed);
        }
    }

    for (int ch = 0; ch < numChannels; ++ch)
        lookaheadBuffer.copyFrom(ch, 0, combinedBuffer, ch, numSamples, lookaheadSamples);
}

void Subbass_LUFSEqualizerAudioProcessor::applyEqualizationChain(juce::AudioBuffer<float>& buffer)
{

}

void Subbass_LUFSEqualizerAudioProcessor::applySaturation(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    auto chainSettings = getChainSettings(apvts);

    const float gainAmount = juce::Decibels::decibelsToGain(chainSettings.crunchAmount);
    const float softCeiling = juce::Decibels::decibelsToGain(SOFT_CEILING_DB);
    const float hardCeiling = juce::Decibels::decibelsToGain(HARD_CEILING_DB);
    const float fixedDrive = SATURATION_DRIVE + gainAmount * 0.1f;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            data[i] *= gainAmount;

            if (std::abs(data[i]) > softCeiling)
            {
                float saturated = std::tanh(data[i] * fixedDrive) / std::tanh(fixedDrive);
                float blend = gainAmount / 10.0f;
                data[i] = data[i] * (1.0f - blend) + saturated * blend;
            }

          //  data[i] = juce::jlimit(-hardCeiling, hardCeiling, data[i]);
        }
    }
}

void Subbass_LUFSEqualizerAudioProcessor::applyLimiter(juce::AudioBuffer<float>& buffer)
{
    const float ceiling = juce::Decibels::decibelsToGain(HARD_CEILING_DB);

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            data[i] = juce::jlimit(-ceiling, ceiling, data[i]);
        }
    }
}

void Subbass_LUFSEqualizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // This up here it should be here it is needed here
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

    //Get Chain Settings
    auto chainSettings = getChainSettings(apvts);


	/// 0. Apply input gain
    float inputGain = juce::Decibels::decibelsToGain(chainSettings.inputGainInDecibels);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.applyGain(ch, 0, buffer.getNumSamples(), inputGain);


    /// 1. Apply lookahead limiter to normalize to -12dBFS
    if (chainSettings.normalizationEnabled)
        applyLookaheadLimiter(buffer);


    /// 2. Process through DSP chain (EQ Filters)
    // SubEqualizer Curve
    float lowShelfEqualize = juce::jmap(chainSettings.subEqualizer, 0.0f, 1.0f, .0f, LOW_SHELF_GAIN);
    float peak0Equalize = juce::jmap(chainSettings.subEqualizer, 0.0f, 1.0f, .0f, PEAK0_GAIN);
    float peak1Equalize = juce::jmap(chainSettings.subEqualizer, 0.0f, 1.0f, .0f, PEAK1_GAIN);

    leftChain.setBypassed<ChainPositions::HighCut>(!chainSettings.highCutEnabled);
    rightChain.setBypassed<ChainPositions::HighCut>(!chainSettings.highCutEnabled);

    auto lowShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), LOW_SHELF_FREQ, LOW_SHELF_QUALITY, juce::Decibels::decibelsToGain(lowShelfEqualize));
    auto peak0Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), PEAK0_FREQ, PEAK0_QUALITY, juce::Decibels::decibelsToGain(peak0Equalize));
    auto peak1Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), PEAK1_FREQ, PEAK1_QUALITY, juce::Decibels::decibelsToGain(peak1Equalize));
    auto highCut = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(HIGH_CUT_FREQ, getSampleRate(), 8);

    *leftChain.get<ChainPositions::LowShelf>().coefficients = *lowShelfCoefficients;
    *leftChain.get<ChainPositions::Peak0>().coefficients = *peak0Coefficients;
    *leftChain.get<ChainPositions::Peak1>().coefficients = *peak1Coefficients;

    *rightChain.get<ChainPositions::LowShelf>().coefficients = *lowShelfCoefficients;
    *rightChain.get<ChainPositions::Peak0>().coefficients = *peak0Coefficients;
    *rightChain.get<ChainPositions::Peak1>().coefficients = *peak1Coefficients;

    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();

    *leftHighCut.get<0>().coefficients = *highCut[0];
    *leftHighCut.get<1>().coefficients = *highCut[1];
    *leftHighCut.get<2>().coefficients = *highCut[2];
    *leftHighCut.get<3>().coefficients = *highCut[3];

    *rightHighCut.get<0>().coefficients = *highCut[0];
    *rightHighCut.get<1>().coefficients = *highCut[1];
    *rightHighCut.get<2>().coefficients = *highCut[2];
    *rightHighCut.get<3>().coefficients = *highCut[3];   

    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);


    /// 4. Saturation
    if (chainSettings.crunchAmount > 0.01f)
        applySaturation(buffer);


	/// 5. Limiter
    if (chainSettings.limiterEnabled)
		applyLimiter(buffer);


    /// 6. Output gain
    float knobOutputGain = juce::Decibels::decibelsToGain(chainSettings.outputGainInDecibels);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.applyGain(ch, 0, buffer.getNumSamples(), knobOutputGain);
}

//==============================================================================
bool Subbass_LUFSEqualizerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Subbass_LUFSEqualizerAudioProcessor::createEditor()
{
    return new Subbass_LUFSEqualizerAudioProcessorEditor (*this);
}

//==============================================================================
void Subbass_LUFSEqualizerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void Subbass_LUFSEqualizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
        //updateFilters();
    }
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    settings.inputGainInDecibels = apvts.getRawParameterValue("Input Gain")->load();

    settings.normalizationEnabled = apvts.getRawParameterValue("Normalize Enabled")->load();

    settings.subEqualizer = apvts.getRawParameterValue("Sub Equalizer")->load();
    settings.highCutEnabled = apvts.getRawParameterValue("HighCut Enabled")->load();

    settings.crunchAmount = apvts.getRawParameterValue("Crunch Amount")->load();
    settings.limiterEnabled = apvts.getRawParameterValue("Limiter Enabled")->load();
     
    settings.outputGainInDecibels = apvts.getRawParameterValue("Output Gain")->load();

    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout Subbass_LUFSEqualizerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Elements added in a way how they are processed in the chain
    layout.add(std::make_unique<juce::AudioParameterFloat>("Input Gain", "Input Gain", juce::NormalisableRange<float>(-60.f, 20.f, 0.1f, 0.5f), 0.f));
    
    layout.add(std::make_unique<juce::AudioParameterBool>("Normalize Enabled", "Normalize Enabled", true));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Sub Equalizer", "Sub Equalizer", juce::NormalisableRange<float>(0.0f, 1.0f), 1.f));
    layout.add(std::make_unique<juce::AudioParameterBool>("HighCut Enabled", "HighCut Enabled", true));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Crunch Amount", "Crunch Amount", juce::NormalisableRange<float>(0.0f, 20.0f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterBool>("Limiter Enabled", "Limiter Enabled", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Output Gain", "Output Gain", juce::NormalisableRange<float>(-60.f, 20.f, 0.1f, 0.5f), 0.f));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Subbass_LUFSEqualizerAudioProcessor();
}
