/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Subbass_LUFSEqualizerAudioProcessorEditor::Subbass_LUFSEqualizerAudioProcessorEditor (Subbass_LUFSEqualizerAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p), subBassEqualizerAttachment(audioProcessor.apvts, "Sub Equalizer", subBassEqualizerSlider), saturationSliderAttachment(audioProcessor.apvts, "Saturation Amount", saturationSlider),
	inputGainSliderAttachment(audioProcessor.apvts, "Input Gain", inputGainSlider), outputGainSliderAttachment(audioProcessor.apvts, "Output Gain", outputGainSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    addAndMakeVisible(highCutButton);
    addAndMakeVisible(compressorButton);
    addAndMakeVisible(subBassEqualizerSlider);
    addAndMakeVisible(saturationButton);
    addAndMakeVisible(saturationSlider);
    addAndMakeVisible(inputGainSlider);
    addAndMakeVisible(outputGainSlider);
    addAndMakeVisible(normalizeButton);

    highCutAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ButtonAttachment
    >(
        audioProcessor.apvts,
        "HighCut Enabled",
        highCutButton
    );

    compressorButtonAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ButtonAttachment
    >(
        audioProcessor.apvts,
        "COMP_BYPASS",
        compressorButton
    );

    saturationButtonAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ButtonAttachment
    >(
        audioProcessor.apvts,
        "Saturation Enabled",
        saturationButton
    );

    normalizeButtonAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ButtonAttachment
    >(
        audioProcessor.apvts,
        "Normalize Enabled",
        normalizeButton
    );


    setSize (400, 300);
}

Subbass_LUFSEqualizerAudioProcessorEditor::~Subbass_LUFSEqualizerAudioProcessorEditor()
{
}

//==============================================================================
void Subbass_LUFSEqualizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
  /*  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);*/
}

void Subbass_LUFSEqualizerAudioProcessorEditor::resized()
{
    highCutButton.setBounds(20, 20, 120, 30);
    compressorButton.setBounds(40, 40, 120, 30);
    saturationButton.setBounds(200, 30, 120, 30);
    saturationSlider.setBounds(250, 30, 50, 50);

    highCutButton.setBounds(20, 20, 120, 30);

    normalizeButton.setBounds(20, 200, 50, 50);

    inputGainSlider.setBounds(20, 250, 50, 50);
    outputGainSlider.setBounds(250, 250, 50, 50);


    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    subBassEqualizerSlider.setBounds(bounds);
}
