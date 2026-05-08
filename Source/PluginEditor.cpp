/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Subbass_LUFSEqualizerAudioProcessorEditor::Subbass_LUFSEqualizerAudioProcessorEditor(Subbass_LUFSEqualizerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    subBassEqualizerAttachment(audioProcessor.apvts, "Sub Equalizer", subBassEqualizerSlider),
    crunchSliderAttachment(audioProcessor.apvts, "Crunch Amount", crunchSlider),
    inputGainSliderAttachment(audioProcessor.apvts, "Input Gain", inputGainSlider),
    outputGainSliderAttachment(audioProcessor.apvts, "Output Gain", outputGainSlider),
    highCutButtonAttachment(audioProcessor.apvts, "HighCut Enabled", highCutButton),
    limiterButtonAttachment(audioProcessor.apvts, "Limiter Enabled", limiterButton),
    normalizeButtonAttachment(audioProcessor.apvts, "Normalize Enabled", normalizeButton)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(inputGainSlider);
    addAndMakeVisible(normalizeButton);
    addAndMakeVisible(highCutButton);
    addAndMakeVisible(subBassEqualizerSlider);
    addAndMakeVisible(crunchSlider);
    addAndMakeVisible(outputGainSlider);
    addAndMakeVisible(limiterButton);

    addAndMakeVisible(inputGainLabel);
    addAndMakeVisible(subBassEqualizerLabel);
    addAndMakeVisible(crunchLabel);
    addAndMakeVisible(outputGainLabel);

    setSize(600, 300);

    // Initialize labels
    subBassEqualizerLabel.setText("Sub Bass Equalization", juce::dontSendNotification);
    subBassEqualizerLabel.setJustificationType(juce::Justification::centred);

    crunchLabel.setText("CRUNCH!!!", juce::dontSendNotification);
    crunchLabel.setJustificationType(juce::Justification::centred);

    inputGainLabel.setText("Input Gain", juce::dontSendNotification);
    inputGainLabel.setJustificationType(juce::Justification::centred);

    outputGainLabel.setText("Output Gain", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centred);

    highCutButton.onClick = [this]()
        {
            highCutButton.updateToggleColour();
        };

    limiterButton.onClick = [this]()
        {
            limiterButton.updateToggleColour();
        };

    normalizeButton.onClick = [this]()
        {
            normalizeButton.updateToggleColour();
        };

    highCutButton.updateToggleColour();
    limiterButton.updateToggleColour();
    normalizeButton.updateToggleColour();

    // Configure labels (they will be shown under knobs)
    subBassEqualizerLabel.setJustificationType(juce::Justification::centred);
    crunchLabel.setJustificationType(juce::Justification::centred);
    inputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.setJustificationType(juce::Justification::centred);
}

Subbass_LUFSEqualizerAudioProcessorEditor::~Subbass_LUFSEqualizerAudioProcessorEditor()
{
}

//==============================================================================
void Subbass_LUFSEqualizerAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background gradient
    auto bounds = getLocalBounds().toFloat();
    juce::Colour top = juce::Colour::fromRGB(28, 30, 34);
    juce::Colour bottom = juce::Colour::fromRGB(18, 20, 24);
    juce::ColourGradient bgGradient(top, bounds.getX(), bounds.getY(), bottom, bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(bgGradient);
    g.fillRect(bounds);

    // Rounded panel behind controls
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    g.fillRoundedRectangle(bounds.reduced(6.0f), 8.0f);

    // Title
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.drawText("Subbass Equalizer", 12, 8, getWidth() - 24, 28, juce::Justification::centredLeft);
}

void Subbass_LUFSEqualizerAudioProcessorEditor::resized()
{
    // Responsive layout: divide the editor into top, center and bottom regions
    auto area = getLocalBounds().reduced(8);

    const int totalW = area.getWidth();
    const int totalH = area.getHeight();

    // Reserve space for title to avoid overlap
    const int titleH = 36;
    area.removeFromTop(titleH);

    // Top area: all toggle buttons in a single row
    auto topArea = area.removeFromTop((int)(totalH * 0.14f));
    int numToggles = 3; // highCut, saturation toggle, normalize
    int togW = topArea.getWidth() / numToggles;
    // Order: Normalize | High Cut | Saturate
    auto t0 = topArea.removeFromLeft(togW).reduced(6);
    normalizeButton.setBounds(t0);

    auto t1 = topArea.removeFromLeft(togW).reduced(6);
    highCutButton.setBounds(t1);

    auto t2 = topArea.removeFromLeft(togW).reduced(6);
    limiterButton.setBounds(t2);

    // Slider row: InputGain (small slider), SubBass (large slider), Saturation (large slider), OutputGain (small slider)
    auto sliderRow = area.removeFromTop((int)(totalH * 0.58f)).reduced(8);
    const int totalSliderW = sliderRow.getWidth();
    // small width = total/6, large = total/3
    int smallW = totalSliderW / 6;
    int largeW = totalSliderW / 3;
    const int labelH = 18;

    // Start from left: InputGain (small slider)
    juce::Rectangle<int> r_input = sliderRow.removeFromLeft(smallW).reduced(6);
    inputGainSlider.setBounds(r_input);
    inputGainLabel.setBounds(r_input.getX(), r_input.getBottom() + 4, r_input.getWidth(), labelH);

    // Sub Bass (large slider)
    juce::Rectangle<int> r_sub = sliderRow.removeFromLeft(largeW).reduced(6);
    subBassEqualizerSlider.setBounds(r_sub);
    subBassEqualizerLabel.setBounds(r_sub.getX(), r_sub.getBottom() + 4, r_sub.getWidth(), labelH);

    // Saturation (large slider)
    juce::Rectangle<int> r_sat = sliderRow.removeFromLeft(largeW).reduced(6);
    crunchSlider.setBounds(r_sat);
    crunchLabel.setBounds(r_sat.getX(), r_sat.getBottom() + 4, r_sat.getWidth(), labelH);

    // OutputGain (small slider)
    juce::Rectangle<int> r_out = sliderRow.reduced(6);
    outputGainSlider.setBounds(r_out);
    outputGainLabel.setBounds(r_out.getX(), r_out.getBottom() + 4, r_out.getWidth(), labelH);

    // If any component is too small, ensure minimum sizes
    auto ensureMin = [&](juce::Component& c, int minW, int minH) { auto b = c.getBounds(); if (b.getWidth() < minW) b.setWidth(minW); if (b.getHeight() < minH) b.setHeight(minH); c.setBounds(b); };
    ensureMin(inputGainSlider, 40, 40);
    ensureMin(outputGainSlider, 40, 40);
    ensureMin(subBassEqualizerSlider, 80, 80);
    ensureMin(crunchSlider, 80, 80);
}