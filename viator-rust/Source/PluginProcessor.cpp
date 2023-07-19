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
    
    // noise lowpass
    auto noiseLP = _treeState.getRawParameterValue(ViatorParameters::lowpassNoiseID)->load();
    _noiseLowpassModule.setCutoffFrequency(noiseLP);
    
    // hum
    auto hum = _treeState.getRawParameterValue(ViatorParameters::lfoFreqID)->load();
    auto humFreq = _treeState.getRawParameterValue(ViatorParameters::lfoFreqID)->load();
    _lfoOsc.setFrequency(humFreq);
    _humFilterModule.setParameter(svFilter::ParameterId::kCutoff, humFreq * 2.0);
    _humFilterModule.setParameter(svFilter::ParameterId::kGain, hum);
    
    phaseIncrement = 2.0 * 3.14 * humFreq / getSampleRate();
    
    // testing
    _coeffA.store(_treeState.getRawParameterValue(ViatorParameters::coeffAID)->load());
}

//==============================================================================
void ViatorrustAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    _spec.maximumBlockSize = samplesPerBlock;
    _spec.numChannels = getTotalNumInputChannels();
    _spec.sampleRate = sampleRate;
    
//    _henonOsc.prepare(_spec);
//    _henonOsc.initialise([this](float input)
//     {
//            double x = y_ + 1.0 - a_ * x_ * x_;
//            double y = b_ * x_;
//
//            x_ = x;
//            y_ = y;
//
//            input = x_ * amplitude_;
//            input *= 0.5; // Scale the output to prevent clipping
//
//            return static_cast<float>(input);
//     });
    
    // lfo
    _lfoOsc.prepare(_spec);
    _lfoOsc.initialise([this](float input)
     {
        return std::sin(input);
     });
    
    // noise lowpass
    _noiseLowpassModule.prepare(_spec);
    _noiseLowpassModule.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    
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
    
//    auto* channelData = buffer.getArrayOfWritePointers();
    
//    for (int sample = 0; sample < buffer.getNumSamples(); sample++)
//    {
//        for (int ch = 0; ch < buffer.getNumChannels(); ch++)
//        {
//            auto input = channelData[ch][sample];
//            auto modulationSignal = _lfoOsc.processSample(input) + 1.0f;
//
//            channelData[ch][sample] = std::sin(x);
//        }
//
//        const double new_x = y + 1 - a * x * x;
//        const double new_y = b * x;
//        x = new_x;
//        y = new_y;
//    }
    int numSamplesProcessed = 0;
    
    for (int ch = 0; ch < buffer.getNumChannels(); ch++)
    {
        auto* channelData = buffer.getWritePointer(ch);
        
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            auto input = channelData[sample];
            auto modulationSignal = _lfoOsc.processSample(input) + 1.0f;
            
            channelData[sample] = std::sin(x);
            
            if (sample % static_cast<int>(_treeState.getRawParameterValue(ViatorParameters::chaosFreqID)->load()) == 0)
            {
                const double new_x = y + 1 - a * (x * x);
                const double new_y = _coeffA.load() * x;
                x = new_x;
                y = new_y;
            }
        }
    }
}

float ViatorrustAudioProcessor::processPolynomial(float input)
{
    auto rawDrive = _treeState.getRawParameterValue(ViatorParameters::driveID)->load();
    auto drive = juce::Decibels::decibelsToGain(rawDrive);
    input *= drive;
    
    return 16.0 * pow(input , 5.0) - 20.0 * pow(input, 3.0) + 5 * input;
}

float ViatorrustAudioProcessor::getHenonSample()
{
//    double x = y_ + 1.0 - a_ * x_ * x_;
//    double y = b_ * x_;
//
//    x_ = x;
//    y_ = y;
//
//    double sample = x_ * amplitude_;
//    sample *= 0.5; // Scale the output to prevent clipping
//
//    return static_cast<float>(sample);
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
