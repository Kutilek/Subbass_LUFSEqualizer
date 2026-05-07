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

struct CustomToggleButton : juce::TextButton
{
    CustomToggleButton(const juce::String& buttonText)
        : juce::TextButton(buttonText)
    {
        setClickingTogglesState(true);

        // Text colours
        setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        setColour(juce::TextButton::textColourOffId, juce::Colours::black);

        // Start in OFF appearance
        updateToggleColour();
    }

    void updateToggleColour()
    {
        if (getToggleState()) // EFFECT ENABLED
        {
            // ON = brighter
            setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(255, 140, 40));
        }
        else // EFFECT DISABLED
        {
            // OFF = darker
            setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(55, 55, 55));
        }

        repaint();
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

    using APVTS = juce::AudioProcessorValueTreeState;
    
    // Sliders
    using SliderAttachment = APVTS::SliderAttachment;

    CustomRotarySlider subBassEqualizerSlider;
    CustomRotarySlider saturationSlider;
    CustomRotarySlider inputGainSlider;
    CustomRotarySlider outputGainSlider;

    SliderAttachment subBassEqualizerAttachment;
    SliderAttachment saturationSliderAttachment;
    SliderAttachment inputGainSliderAttachment;
    SliderAttachment outputGainSliderAttachment;

    // Toggles
	using ButtonAttachment = APVTS::ButtonAttachment;

    CustomToggleButton highCutButton{ "High Cut" };
    CustomToggleButton saturationButton{ "Saturate" };
    CustomToggleButton normalizeButton{ "Normalize" };

    ButtonAttachment highCutButtonAttachment;
    ButtonAttachment saturationButtonAttachment;
    ButtonAttachment normalizeButtonAttachment;

    // Labels
    juce::Label subBassEqualizerLabel;
    juce::Label saturationLabel;
    juce::Label inputGainLabel;
    juce::Label outputGainLabel;
    juce::Label highCutLabel;
    juce::Label saturationButtonLabel;
    juce::Label normalizeLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Subbass_LUFSEqualizerAudioProcessorEditor)
};
