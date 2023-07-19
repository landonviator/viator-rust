#pragma once
#include <JuceHeader.h>
#include "globals/Parameters.h"

//==============================================================================
/**
*/
class ViatorrustAudioProcessor  : public juce::AudioProcessor
, public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    ViatorrustAudioProcessor();
    ~ViatorrustAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // parameters
    ViatorParameters::Params _parameterMap;
    juce::AudioProcessorValueTreeState _treeState;
    
private:
    
    // parameters
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    using Parameter = juce::AudioProcessorValueTreeState::Parameter;
    static juce::String valueToTextFunction(float x) { return juce::String(static_cast<int>(x)); }
    static float textToValueFunction(const juce::String& str) { return str.getFloatValue(); }
    void updateParameters();
    
    // dsp
    juce::dsp::ProcessSpec _spec;
    float processPolynomial(float input);
    float getHenonSample();
    juce::dsp::Oscillator<float> _henonOsc;
    juce::dsp::Oscillator<float> _lfoOsc;
    double frequency_;
    double amplitude_;
    double a_;
    double b_;
    double x_; // State variable for the Henon map
    double y_; // State variable for the Henon map
    juce::dsp::LinkwitzRileyFilter<float> _noiseLowpassModule;
    viator_dsp::SVFilter<float> _humFilterModule;
    
    // Lookup Table
    int _lookupTableSize;
    juce::dsp::LookupTableTransform<float> _lookupTable;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViatorrustAudioProcessor)
};
