#include "SuzKnobWetterPluginProcessor.h"
#include "SuzKnobWetterPluginEditor.h"
#include <cmath>

//==============================================================================

SuzKnobWetterAudioProcessor::SuzKnobWetterAudioProcessor()
    #if JucePlugin_Enable_ARA
    : AudioProcessor (BusesProperties().withAnyAudio (true)),
    #else
    : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                      ),
    #endif
    parameters (*this, nullptr, "SuzKnobWetterParameters", createParameterLayout())
{
}

SuzKnobWetterAudioProcessor::~SuzKnobWetterAudioProcessor() {}

//==============================================================================

const juce::String SuzKnobWetterAudioProcessor::getName() const { return JucePlugin_Name; }
bool SuzKnobWetterAudioProcessor::acceptsMidi() const { return false; }
bool SuzKnobWetterAudioProcessor::producesMidi() const { return false; }
bool SuzKnobWetterAudioProcessor::isMidiEffect() const { return false; }
double SuzKnobWetterAudioProcessor::getTailLengthSeconds() const { return 0.0; }

//==============================================================================

juce::AudioProcessorEditor* SuzKnobWetterAudioProcessor::createEditor()
{
    return new SuzKnobWetterAudioProcessorEditor (*this);
}

bool SuzKnobWetterAudioProcessor::hasEditor() const { return true; }

//==============================================================================

bool SuzKnobWetterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    #if JucePlugin_Build_Standalone
    return true;
    #else
    return layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    #endif
}

//==============================================================================

void SuzKnobWetterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // リバーブパラメーターの初期化
    reverbParameters.roomSize   = 0.5f;
    reverbParameters.damping    = 0.5f;
    reverbParameters.width      = 1.0f;
    reverbParameters.wetLevel   = 0.0f;
    reverbParameters.dryLevel   = 1.0f;
    reverbParameters.freezeMode = 0.0f;

    // リバーブのサンプルレートを設定
    reverb.setSampleRate (sampleRate);
}

void SuzKnobWetterAudioProcessor::releaseResources() {}

//==============================================================================

void SuzKnobWetterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // 出力チャンネルが入力チャンネルより多い場合、余分なチャンネルをクリア
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // パラメータ取得
    bool isEnabled = *parameters.getRawParameterValue("onoff");
    float wetterValue = *parameters.getRawParameterValue("wetter"); // 0.0f から 1.0f

    if (isEnabled)
    {
        // OneKnobの値をリバーブパラメータにマッピング（非線形マッピングを適用）
        
        // WetterValueの平方根を取り、ノブの左側でより細かな制御を可能に
        float curveWetter = std::pow(wetterValue, 0.5f); 
        
        // 1. roomSize : 最大値を0.85に制限し、過剰な残響を抑制
        reverbParameters.roomSize   = juce::jmap (curveWetter, 0.0f, 0.85f);
        
        // 2. damping : 0.1f（明るい）から0.7f（暗い）まで、変更なし
        reverbParameters.damping    = juce::jmap (curveWetter, 0.1f, 0.7f);
        
        // 3. wetLevel : 最大値を0.4に制限し、ウェット音量を控えめに
        reverbParameters.wetLevel   = juce::jmap (curveWetter, 0.0f, 0.4f);
        
        // 4. dryLevel : wetLevelが増えても、最小値を0.6に保ち、原音の存在感を確保
        reverbParameters.dryLevel   = juce::jmap (1.0f - wetterValue, 0.6f, 1.0f);
        
        // リバーブパラメータを適用
        reverb.setParameters (reverbParameters);
        
        // リバーブを適用 (チャンネル数に応じて処理を分岐)
        if (buffer.getNumChannels() >= 2)
            reverb.processStereo (buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
        else if (buffer.getNumChannels() == 1)
            reverb.processMono (buffer.getWritePointer(0), buffer.getNumSamples());
    }
    else
    {
        // OFFの場合、入力をそのまま出力
    }
}

//==============================================================================

int SuzKnobWetterAudioProcessor::getNumPrograms() { return 1; }
int SuzKnobWetterAudioProcessor::getCurrentProgram() { return 0; }
void SuzKnobWetterAudioProcessor::setCurrentProgram (int index) { juce::ignoreUnused (index); }
const juce::String SuzKnobWetterAudioProcessor::getProgramName (int index) { juce::ignoreUnused (index); return {}; }
void SuzKnobWetterAudioProcessor::changeProgramName (int index, const juce::String& newName) { juce::ignoreUnused (index, newName); }

//==============================================================================

void SuzKnobWetterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    parameters.state.writeToStream(stream);
}

void SuzKnobWetterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
        parameters.state = tree;
}

//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout SuzKnobWetterAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Wetterノブ用のパラメータ
    layout.add (std::make_unique<juce::AudioParameterFloat> ("wetter", "Wetter", 0.0f, 1.0f, 0.5f));
    
    // ON/OFFボタン用パラメータ
    layout.add (std::make_unique<juce::AudioParameterBool> ("onoff", "On/Off", true));

    return layout;
}