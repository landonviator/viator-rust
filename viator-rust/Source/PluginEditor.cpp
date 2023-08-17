#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ViatorrustAudioProcessorEditor::ViatorrustAudioProcessorEditor (ViatorrustAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // header
    addAndMakeVisible(_headerComp);
    
    // io
    setIOSliderProps();
    
    // vinyl
    setVinylDialProps();
    
    // buttons
    setButtonProps();
    
    // vu
    auto main = juce::ImageCache::getFromMemory(BinaryData::vu_meter_png, BinaryData::vu_meter_pngSize);
    auto grid = juce::ImageCache::getFromMemory(BinaryData::scale_vumeter_png, BinaryData::scale_vumeter_pngSize);
    auto bg = juce::ImageCache::getFromMemory(BinaryData::back_vumeter_decore5_png, BinaryData::back_vumeter_decore5_pngSize);
    _vuMeter.setVUImages(main, grid, bg);
    addAndMakeVisible(_vuMeter);
    
    startTimer(20);
    
    addAndMakeVisible(_tooltipLabel);
    _tooltipLabel.setJustificationType(juce::Justification::centred);
    _tooltipLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::whitesmoke.withAlpha(0.5f));
    
    // window
    viator_utils::PluginWindow::setPluginWindowSize(audioProcessor._width, audioProcessor._height, *this, 1.5, 0.75);
}

ViatorrustAudioProcessorEditor::~ViatorrustAudioProcessorEditor()
{
    stopTimer();
    
    for (int i = 0; i < _vinylDials.size(); i++)
    {
        _vinylDials[i]->removeMouseListener(this);
    }
    
    for (int i = 0; i < _ioFaders.size(); i++)
    {
        _ioFaders[i]->removeMouseListener(this);
    }
    
    for (int i = 0; i < _buttons.size(); i++)
    {
        _buttons[i]->getButton().removeMouseListener(this);
    }
}

//==============================================================================
void ViatorrustAudioProcessorEditor::paint (juce::Graphics& g)
{
    // bg color
    g.fillAll (juce::Colours::black);
    
    // image bg
    auto bgImage = juce::ImageCache::getFromMemory(BinaryData::back_d_png, BinaryData::back_d_pngSize);
    auto bgWidth = getWidth() * 0.9;
    auto bgHeight = getHeight() * 0.82;
    auto bgY = getHeight() * 0.13;
    g.drawImage(bgImage, getLocalBounds().toFloat().withSizeKeepingCentre(bgWidth, bgHeight).withY(bgY));
}

void ViatorrustAudioProcessorEditor::resized()
{
    // header
    const auto headerHeightMult = 0.1;
    const auto headerHeight = getHeight() * headerHeightMult;
    _headerComp.setBounds(0, 0, getWidth(), headerHeight);
    
    const auto tooltipX = getWidth() * 0.2;
    const auto tooltipWidth = getWidth() * 0.75;
    _tooltipLabel.setBounds(tooltipX, 0, tooltipWidth, headerHeight);
    _tooltipLabel.setFont(juce::Font("Helvetica", juce::jmax(getWidth() * 0.015, 10.0), juce::Font::FontStyleFlags::bold));
    
    // io
    auto faderX = getWidth() * 0.125;
    const auto faderY = getHeight() * 0.26;
    const auto faderWidth  = getWidth() * 0.075;
    const auto faderHeight  = getHeight() * 0.6;
    const auto faderPadding = 9.0;
    for (auto& fader : _ioFaders)
    {
        fader->setBounds(faderX, faderY, faderWidth, faderHeight);
        fader->setSliderTextBoxWidth(faderWidth);
        faderX += faderWidth * faderPadding;
    }
    
    // vinyl
    auto vinylX = _ioFaders[0]->getRight() * 1.1;
    const auto vinylY = getHeight() * 0.58;
    const auto vinylWidth  = getWidth() * 0.11;
    const auto vinylPadding = 1.36;
    for (auto& dial : _vinylDials)
    {
        dial->setBounds(vinylX, vinylY, vinylWidth, vinylWidth);
        dial->setSliderTextBoxWidth(vinylWidth * 0.8);
        vinylX += vinylWidth * vinylPadding;
    }
    
    // buttons
    auto btnX = getWidth() * 0.4;
    const auto btnY = getHeight() * 0.83;
    const auto btnWidth  = getWidth() * 0.065;
    for (auto& btn : _buttons)
    {
        btn->setBounds(btnX, btnY, btnWidth, btnWidth);
        btnX += btnWidth;
    }
    
    // vu
    const auto vuX = getWidth() * 0.345;
    const auto vuY = getHeight() * 0.18;
    const auto vuWidth = getWidth() * 0.3;
    const auto vuHeight = getHeight() * 0.3;
    _vuMeter.setBounds(vuX, vuY, vuWidth, vuHeight);
    
    // Save plugin size in value tree
    savePluginBounds();
}

void ViatorrustAudioProcessorEditor::setIOSliderProps()
{
    const auto image = juce::ImageCache::getFromMemory(BinaryData::Ver_slider_png, BinaryData::Ver_slider_pngSize);
    const auto numFrames = 256;
    const auto params = audioProcessor._parameterMap.getIOSliderParams();
    
    for (int i = 0; i < params.size(); i++)
    {
        _ioFaders.add(std::make_unique<viator_gui::ImageFader>());
        _ioAttachments.add(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor._treeState, params[i].paramID, *_ioFaders[i]));
        _ioFaders[i]->setFaderImageAndNumFrames(image, numFrames);
        _ioFaders[i]->setName(params[i].paramName);
        _ioFaders[i]->setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        _ioFaders[i]->addMouseListener(this, false);
        addAndMakeVisible(*_ioFaders[i]);
    }
}

void ViatorrustAudioProcessorEditor::setVinylDialProps()
{
    const auto image = juce::ImageCache::getFromMemory(BinaryData::Knob_mid_png, BinaryData::Knob_mid_pngSize);
    const auto numFrames = 256;
    const auto params = audioProcessor._parameterMap.getVinylSliderParams();
    
    for (int i = 0; i < params.size(); i++)
    {
        _vinylDials.add(std::make_unique<viator_gui::ImageFader>());
        _vinylAttachments.add(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor._treeState, params[i].paramID, *_vinylDials[i]));
        _vinylDials[i]->setFaderImageAndNumFrames(image, numFrames);
        _vinylDials[i]->setName(params[i].paramName);
        _vinylDials[i]->addMouseListener(this, false);
        _vinylDials[i]->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        addAndMakeVisible(*_vinylDials[i]);
    }
}

void ViatorrustAudioProcessorEditor::setButtonProps()
{
    const auto imageOff = juce::ImageCache::getFromMemory(BinaryData::pad_off_png, BinaryData::pad_off_pngSize);
    const auto imageOn = juce::ImageCache::getFromMemory(BinaryData::pad_on_png, BinaryData::pad_on_pngSize);
    const auto params = audioProcessor._parameterMap.getButtonParams();
    
    for (int i = 0; i < params.size(); i++)
    {
        _buttons.add(std::make_unique<viator_gui::ImageButton>(imageOff, imageOn, params[i].paramName, params[i].paramName));
        _buttonAttachments.add(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor._treeState, params[i].paramID, _buttons[i]->getButton()));
        _buttons[i]->getButton().addMouseListener(this, true);
        addAndMakeVisible(*_buttons[i]);
    }
}

void ViatorrustAudioProcessorEditor::timerCallback()
{
    _vuMeter.getVUMeter().setValue(audioProcessor.getCurrentPeakSignal());
    
    if (audioProcessor.getNumChannels() < 2)
    {
        _buttons[1]->setVisible(false);
    }
    
    else
    {
        _buttons[1]->setVisible(true);
    }
}

void ViatorrustAudioProcessorEditor::savePluginBounds()
{
    audioProcessor.variableTree.setProperty("width", getWidth(), nullptr);
    audioProcessor.variableTree.setProperty("height", getHeight(), nullptr);
    audioProcessor._width = getWidth();
    audioProcessor._height = getHeight();
}

void ViatorrustAudioProcessorEditor::mouseEnter(const juce::MouseEvent &event)
{
    for (int i = 0; i < _vinylDials.size(); ++i)
    {
        if (event.eventComponent == _vinylDials[i])
        {
            _tooltipLabel.setText(_vinylDialsTooltips[i], juce::dontSendNotification);
        }
    }
    
    for (int i = 0; i < _ioFaders.size(); ++i)
    {
        if (event.eventComponent == _ioFaders[i])
        {
            _tooltipLabel.setText(_ioTooltips[i], juce::dontSendNotification);
        }
    }
    
    for (int i = 0; i < _buttons.size(); ++i)
    {
        if (event.eventComponent == &_buttons[i]->getButton())
        {
            _tooltipLabel.setText(_buttonTooltips[i], juce::dontSendNotification);
        }
    }
}

void ViatorrustAudioProcessorEditor::mouseExit(const juce::MouseEvent &event)
{
    for (int i = 0; i < _vinylDials.size(); ++i)
    {
        if (event.eventComponent == _vinylDials[i])
        {
            _tooltipLabel.setText("", juce::dontSendNotification);
        }
    }
    
    for (int i = 0; i < _ioFaders.size(); ++i)
    {
        if (event.eventComponent == _ioFaders[i])
        {
            _tooltipLabel.setText("", juce::dontSendNotification);
        }
    }
    
    for (int i = 0; i < _buttons.size(); ++i)
    {
        if (event.eventComponent == &_buttons[i]->getButton())
        {
            _tooltipLabel.setText("", juce::dontSendNotification);
        }
    }
}
