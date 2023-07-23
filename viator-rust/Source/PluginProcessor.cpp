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
    // filters
    auto cutoff = _treeState.getRawParameterValue(ViatorParameters::driveID)->load();
    auto lowRrange = juce::NormalisableRange<float>(20.0f, 300.0f, 1.0f);
    auto highRange = juce::NormalisableRange<float>(3000.0f, 20000.0f, 1.0f);
    lowRrange.setSkewForCentre(100.0f);
    highRange.setSkewForCentre(10000.0);
    auto lowCutoff = juce::jmap(cutoff, 0.0f, 30.0f, lowRrange.start, lowRrange.end);
    auto highCutoff = juce::jmap(cutoff, 0.0f, 30.0f, highRange.end, highRange.start);
    _lowConstrainFilterModule.setCutoffFrequency(lowCutoff);
    _highConstrainFilterModule.setCutoffFrequency(highCutoff);
    
    // gain
    auto hissVolume = _treeState.getRawParameterValue(ViatorParameters::hissVolumeID)->load();
    _hissVolumeModule.setGainDecibels(hissVolume);
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
    _hissHighpassModule.prepare(_spec);
    _hissHighpassModule.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    _hissHighpassModule.setCutoffFrequency(100.0);
    _noiseLowpassModule.prepare(_spec);
    _noiseLowpassModule.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    _noiseLowpassModule.setCutoffFrequency(7000.0);
    
    _midSeparaterModule.prepare(_spec);
    _midSeparaterModule.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    _midSeparaterModule.setCutoffFrequency(4000.0);
    _lowSeparaterModule.prepare(_spec);
    _lowSeparaterModule.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    _lowSeparaterModule.setCutoffFrequency(300.0);
    
    _lowConstrainFilterModule.prepare(_spec);
    _lowConstrainFilterModule.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    _highConstrainFilterModule.prepare(_spec);
    _highConstrainFilterModule.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    
    _hissVolumeModule.prepare(_spec);
    _hissVolumeModule.setRampDurationSeconds(0.02);
    
    _ramper.setTarget(0.0f, 1.0f, sampleRate * 0.02);
    
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
    // init buffers
    _dustBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples());
    
    juce::dsp::AudioBlock<float> block {buffer};
    juce::dsp::AudioBlock<float> dustBlock {_dustBuffer};
    _lowConstrainFilterModule.process(juce::dsp::ProcessContextReplacing<float>(block));
    _highConstrainFilterModule.process(juce::dsp::ProcessContextReplacing<float>(block));
    
    // generate dust
    synthesizeRandomCrackle(_dustBuffer);
    
    // apply age
    distortMidRange(_dustBuffer);
    
    // dust volume
    _hissVolumeModule.process(juce::dsp::ProcessContextReplacing<float>(dustBlock));
    
    // add effects to main signal
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        buffer.addFrom(channel, 0, _dustBuffer, channel, 0, buffer.getNumSamples());
    }
    
    auto isStereo = _treeState.getRawParameterValue(ViatorParameters::modeID)->load();
    
    if (isStereo)
    {
        buffer.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    }
}

void ViatorrustAudioProcessor::synthesizeRandomCrackle(juce::AudioBuffer<float> &buffer)
{
    _hissSpeedFilterModule.setCutoffFrequency(60.0);
    auto driveDB = _treeState.getRawParameterValue(ViatorParameters::driveID)->load();
    auto drive = juce::Decibels::decibelsToGain(driveDB);
    
    for (int ch = 0; ch < buffer.getNumChannels(); ch++)
    {
        auto* channelData = buffer.getWritePointer(ch);
        
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            auto noise = (_noise.nextFloat() * 2.0 - 1.0) * 0.1;
            auto filteredNoise = _hissLowpassModule.processSample(ch, noise);
            auto noiseSpeed = _hissSpeedFilterModule.processSample(ch, filteredNoise);
            noiseSpeed *= 10.0;
            noiseSpeed *= noiseSpeed;
            noiseSpeed *= 20.0;
            auto signal = noiseSpeed;
            
            if (rampedValue < 1.0)
            {
                signal *= rampedValue;
            }
            
            else
            {
                _ramper.setTarget(0.96f, 1.0f, getSampleRate() * 0.003);
                rampedValue = 0.0;
            }
            
            while(!_ramper.ramp(rampedValue))
            {
                signal *= rampedValue;
            }
            
            if (rampedValue < 1.0)
            {
                auto outNoise = _noiseLowpassModule.processSample(ch, noise) * 0.01;
                auto hpNoise = _hissHighpassModule.processSample(ch, signal);
                auto output = outNoise + hpNoise;
                channelData[sample] = output;
            }
        }
    }
}

void ViatorrustAudioProcessor::distortMidRange(juce::AudioBuffer<float> &buffer)
{
    auto driveDB = _treeState.getRawParameterValue(ViatorParameters::driveID)->load();
    auto drive = juce::Decibels::decibelsToGain(driveDB);
    
    for (int ch = 0; ch < buffer.getNumChannels(); ch++)
    {
        auto* channelData = buffer.getWritePointer(ch);
        
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            auto input = channelData[sample];
            auto highPassed = _lowSeparaterModule.processSample(ch, input);
            float midRange;
            float high;
            _midSeparaterModule.processSample(ch, highPassed, midRange, high);
            auto distortedMid = 2.0 / 3.14 * std::atan(midRange * drive);
            auto compensatedMid = distortedMid * juce::Decibels::decibelsToGain(-driveDB * 0.5);
            auto output = input - highPassed + compensatedMid + high;
            channelData[sample] = output;
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
