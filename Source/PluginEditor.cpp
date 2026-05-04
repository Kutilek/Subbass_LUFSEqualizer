/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Subbass_LUFSEqualizerAudioProcessorEditor::Subbass_LUFSEqualizerAudioProcessorEditor (Subbass_LUFSEqualizerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), subBassEqualizerAttachment(audioProcessor.apvts, "Sub Equalizer", subBassEqualizerSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    addAndMakeVisible(highCutButton);
    addAndMakeVisible(subBassEqualizerSlider);


    highCutAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ButtonAttachment
    >(
        audioProcessor.apvts,
        "HighCut Enabled",
        highCutButton
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
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void Subbass_LUFSEqualizerAudioProcessorEditor::resized()
{
    highCutButton.setBounds(20, 20, 120, 30);

    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    subBassEqualizerSlider.setBounds(bounds);
}
