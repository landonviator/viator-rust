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
    
    // vinyl
    _sliderParams.push_back({ViatorParameters::driveID, ViatorParameters::driveName, 0.0f, 30.0f, 30.0f, skew::kNoSkew, 0.0, type::kFloat});
    _sliderParams.push_back({ViatorParameters::hissVolumeID, ViatorParameters::hissVolumeName, -30.0f, 30.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    _sliderParams.push_back({ViatorParameters::dustVolumeID, ViatorParameters::dustVolumeName, -30.0f, 30.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    _sliderParams.push_back({ViatorParameters::vinylLFOID, ViatorParameters::vinylLFOName, 0.0f, 1.0f, 0.0f, skew::kNoSkew, 0.1, type::kFloat});
}

void ViatorParameters::Params::initButtonParams()
{
    // mode
    _buttonParams.push_back({ViatorParameters::stereoModeID, ViatorParameters::stereoModeName, false});
    
    // source
    _buttonParams.push_back({ViatorParameters::sourceModeID, ViatorParameters::sourceModeName, true});
}
