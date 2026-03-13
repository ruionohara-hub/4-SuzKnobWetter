#include "SuzKnobWetterPluginEditor.h"

//==============================================================================

SuzKnobWetterAudioProcessorEditor::SuzKnobWetterAudioProcessorEditor (SuzKnobWetterAudioProcessor& owner)
    : AudioProcessorEditor (&owner),
      processor (owner)
{
    // Wetterノブとラベルの初期化
    wetterSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    wetterSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    wetterSlider.setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible (wetterSlider);

    wetterLabel.setText ("Wetter", juce::dontSendNotification);
    wetterLabel.setFont (juce::Font (24.0f));
    wetterLabel.setJustificationType (juce::Justification::centred);
    wetterLabel.attachToComponent (&wetterSlider, false);
    addAndMakeVisible (wetterLabel);

    // ON/OFFボタンの初期化
    onOffButton.setClickingTogglesState (true);
    addAndMakeVisible (onOffButton);
    
    // 修正 : 初期状態に基づいてテキストを設定
    bool initialState = *processor.parameters.getRawParameterValue("onoff") > 0.5f;
    onOffButton.setButtonText (initialState ? "ON" : "OFF");

    // パラメータへのアタッチメントを作成
    wetterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.parameters, "wetter", wetterSlider);
    onOffAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.parameters, "onoff", onOffButton);

    // 修正 : パラメータの変更をリッスンするためのリスナーを登録
    processor.parameters.addParameterListener ("onoff", this);

    // ウィンドウサイズの設定
    setSize (300, 450);
}

SuzKnobWetterAudioProcessorEditor::~SuzKnobWetterAudioProcessorEditor()
{
    // リスナーを解除
    processor.parameters.removeParameterListener ("onoff", this);
}

void SuzKnobWetterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // 背景色を設定
    g.fillAll (juce::Colour::fromRGB (40, 40, 45)); 

    // タイトルの描画
    g.setColour (juce::Colours::white.withAlpha (0.9f));
    g.setFont (juce::Font ("Helvetica", 32.0f, juce::Font::bold));
    g.drawFittedText ("SuzKnob\nWetter", getLocalBounds().removeFromTop (100).reduced (10, 0), juce::Justification::centred, 2);

    // 製作者名の描画
    g.setColour (juce::Colours::white.withAlpha (0.4f));
    g.setFont (12.0f);
    g.drawFittedText ("Producer : suzuya", getLocalBounds().removeFromBottom(20).reduced(10, 0), juce::Justification::centredRight, 1);
}

void SuzKnobWetterAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    area.removeFromTop(100); 
    area.removeFromBottom(20);

    auto buttonHeight = 40; 
    auto buttonArea = area.removeFromBottom(buttonHeight + 10); 
    onOffButton.setBounds (buttonArea.reduced (buttonArea.getWidth() * 0.35, 5)); 

    auto knobArea = area.reduced(area.getWidth() * 0.15, 0); 
    wetterLabel.setBounds (knobArea.removeFromTop(30)); 
    wetterSlider.setBounds (knobArea.reduced(20, 0)); 
}

// パラメータ変更時にボタンテキストを更新
void SuzKnobWetterAudioProcessorEditor::parameterChanged (const juce::String& parameterID, float newValue)
{
    // ON/OFF パラメータが変更された場合のみ実行
    if (parameterID == "onoff")
    {
        // 値 (0.0f または 1.0f) に基づいてテキストを設定
        bool isEnabled = newValue > 0.5f; 
        onOffButton.setButtonText (isEnabled ? "ON" : "OFF");
    }
}