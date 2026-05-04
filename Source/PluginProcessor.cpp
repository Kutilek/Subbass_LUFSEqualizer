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
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;

    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

	auto chainSettings = getChainSettings(apvts);

    leftChain.setBypassed<ChainPositions::HighCut>(!chainSettings.highCutEnabled);
    rightChain.setBypassed<ChainPositions::HighCut>(!chainSettings.highCutEnabled);

	auto lowShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, chainSettings.lowShelfFreq, chainSettings.lowShelfQuality, juce::Decibels::decibelsToGain(chainSettings.lowShelfGainInDecibels));
	auto peak0Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peakFreq0, chainSettings.peakQuality0, juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels0));
	auto peak1Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peakFreq1, chainSettings.peakQuality1, juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels1));
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

void Subbass_LUFSEqualizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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


    auto chainSettings = getChainSettings(apvts);

    

    // Example mapping (you define the curve)
    float lowShelfEqualize = juce::jmap(chainSettings.subEqualizer, 0.0f, 1.0f, .0f, 11.9f);
    float peak0Equalize = juce::jmap(chainSettings.subEqualizer, 0.0f, 1.0f, .0f, -1.1f);
    float peak1Equalize = juce::jmap(chainSettings.subEqualizer, 0.0f, 1.0f, .0f, 0.1f);

    leftChain.setBypassed<ChainPositions::HighCut>(!chainSettings.highCutEnabled);
    rightChain.setBypassed<ChainPositions::HighCut>(!chainSettings.highCutEnabled);

    auto lowShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), chainSettings.lowShelfFreq, chainSettings.lowShelfQuality, juce::Decibels::decibelsToGain(lowShelfEqualize));
    auto peak0Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peakFreq0, chainSettings.peakQuality0, juce::Decibels::decibelsToGain(peak0Equalize));
    auto peak1Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peakFreq1, chainSettings.peakQuality1, juce::Decibels::decibelsToGain(peak1Equalize));
    auto highCut = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(100.f, getSampleRate(), 8);

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
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Subbass_LUFSEqualizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    settings.lowShelfFreq = apvts.getRawParameterValue("LowShelf Freq")->load();
    settings.lowShelfGainInDecibels = apvts.getRawParameterValue("LowShelf Gain")->load();
    settings.lowShelfQuality = apvts.getRawParameterValue("LowShelf Quality")->load();

    settings.peakFreq0 = apvts.getRawParameterValue("Peak0 Freq")->load();
    settings.peakGainInDecibels0 = apvts.getRawParameterValue("Peak0 Gain")->load();
    settings.peakQuality0 = apvts.getRawParameterValue("Peak0 Quality")->load();

    settings.peakFreq1 = apvts.getRawParameterValue("Peak1 Freq")->load();
    settings.peakGainInDecibels1 = apvts.getRawParameterValue("Peak1 Gain")->load();
    settings.peakQuality1 = apvts.getRawParameterValue("Peak1 Quality")->load();

    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.highCutEnabled = apvts.getRawParameterValue("HighCut Enabled")->load();

    settings.subEqualizer = apvts.getRawParameterValue("Sub Equalizer")->load();
    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout Subbass_LUFSEqualizerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("LowShelf Freq", "LowShelf Freq", juce::NormalisableRange<float>(20.f, 500.f, 1.f, 0.5f), 30.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowShelf Gain", "LowShelf Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.1f, 1.f), 11.9f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowShelf Quality", "LowShelf Quality", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.f), 0.52f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak0 Freq", "Peak0 Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.5f), 93.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak0 Gain", "Peak0 Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.1f, 1.f), -1.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak0 Quality", "Peak0 Quality", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.f), 0.19f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak1 Freq", "Peak1 Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.5f), 51.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak1 Gain", "Peak1 Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.1f, 1.f), 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak1 Quality", "Peak1 Quality", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.f), 0.24f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq", "HighCut Freq", juce::NormalisableRange<float>(100.f, 20000.f, 1.f, 0.5f), 100.f));
    layout.add(std::make_unique<juce::AudioParameterBool>("HighCut Enabled", "HighCut Enabled", true));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Sub Equalizer", "Sub Equalizer", juce::NormalisableRange<float>(0.0f, 1.0f),1.f));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Subbass_LUFSEqualizerAudioProcessor();
}
