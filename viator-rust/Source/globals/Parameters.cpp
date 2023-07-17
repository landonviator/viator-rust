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
    
    // degree
    _sliderParams.push_back({ViatorParameters::degreeID, ViatorParameters::degreeName, 1.0f, 10.0f, 1.0f, skew::kNoSkew, 0.0, type::kInt});
    
    // order
    _sliderParams.push_back({ViatorParameters::orderID, ViatorParameters::orderName, 1.0f, 10.0f, 1.0f, skew::kNoSkew, 0.0, type::kInt});
}

void ViatorParameters::Params::initButtonParams()
{
}
