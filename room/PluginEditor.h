/*
  ==============================================================================
   PluginEditor.h  —  Room Corrector (Uncertain)  v6
   SIMPLE (6 knobs ronds) / ADVANCED (courbe + reglages fins) + 3 presets
  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class CorrectorLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CorrectorLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float pos, float startAngle, float endAngle,
                           juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool, bool) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h,
                           float pos, float minPos, float maxPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;
};

//==============================================================================
// Bouton "pilule" (toggle SIMPLE/ADVANCED et presets)
class PillButton : public juce::Button
{
public:
    PillButton (const juce::String& text, juce::Colour accent)
        : juce::Button(text), accentCol(accent) {}
    void paintButton (juce::Graphics&, bool, bool) override;
    bool selected=false;
private:
    juce::Colour accentCol;
};

//==============================================================================
// Courbe de correction temps reel (mode ADVANCED)
class CurveView : public juce::Component, private juce::Timer
{
public:
    explicit CurveView (juce::AudioProcessorValueTreeState& s) : apvts(s)
    { startTimerHz(20); }
    void paint (juce::Graphics&) override;
private:
    void timerCallback() override { repaint(); }
    float respDb (float f) const;
    float sibDb  (float f) const;
    juce::AudioProcessorValueTreeState& apvts;
};

//==============================================================================
class RoomCorrectorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit RoomCorrectorAudioProcessorEditor (RoomCorrectorAudioProcessor&);
    ~RoomCorrectorAudioProcessorEditor() override;
    void paint   (juce::Graphics&) override;
    void resized () override;

private:
    using Att = juce::AudioProcessorValueTreeState;

    RoomCorrectorAudioProcessor& proc;
    CorrectorLookAndFeel lnf;

    // 6 knobs + output
    juce::Slider   cleanK, repairK, toneK, compK, sibK, mixK, outputK;
    juce::Label    cleanL, repairL, toneL, compL, sibL, mixL, outputL;
    juce::ToggleButton bypassBtn { "BYPASS" };

    std::unique_ptr<Att::SliderAttachment> cleanA, repairA, toneA, compA, sibA, mixA, outputA;
    std::unique_ptr<Att::ButtonAttachment> bypassA;

    // Mode SIMPLE / ADVANCED
    PillButton simpleBtn { "SIMPLE",   juce::Colour(0xffC0203A) };
    PillButton advBtn    { "ADVANCED", juce::Colour(0xffC0203A) };
    bool advanced=false;
    void setAdvanced (bool);

    // Presets
    PillButton presetBandlab { "BANDLAB",     juce::Colour(0xffC0203A) };
    PillButton presetHome    { "HOME STUDIO", juce::Colour(0xffE08A30) };
    PillButton presetPro     { "STUDIO PRO",  juce::Colour(0xff3FB950) };
    void applyPreset (float cl,float rp,float tn,float cp,float sb,float mx);

    // Zone ADVANCED
    CurveView curve;
    juce::Slider bodyS, presS, airS, sibFS;
    juce::Label  bodyL, presL, airL, sibFL;
    std::unique_ptr<Att::SliderAttachment> bodyA2, presA2, airA2, sibFA2;

    void initKnob (juce::Slider&, juce::Label&, const juce::String& name);
    void initTrim (juce::Slider&, juce::Label&, const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoomCorrectorAudioProcessorEditor)
};
