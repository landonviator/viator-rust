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
        juce::String name;
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
        juce::String _id;
        juce::String _name;
        bool _initial;
};

    class Params
    {
    public:
        Params();
        
        // Get a ref to the param data
        std::vector<ViatorParameters::SliderParameterData>& getSliderParams(){return _sliderParams;};
        std::vector<ViatorParameters::ButtonParameterData>& getButtonParams(){return _buttonParams;};
        
    private:
        // Adds params to the vector
        void initSliderParams();
        void initButtonParams();
        
        
    private:
        // Vector holding param data
        std::vector<ViatorParameters::SliderParameterData> _sliderParams;
        std::vector<ViatorParameters::ButtonParameterData> _buttonParams;
        
    };
}
