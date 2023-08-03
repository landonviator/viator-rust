#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "components/Header.h"

//==============================================================================
/**
*/
class ViatorrustAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ViatorrustAudioProcessorEditor (ViatorrustAudioProcessor&);
    ~ViatorrustAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ViatorrustAudioProcessor& audioProcessor;
    
    // header
    Header _headerComp;
    
    // io
    juce::OwnedArray<viator_gui::ImageFader> _ioFaders;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> _ioAttachments;
    void setIOSliderProps();
    
    // vinyl
    juce::OwnedArray<viator_gui::ImageFader> _vinylDials;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> _vinylAttachments;
    void setVinylDialProps();
    
    // buttons
    juce::OwnedArray<viator_gui::ImageButton> _buttons;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> _buttonAttachments;
    void setButtonProps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViatorrustAudioProcessorEditor)
};
