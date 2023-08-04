#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "components/Header.h"

class ViatorrustAudioProcessorEditor  : public juce::AudioProcessorEditor
, private juce::Timer
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
    juce::StringArray _ioTooltips =
    {
      "Controls the gain of the signal coming into the plugin. Positive values will increase distortion if the Track button is on.",
        "Controls the volume of the signal coming out of the plugin (master volume)."
    };
    
    // vinyl
    juce::OwnedArray<viator_gui::ImageFader> _vinylDials;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> _vinylAttachments;
    void setVinylDialProps();
    juce::StringArray _vinylDialsTooltips =
    {
      "Constrains the frequency response while distorting the mid range to model vintage audio recordings.",
        "Controls the volume of synthesized noise modeling the hiss of tape and vintage audio equiptment.",
        "Controls the volume of synthesized crackle to model vinyl dust and dirty potentiometers.",
        "Contols the frequency of an LFO modulating the amplitued of the hiss."
    };
    
    
    // buttons
    juce::OwnedArray<viator_gui::ImageButton> _buttons;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> _buttonAttachments;
    void setButtonProps();
    juce::StringArray _buttonTooltips =
    {
      "Toggles the input from the DAW track on and off. Off means you only hear the hiss and dust. Some DAWs require audio for this to run, though. Try kick-starting with an audio clip if it doesn't work.",
        "Sums the signal to mono in case you REALLY want that old-school recording vibe.",
        "Limits the output volume of the plugin with a hard clipper so that nothing blows up."
    };
    
    // vu
    viator_gui::VUMeter _vuMeter;
    
    // timer
    void timerCallback() override;
    
    juce::Label _tooltipLabel;
    void mouseEnter(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;
    
    void savePluginBounds();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViatorrustAudioProcessorEditor)
};
