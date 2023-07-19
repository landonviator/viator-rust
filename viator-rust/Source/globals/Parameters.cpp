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
    
    // drive
    _sliderParams.push_back({ViatorParameters::driveID, ViatorParameters::driveName, 0.0f, 30.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    
    // hum
    _sliderParams.push_back({ViatorParameters::humID, ViatorParameters::humName, -30.0f, 30.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    _sliderParams.push_back({ViatorParameters::humFreqID, ViatorParameters::humFreqName, 20.0f, 20000.0f, 60.0f, skew::kSkew, 1000.0, type::kInt});
    
    // filter
    _sliderParams.push_back({ViatorParameters::lowpassNoiseID, ViatorParameters::lowpassNoiseName, 20.0f, 20000.0f, 1400.0f, skew::kSkew, 1400.0, type::kInt});
}

void ViatorParameters::Params::initButtonParams()
{
    // mode
    _buttonParams.push_back({ViatorParameters::modeID, ViatorParameters::modeName, true});
}
