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
struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox)
    {

    }
};

class Subbass_LUFSEqualizerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Subbass_LUFSEqualizerAudioProcessorEditor (Subbass_LUFSEqualizerAudioProcessor&);
    ~Subbass_LUFSEqualizerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Subbass_LUFSEqualizerAudioProcessor& audioProcessor;

    juce::ToggleButton highCutButton;
    CustomRotarySlider subBassEqualizerSlider;

    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    Attachment subBassEqualizerAttachment;

    std::vector<juce::Component*> getComps();


    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> highCutAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Subbass_LUFSEqualizerAudioProcessorEditor)
};
