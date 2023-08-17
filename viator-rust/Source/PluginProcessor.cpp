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
        _treeState.addParameterListener(_parameterMap.getButtonParams()[i].paramID, this);
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
        _treeState.removeParameterListener(_parameterMap.getButtonParams()[i].paramID, this);
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

            params.push_back (std::make_unique<juce::AudioProcessorValueTreeState::Parameter>(juce::ParameterID { param.paramID, 1 }, param.paramName, param.paramName, range, param.initial, valueToTextFunction, textToValueFunction));
        }

        else
        {
            params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { param.paramID, 1 }, param.paramName, param.min, param.max, param.initial));
        }
    }
    
    // buttons
    for (int i = 0; i < _parameterMap.getButtonParams().size(); i++)
    {
        auto param = _parameterMap.getButtonParams()[i];
        params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID { param.paramID, 1 }, param.paramName, _parameterMap.getButtonParams()[i].initial));
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
    auto rawDrive = _treeState.getRawParameterValue(ViatorParameters::driveID)->load();
    
    auto reso = juce::jmap(rawDrive, 0.0f, 30.0f, 0.05f, 0.95f);
    _bpFilterModule.setResonance(reso);
    
    // volume
    auto ageCompensation = juce::jmap(rawDrive, 0.0f, 30.0f, 0.0f, 6.0f);
    _ageCompensationModule.setGainDecibels(ageCompensation);
    
    auto input = _treeState.getRawParameterValue(ViatorParameters::inputID)->load();
    auto output = _treeState.getRawParameterValue(ViatorParameters::outputID)->load();
    _inputVolumeModule.setGainDecibels(input);
    _outputVolumeModule.setGainDecibels(output);
}

//==============================================================================
void ViatorrustAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    _spec.maximumBlockSize = samplesPerBlock;
    _spec.numChannels = getTotalNumInputChannels();
    _spec.sampleRate = sampleRate;
    
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
    
    _ageCompensationModule.prepare(_spec);
    _ageCompensationModule.setRampDurationSeconds(0.02);
    
    _inputVolumeModule.prepare(_spec);
    _inputVolumeModule.setRampDurationSeconds(0.02);
    _outputVolumeModule.prepare(_spec);
    _outputVolumeModule.setRampDurationSeconds(0.02);
    
    _ramper.setTarget(0.0f, 1.0f, sampleRate * 0.02);
    
    silentBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    silentBuffer.clear();
    _dustBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    
    _vinylLFO.initialise([this](float x){return std::sin(x);});
    
    levelGain.reset(sampleRate, 0.5);
    
    _bpFilterModule.prepare(_spec);
    _bpFilterModule.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    _bpFilterModule.setCutoffFrequency(600.0);
    
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
    numChannels.store(buffer.getNumChannels());
    
    juce::dsp::AudioBlock<float> block {buffer};
    juce::dsp::AudioBlock<float> dustBlock {_dustBuffer};
    
    _inputVolumeModule.process(juce::dsp::ProcessContextReplacing<float>(block));
    
    // generate dust
    synthesizeRandomCrackle(_dustBuffer);

    // source
    auto sourceTrack = _treeState.getRawParameterValue(ViatorParameters::sourceModeID)->load();

    // add effects to main signal
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        if (sourceTrack)
        {
            buffer.addFrom(channel, 0, _dustBuffer, channel, 0, buffer.getNumSamples());
        }

        else
        {
            buffer.copyFrom(channel, 0, _dustBuffer, channel, 0, buffer.getNumSamples());
        }
    }

    // apply age
    distortMidRange(buffer);

    // mono vs stereo
    auto isStereo = _treeState.getRawParameterValue(ViatorParameters::stereoModeID)->load();

    if (isStereo && numChannels.load() == 2)
    {
        buffer.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    }

    auto safeClip = _treeState.getRawParameterValue(ViatorParameters::safeClipID)->load();

    if (safeClip)
    {
        viator_utils::utils::hardClipBlock(block);
    }
    
    _outputVolumeModule.process(juce::dsp::ProcessContextReplacing<float>(block));
    
    // get meter value
    calculatePeakSignal(buffer);
}

void ViatorrustAudioProcessor::calculatePeakSignal(juce::AudioBuffer<float> &buffer)
{
    levelGain.skip(buffer.getNumSamples());
    peakDB = buffer.getMagnitude(0, 0, buffer.getNumSamples());
    
    if (peakDB < levelGain.getCurrentValue())
    {
        levelGain.setTargetValue(peakDB);
    }

    else
    {
        levelGain.setCurrentAndTargetValue(peakDB);
    }
}

float ViatorrustAudioProcessor::getCurrentPeakSignal()
{
    return juce::Decibels::gainToDecibels(levelGain.getNextValue());
}

void ViatorrustAudioProcessor::synthesizeRandomCrackle(juce::AudioBuffer<float> &buffer)
{
    _hissSpeedFilterModule.setCutoffFrequency(60.0);
    auto hissVolume = _treeState.getRawParameterValue(ViatorParameters::hissVolumeID)->load();
    auto hissGain = juce::Decibels::decibelsToGain(hissVolume + 5.0);
    auto dustVolume = _treeState.getRawParameterValue(ViatorParameters::dustVolumeID)->load();
    auto dustGain = juce::Decibels::decibelsToGain(dustVolume - 6.0);
    
    // lfo
    auto lfo = _treeState.getRawParameterValue(ViatorParameters::vinylLFOID)->load();
    _vinylLFO.setFrequency(_noise.nextFloat() * lfo);
    
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
                auto hiss = _noiseLowpassModule.processSample(ch, noise) * 0.01;
                auto dust = _hissHighpassModule.processSample(ch, signal);
                _vinylLFO.processSample(hiss);
                
                if (_vinylLFO.getFrequency() > 0.0)
                {
                    auto output = hiss * hissGain * _vinylLFO.processSample(hiss) + dust * dustGain;
                    channelData[sample] = output;
                }
                
                else
                {
                    auto output = hiss * hissGain + dust * dustGain;
                    channelData[sample] = output;
                }
            }
        }
    }
}

void ViatorrustAudioProcessor::distortMidRange(juce::AudioBuffer<float> &buffer)
{
    auto driveDB = _treeState.getRawParameterValue(ViatorParameters::driveID)->load();
    auto compensate = juce::jmap(driveDB, 0.0f, 30.0f, 0.0f, -6.0f);
    auto mix = driveDB / 30.0f;
    auto drive = juce::Decibels::decibelsToGain(driveDB);
    
    for (int ch = 0; ch < buffer.getNumChannels(); ch++)
    {
        auto* channelData = buffer.getWritePointer(ch);
        
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            auto input = channelData[sample];
            float midRange = _bpFilterModule.processSample(ch, input);
            float restRange = input - midRange;
            auto distortedMid = 2.0 / 3.14 * std::atan(midRange * drive);
            auto compensatedMid = distortedMid * juce::Decibels::decibelsToGain(compensate);
            channelData[sample] = (1.0 - mix) * restRange + compensatedMid * mix;
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
    return new ViatorrustAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void ViatorrustAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    _treeState.state.appendChild(variableTree, nullptr);
    juce::MemoryOutputStream stream(destData, false);
    _treeState.state.writeToStream (stream);
}

void ViatorrustAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, size_t(sizeInBytes));
    variableTree = tree.getChildWithName("Variables");
    
    if (tree.isValid())
    {
        _treeState.state = tree;
        _width = variableTree.getProperty("width");
        _height = variableTree.getProperty("height");
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ViatorrustAudioProcessor();
}
