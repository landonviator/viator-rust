#pragma once
#include <JuceHeader.h>
#include "globals/Parameters.h"

/** A Ramper applies linear ramping to a value.
*    @ingroup utility
*
*/
class Ramper
{
public:
    Ramper():
        targetValue(0.0f),
        stepDelta(0.0f),
        stepAmount(-1)
    {};

    /** Sets the step amount that the ramper will use. You can overwrite this value by supplying a step number in setTarget. */
    void setStepAmount(int newStepAmount) { stepAmount = newStepAmount; };
    /** sets the new target and recalculates the step size using either the supplied step number or the step amount previously set by setStepAmount(). */
    void setTarget(float currentValue, float newTarget, int numberOfSteps=-1)
    {
        if(numberOfSteps != -1) stepDelta = (newTarget - currentValue) / numberOfSteps;
        else if (stepAmount != -1) stepDelta = (newTarget - currentValue) / stepAmount;
        else jassertfalse; // Either the step amount should be set, or a new step amount should be supplied
        targetValue = newTarget;
    };
    /** Sets the ramper value and the target to the new value and stops ramping. */
    void setValue(float newValue)
    {
        targetValue = newValue;
        stepDelta = 0.0f;
    };
    /** ramps the supplied value and returns true if the targetValue is reached. */
    inline bool ramp(float &valueToChange)
    {
        valueToChange += stepDelta;
        return abs(targetValue - valueToChange) > 0.001;
        
    };
private:
    float targetValue, stepDelta;
    int stepAmount;
};

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
    juce::dsp::LinkwitzRileyFilter<float> _hissLowpassModule;
    juce::dsp::LinkwitzRileyFilter<float> _inputLowpassModule;
    juce::dsp::LinkwitzRileyFilter<float> _hissSpeedFilterModule;
    juce::dsp::LinkwitzRileyFilter<float> _hissHighpassModule;
    juce::dsp::LinkwitzRileyFilter<float> _noiseLowpassModule;
    juce::dsp::LinkwitzRileyFilter<float> _midSeparaterModule;
    juce::dsp::LinkwitzRileyFilter<float> _lowSeparaterModule;
    juce::dsp::LinkwitzRileyFilter<float> _lowConstrainFilterModule;
    juce::dsp::LinkwitzRileyFilter<float> _highConstrainFilterModule;
    viator_dsp::SVFilter<float> _humFilterModule;
    juce::dsp::Gain<float> _hissVolumeModule;
    juce::AudioBuffer<float> _dustBuffer;
    juce::AudioBuffer<float> silentBuffer;
    juce::Time _time;
    juce::Random _noise{_time.getMilliseconds()};
    void synthesizeRandomCrackle(juce::AudioBuffer<float>& buffer);
    void distortMidRange(juce::AudioBuffer<float>& buffer);
    float rampedValue = 0.0f;
    Ramper _ramper;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViatorrustAudioProcessor)
};
