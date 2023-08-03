#pragma once
#include <JuceHeader.h>
#include "Globals.h"

namespace ViatorParameters
{
// Param data
struct SliderParameterData
{
    enum SkewType
    {
        kSkew,
        kNoSkew
    };
    
    enum NumericType
    {
        kInt,
        kFloat
    };
    
    public:
        juce::String paramID;
        juce::String paramName;
        float min;
        float max;
        float initial;
        SkewType isSkew;
        float center;
        NumericType isInt;
};

struct ButtonParameterData
{
    public:
        juce::String paramID;
        juce::String paramName;
        bool initial;
};

    class Params
    {
    public:
        Params();
        
        // Get a ref to the param data
        std::vector<ViatorParameters::SliderParameterData>& getSliderParams(){return _sliderParams;};
        std::vector<ViatorParameters::ButtonParameterData>& getButtonParams(){return _buttonParams;};
        
        // io attach
        std::vector<ViatorParameters::SliderParameterData>& getIOSliderParams(){return _ioSliderParams;};
        
        // vinyl attach
        std::vector<ViatorParameters::SliderParameterData>& getVinylSliderParams(){return _vinylSliderParams;};
        
    private:
        // Adds params to the vector
        void initSliderParams();
        void initButtonParams();
        
        
    private:
        // Vector holding param data
        std::vector<ViatorParameters::SliderParameterData> _sliderParams;
        std::vector<ViatorParameters::ButtonParameterData> _buttonParams;
        
        // io attach
        std::vector<ViatorParameters::SliderParameterData> _ioSliderParams;
        
        // vinyl attach
        std::vector<ViatorParameters::SliderParameterData> _vinylSliderParams;
    };
}
