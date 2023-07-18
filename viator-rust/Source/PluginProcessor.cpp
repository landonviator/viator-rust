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

}

//==============================================================================
void ViatorrustAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    _spec.maximumBlockSize = samplesPerBlock;
    _spec.numChannels = getTotalNumInputChannels();
    _spec.sampleRate = sampleRate;
    
    _lookupTableSize = samplesPerBlock * 0.1;
    
    // populate table
    _lookupTable.initialise([](float x) {return 2.0 / 3.14 * std::atan(x * 20.0);}, -1.0f, 1.0f, 64);
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
    
//    auto drive = _treeState.getRawParameterValue(ViatorParameters::driveID)->load();
//    auto degree = _treeState.getRawParameterValue(ViatorParameters::degreeID)->load();
//    auto order = _treeState.getRawParameterValue(ViatorParameters::orderID)->load();
    
    for (int ch = 0; ch < buffer.getNumChannels(); ch++)
    {
        auto* channelData = buffer.getWritePointer(ch);
        
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            channelData[sample] = _lookupTable.processSampleUnchecked(channelData[sample]);
            channelData[sample] = _lookupTable.processSampleUnchecked(channelData[sample]);
            channelData[sample] = _lookupTable.processSampleUnchecked(channelData[sample]);
            channelData[sample] = _lookupTable.processSampleUnchecked(channelData[sample]);
            channelData[sample] = _lookupTable.processSampleUnchecked(channelData[sample]);
            channelData[sample] = _lookupTable.processSampleUnchecked(channelData[sample]);
            channelData[sample] = _lookupTable.processSampleUnchecked(channelData[sample]);
            channelData[sample] = _lookupTable.processSampleUnchecked(channelData[sample]);
            channelData[sample] = _lookupTable.processSampleUnchecked(channelData[sample]);
            channelData[sample] = _lookupTable.processSampleUnchecked(channelData[sample]);
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
