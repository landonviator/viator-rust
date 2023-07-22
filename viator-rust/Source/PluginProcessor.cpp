#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
ViatorrustAudioProcessor::ViatorrustAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
, _treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    // sliders
    for (int i = 0; i < _parameterMap.getSliderParams().size(); i++)
    {
        _treeState.addParameterListener(_parameterMap.getSliderParams()[i].paramID, this);
    }
    
    // buttons
    for (int i = 0; i < _parameterMap.getButtonParams().size(); i++)
    {
        _treeState.addParameterListener(_parameterMap.getButtonParams()[i]._id, this);
    }
}

ViatorrustAudioProcessor::~ViatorrustAudioProcessor()
{
    // sliders
    for (int i = 0; i < _parameterMap.getSliderParams().size(); i++)
    {
        _treeState.removeParameterListener(_parameterMap.getSliderParams()[i].paramID, this);
    }
    
    // buttons
    for (int i = 0; i < _parameterMap.getButtonParams().size(); i++)
    {
        _treeState.removeParameterListener(_parameterMap.getButtonParams()[i]._id, this);
    }
}

//==============================================================================
const juce::String ViatorrustAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ViatorrustAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ViatorrustAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ViatorrustAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ViatorrustAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ViatorrustAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ViatorrustAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ViatorrustAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ViatorrustAudioProcessor::getProgramName (int index)
{
    return {};
}

void ViatorrustAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ViatorrustAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // sliders
    for (int i = 0; i < _parameterMap.getSliderParams().size(); i++)
    {
        auto param = _parameterMap.getSliderParams()[i];

        if (param.isInt == ViatorParameters::SliderParameterData::NumericType::kInt || param.isSkew == ViatorParameters::SliderParameterData::SkewType::kSkew)
        {
            auto range = juce::NormalisableRange<float>(param.min, param.max);

            if (param.isSkew == ViatorParameters::SliderParameterData::SkewType::kSkew)
            {
                range.setSkewForCentre(param.center);
            }

            params.push_back (std::make_unique<juce::AudioProcessorValueTreeState::Parameter>(juce::ParameterID { param.paramID, 1 }, param.name, param.name, range, param.initial, valueToTextFunction, textToValueFunction));
        }

        else
        {
            params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { param.paramID, 1 }, param.name, param.min, param.max, param.initial));
        }
    }
    
    // buttons
    for (int i = 0; i < _parameterMap.getButtonParams().size(); i++)
    {
        auto param = _parameterMap.getButtonParams()[i];
        params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID { param._id, 1 }, param._name, false));
    }
    
    return { params.begin(), params.end() };
}

void ViatorrustAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)

{
    updateParameters();
}

void ViatorrustAudioProcessor::updateParameters()
{
    using svFilter = viator_dsp::SVFilter<float>;
}

//==============================================================================
void ViatorrustAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    _spec.maximumBlockSize = samplesPerBlock;
    _spec.numChannels = getTotalNumInputChannels();
    _spec.sampleRate = sampleRate;
    
    _henonOsc.prepare(_spec);
    _henonOsc.initialise([this](float input)
     {
            double x = y_ + 1.0 - a * x_ * x_;
            double y = b * x_;

            x_ = x;
            y_ = y;

            input = x_ * amplitude_;
            input *= 0.5; // Scale the output to prevent clipping

            return static_cast<float>(input);
     });
    
    // lfo
    _lfoOsc.prepare(_spec);
    _lfoOsc.initialise([this](float input)
     {
        return std::sin(input);
     });
    
    // noise lowpass
    _hissLowpassModule.prepare(_spec);
    _hissLowpassModule.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    _hissLowpassModule.setCutoffFrequency(1000.0);
    _hissSpeedFilterModule.prepare(_spec);
    _hissSpeedFilterModule.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    _hissSpeedFilterModule.setCutoffFrequency(10.0);
    _inputLowpassModule.prepare(_spec);
    _inputLowpassModule.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    _inputLowpassModule.setCutoffFrequency(1000.0);
    
    updateParameters();
}

void ViatorrustAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ViatorrustAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ViatorrustAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::dsp::AudioBlock<float> block {buffer};
    
//    for (int ch = 0; ch < buffer.getNumChannels(); ch++)
//    {
//        auto* channelData = buffer.getWritePointer(ch);
//
//        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
//        {
//            auto input = channelData[sample];
//            channelData[sample] = input;
//        }
//    }
    
    synthesizeRandomHiss(buffer);
    buffer.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
}

void ViatorrustAudioProcessor::synthesizeRandomHiss(juce::AudioBuffer<float> &buffer)
{
    auto hissSpeed = _treeState.getRawParameterValue(ViatorParameters::hissSpeedID)->load();
    _hissSpeedFilterModule.setCutoffFrequency(hissSpeed);
    
    auto hissVolumeDB = _treeState.getRawParameterValue(ViatorParameters::hissVolumeID)->load();
    auto hissVolume = juce::Decibels::decibelsToGain(hissVolumeDB);
    
    auto hissTone = _treeState.getRawParameterValue(ViatorParameters::hissToneID)->load();
    _hissLowpassModule.setCutoffFrequency(hissTone);
    
    for (int ch = 0; ch < buffer.getNumChannels(); ch++)
    {
        auto* channelData = buffer.getWritePointer(ch);
        
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            auto input = channelData[sample];
            
            auto noise = _noise.nextFloat() * 2.0 - 1.0;
            auto noiseSpeed = _hissSpeedFilterModule.processSample(ch, noise);
            auto filteredNoise = _hissLowpassModule.processSample(ch, noise);
            auto filteredInput = _inputLowpassModule.processSample(ch, input);
            
            noiseSpeed *= 10.0;
            noiseSpeed *= noiseSpeed;
            noiseSpeed *= 20.0;
            
            auto hiss = filteredNoise * noiseSpeed + filteredNoise * 0.05;
            
            channelData[sample] = hiss * hissVolume + filteredInput;
        }
    }
}

//==============================================================================
bool ViatorrustAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ViatorrustAudioProcessor::createEditor()
{
    //return new ViatorrustAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void ViatorrustAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    _treeState.state.writeToStream (stream);
}

void ViatorrustAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, size_t(sizeInBytes));
    if (tree.isValid())
    {
        _treeState.state = tree;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ViatorrustAudioProcessor();
}
