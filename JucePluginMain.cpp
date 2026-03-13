#include "JuceHeader.h"
#include "SuzKnobWetterPluginProcessor.h"

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SuzKnobWetterAudioProcessor();
}