#include "Parameters.h"


ViatorParameters::Params::Params()
{
    initSliderParams();
    initButtonParams();
}

void ViatorParameters::Params::initSliderParams()
{
    using skew = SliderParameterData::SkewType;
    using type = SliderParameterData::NumericType;
    
    // input
    _sliderParams.push_back({ViatorParameters::inputID, ViatorParameters::inputName, -20.0f, 20.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    
    // output
    _sliderParams.push_back({ViatorParameters::outputID, ViatorParameters::outputName, -20.0f, 20.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    
    // lfo
    _sliderParams.push_back({ViatorParameters::lfoFreqID, ViatorParameters::lfoFreqName, 1.0f, 220.0f, 2.0f, skew::kNoSkew, 0.0, type::kInt});
    
    // hiss
    _sliderParams.push_back({ViatorParameters::hissSpeedID, ViatorParameters::hissSpeedName, 1.0f, 60.0f, 10.0f, skew::kNoSkew, 0.0, type::kInt});
    _sliderParams.push_back({ViatorParameters::hissVolumeID, ViatorParameters::hissVolumeName, -30.0f, 30.0f, 0.0f, skew::kNoSkew, 0.0, type::kInt});
    _sliderParams.push_back({ViatorParameters::hissToneID, ViatorParameters::hissToneName, 20.0f, 20000.0f, 1000.0f, skew::kSkew, 1000.0, type::kInt});
}

void ViatorParameters::Params::initButtonParams()
{
    // mode
    _buttonParams.push_back({ViatorParameters::modeID, ViatorParameters::modeName, true});
}
