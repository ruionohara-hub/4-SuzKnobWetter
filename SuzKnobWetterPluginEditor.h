#pragma once

#include <JuceHeader.h>
#include "SuzKnobWetterPluginProcessor.h"

// juce::AudioProcessorValueTreeState::Listener を継承
class SuzKnobWetterAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                           public juce::AudioProcessorValueTreeState::Listener
{
public:
    SuzKnobWetterAudioProcessorEditor(SuzKnobWetterAudioProcessor&);
    ~SuzKnobWetterAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    // パラメータが変更されたときに呼ばれるメソッドを宣言
    void parameterChanged (const juce::String& parameterID, float newValue) override;

private:
    SuzKnobWetterAudioProcessor& processor;

    juce::Slider wetterSlider;
    juce::Label wetterLabel;
    juce::TextButton onOffButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onOffAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SuzKnobWetterAudioProcessorEditor)
};