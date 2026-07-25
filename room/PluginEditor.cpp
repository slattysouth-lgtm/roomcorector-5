/*
  ==============================================================================
   PluginEditor.cpp  —  Room Corrector (Uncertain)  v6
   Knobs ronds simples · SIMPLE/ADVANCED · courbe temps reel · 3 presets
  ==============================================================================
*/
#include "PluginEditor.h"

namespace Col
{
    const juce::Colour bg      (0xff0A0A0D);
    const juce::Colour surface (0xff111114);
    const juce::Colour border  (0xff1E1E24);
    const juce::Colour body    (0xff141418);
    const juce::Colour text    (0xffEDEDEF);
    const juce::Colour sub     (0xff6A6A74);
    const juce::Colour track   (0xff202026);
    const juce::Colour red     (0xffC0203A);
    const juce::Colour redHi   (0xffE03050);
}

//==============================================================================
CorrectorLookAndFeel::CorrectorLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId,    Col::sub);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::textColourId,            Col::text);
    setColour(juce::ToggleButton::textColourId,     Col::sub);
    setColour(juce::Slider::trackColourId,          Col::red);
    setColour(juce::Slider::backgroundColourId,     Col::track);
    setColour(juce::Slider::thumbColourId,          Col::redHi);
}

// ── KNOB ROND & SIMPLE : cercle doux, point rouge sur la couronne, fin arc ──
void CorrectorLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                             int x,int y,int w,int h,
                                             float pos,
                                             float startAngle,float endAngle,
                                             juce::Slider&)
{
    const auto  b   = juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(6.0f);
    const float rad = juce::jmin(b.getWidth(),b.getHeight())*0.5f;
    const auto  ctr = b.getCentre();
    const float ang = startAngle+pos*(endAngle-startAngle);
    const float lw  = juce::jmax(2.0f,rad*0.075f);
    const float arcR= rad-lw*0.5f;
    const float knR = rad-lw*2.6f;

    // Arc fond + arc actif (fins)
    juce::Path back;
    back.addCentredArc(ctr.x,ctr.y,arcR,arcR,0.f,startAngle,endAngle,true);
    g.setColour(Col::track);
    g.strokePath(back,juce::PathStrokeType(lw,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));

    if (pos>0.001f)
    {
        juce::Path arc;
        arc.addCentredArc(ctr.x,ctr.y,arcR,arcR,0.f,startAngle,ang,true);
        g.setColour(Col::red);
        g.strokePath(arc,juce::PathStrokeType(lw,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    }

    // Corps rond, degrade doux
    {
        juce::ColourGradient grad(juce::Colour(0xff1B1B20),ctr.x-knR*0.4f,ctr.y-knR*0.6f,
                                  juce::Colour(0xff101014),ctr.x+knR*0.5f,ctr.y+knR*0.8f,true);
        g.setGradientFill(grad);
        g.fillEllipse(juce::Rectangle<float>(knR*2,knR*2).withCentre(ctr));
        g.setColour(Col::border);
        g.drawEllipse(juce::Rectangle<float>(knR*2,knR*2).withCentre(ctr),1.0f);
    }

    // Point rouge sur la couronne (indicateur) + halo
    {
        const float pr=knR-6.0f;
        const juce::Point<float> p(ctr.x+pr*std::sin(ang),ctr.y-pr*std::cos(ang));
        g.setColour(Col::redHi.withAlpha(0.14f));
        g.fillEllipse(juce::Rectangle<float>(15.0f,15.0f).withCentre(p));
        g.setColour(Col::redHi);
        g.fillEllipse(juce::Rectangle<float>(8.0f,8.0f).withCentre(p));
    }
}

void CorrectorLookAndFeel::drawToggleButton (juce::Graphics& g,
                                             juce::ToggleButton& btn,
                                             bool, bool)
{
    const bool on=btn.getToggleState();
    const auto b=btn.getLocalBounds().toFloat();
    g.setColour(on?Col::red.withAlpha(0.18f):Col::surface);
    g.fillRoundedRectangle(b,5.0f);
    g.setColour(on?Col::red:Col::border);
    g.drawRoundedRectangle(b.reduced(0.5f),5.0f,1.0f);
    g.setColour(on?Col::redHi:Col::sub);
    g.setFont(juce::Font(10.0f,juce::Font::bold));
    g.drawText(btn.getButtonText(),b,juce::Justification::centred);
}

// Sliders verticaux fins (trims ADVANCED)
void CorrectorLookAndFeel::drawLinearSlider (juce::Graphics& g,int x,int y,int w,int h,
                                             float pos,float,float,
                                             juce::Slider::SliderStyle,juce::Slider&)
{
    const float cx=(float)x+(float)w*0.5f;
    g.setColour(Col::track);
    g.fillRoundedRectangle(cx-2.0f,(float)y,4.0f,(float)h,2.0f);
    g.setColour(Col::red);
    g.fillRoundedRectangle(cx-2.0f,pos,4.0f,(float)y+(float)h-pos,2.0f);
    g.setColour(Col::redHi);
    g.fillEllipse(juce::Rectangle<float>(13.0f,13.0f).withCentre({cx,pos}));
}

//==============================================================================
void PillButton::paintButton (juce::Graphics& g, bool over, bool)
{
    const auto b=getLocalBounds().toFloat();
    g.setColour(selected?accentCol:Col::bg);
    g.fillRoundedRectangle(b,b.getHeight()*0.5f);
    g.setColour(selected?accentCol:(over?Col::sub:Col::border));
    g.drawRoundedRectangle(b.reduced(0.5f),b.getHeight()*0.5f,1.0f);
    g.setColour(selected?juce::Colours::white:Col::sub);
    g.setFont(juce::Font(9.5f,juce::Font::bold));
    g.drawText(getButtonText(),b,juce::Justification::centred);
}

//==============================================================================
// Courbe de correction (approximation visuelle du moteur)
float CurveView::respDb (float f) const
{
    auto v=[&](const char* id){ return apvts.getRawParameterValue(id)->load(); };
    float db=0.0f;
    // CLEAN → passe-haut
    const float fc=20.0f+(v("clean")*0.01f)*90.0f;
    const float w=f/fc; const float w4=w*w*w*w;
    db+=10.0f*std::log10(w4/(1.0f+w4));
    // REPAIR → plateau grave attenue
    db+=-(v("repair")*0.01f)*5.0f/(1.0f+std::pow(f/250.0f,2.0f));
    // TONE decompose + trims
    const float t=v("tone")*0.01f;
    const float body=t*2.5f+v("body");
    const float pres=t*3.0f+v("presence");
    const float air =t*6.0f+v("air");
    db+=body/(1.0f+std::pow(f/200.0f,2.0f));
    const float lg=std::log(f/3000.0f);
    db+=pres*std::exp(-(lg*lg)/(2.0f*0.55f*0.55f));
    const float sf=v("sibfreq");
    db+=air/(1.0f+std::pow(sf*1.3f/f,2.0f))*(f>2000.0f?1.0f:0.0f)*juce::jmin(1.0f,f/9000.0f+0.4f);
    return db;
}
float CurveView::sibDb (float f) const
{
    auto v=[&](const char* id){ return apvts.getRawParameterValue(id)->load(); };
    const float s=-(v("sibilance")*0.01f)*10.0f;
    const float sf=v("sibfreq");
    const float lg=std::log(f/sf);
    return s*std::exp(-(lg*lg)/(2.0f*0.35f*0.35f));
}
void CurveView::paint (juce::Graphics& g)
{
    const auto b=getLocalBounds().toFloat();
    g.setColour(Col::bg);
    g.fillRoundedRectangle(b,10.0f);
    g.setColour(Col::border);
    g.drawRoundedRectangle(b.reduced(0.5f),10.0f,1.0f);

    const float pad=10.0f;
    const float W=b.getWidth(),H=b.getHeight();
    const float fMin=20.0f,fMax=20000.0f,dbSpan=28.0f;
    auto fx=[&](float f){ return pad+std::log10(f/fMin)/std::log10(fMax/fMin)*(W-2*pad); };
    auto fy=[&](float db){ return H*0.5f-db/dbSpan*(H-2*pad); };

    // grille
    g.setColour(juce::Colour(0xff17171C));
    for (float f : {50.f,100.f,200.f,500.f,1000.f,2000.f,5000.f,10000.f})
        g.drawVerticalLine((int)fx(f),pad,H-pad);
    g.setColour(juce::Colour(0xff26262E));
    g.drawHorizontalLine((int)fy(0.0f),pad,W-pad);

    // labels frequence
    g.setColour(juce::Colour(0xff3A3A42));
    g.setFont(juce::Font(7.5f));
    for (float f : {100.f,1000.f,10000.f})
        g.drawText(f>=1000.f?juce::String((int)(f/1000))+"k":juce::String((int)f),
                   (int)fx(f)-12,(int)(H-13),24,10,juce::Justification::centred);

    // courbes
    juce::Path main,sib;
    const int N=100;
    for (int i=0;i<=N;++i)
    {
        const float f=fMin*std::pow(fMax/fMin,(float)i/(float)N);
        const float d =juce::jlimit(-dbSpan*0.5f,dbSpan*0.5f,respDb(f));
        const float d2=juce::jlimit(-dbSpan*0.5f,dbSpan*0.5f,respDb(f)+sibDb(f));
        if (i==0){ main.startNewSubPath(fx(f),fy(d)); sib.startNewSubPath(fx(f),fy(d2)); }
        else     { main.lineTo(fx(f),fy(d));          sib.lineTo(fx(f),fy(d2)); }
    }
    g.setColour(Col::redHi.withAlpha(0.45f));
    const float dash[2]={3.0f,4.0f};
    juce::Path sibDashed; juce::PathStrokeType(1.2f).createDashedStroke(sibDashed,sib,dash,2);
    g.fillPath(sibDashed);
    g.setColour(Col::redHi);
    g.strokePath(main,juce::PathStrokeType(2.0f));
}

//==============================================================================
void RoomCorrectorAudioProcessorEditor::initKnob (juce::Slider& s, juce::Label& l,
                                                  const juce::String& name)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,64,15);
    s.setRotaryParameters(juce::MathConstants<float>::pi*1.25f,
                          juce::MathConstants<float>::pi*2.75f,true);
    s.setDoubleClickReturnValue(true,0.0);
    addAndMakeVisible(s);

    l.setText(name,juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(11.0f,juce::Font::bold));
    l.setColour(juce::Label::textColourId,Col::text);
    addAndMakeVisible(l);
}

void RoomCorrectorAudioProcessorEditor::initTrim (juce::Slider& s, juce::Label& l,
                                                  const juce::String& name)
{
    s.setSliderStyle(juce::Slider::LinearVertical);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,58,14);
    s.setDoubleClickReturnValue(true,0.0);
    addChildComponent(s);

    l.setText(name,juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(8.5f));
    l.setColour(juce::Label::textColourId,Col::sub);
    addChildComponent(l);
}

//==============================================================================
RoomCorrectorAudioProcessorEditor::RoomCorrectorAudioProcessorEditor (RoomCorrectorAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p), curve(p.apvts)
{
    setLookAndFeel(&lnf);

    initKnob(cleanK, cleanL, "CLEAN");
    initKnob(repairK,repairL,"REPAIR");
    initKnob(toneK,  toneL,  "TONE");
    initKnob(compK,  compL,  "COMP");
    initKnob(sibK,   sibL,   "SIBILANCE");
    initKnob(mixK,   mixL,   "MIX");
    initKnob(outputK,outputL,"OUTPUT");
    addAndMakeVisible(bypassBtn);

    using A=juce::AudioProcessorValueTreeState;
    cleanA  = std::make_unique<A::SliderAttachment>(proc.apvts,"clean",    cleanK);
    repairA = std::make_unique<A::SliderAttachment>(proc.apvts,"repair",   repairK);
    toneA   = std::make_unique<A::SliderAttachment>(proc.apvts,"tone",     toneK);
    compA   = std::make_unique<A::SliderAttachment>(proc.apvts,"comp",     compK);
    sibA    = std::make_unique<A::SliderAttachment>(proc.apvts,"sibilance",sibK);
    mixA    = std::make_unique<A::SliderAttachment>(proc.apvts,"mix",      mixK);
    outputA = std::make_unique<A::SliderAttachment>(proc.apvts,"output",   outputK);
    bypassA = std::make_unique<A::ButtonAttachment>(proc.apvts,"bypass",   bypassBtn);

    // ── Mode SIMPLE / ADVANCED ──
    addAndMakeVisible(simpleBtn);
    addAndMakeVisible(advBtn);
    simpleBtn.selected=true;
    simpleBtn.onClick=[this]{ setAdvanced(false); };
    advBtn.onClick   =[this]{ setAdvanced(true);  };

    // ── Presets ──
    addAndMakeVisible(presetBandlab);
    addAndMakeVisible(presetHome);
    addAndMakeVisible(presetPro);
    presetBandlab.onClick=[this]{ applyPreset(75,60,70,65,55,100); };
    presetHome.onClick   =[this]{ applyPreset(45,30,55,45,35,100); };
    presetPro.onClick    =[this]{ applyPreset(20,10,35,25,20,100); };

    // ── Zone ADVANCED ──
    addChildComponent(curve);
    initTrim(bodyS,bodyL,"BODY");
    initTrim(presS,presL,"PRESENCE");
    initTrim(airS, airL, "AIR");
    initTrim(sibFS,sibFL,"DE-ESS FRQ");
    bodyA2 = std::make_unique<A::SliderAttachment>(proc.apvts,"body",    bodyS);
    presA2 = std::make_unique<A::SliderAttachment>(proc.apvts,"presence",presS);
    airA2  = std::make_unique<A::SliderAttachment>(proc.apvts,"air",     airS);
    sibFA2 = std::make_unique<A::SliderAttachment>(proc.apvts,"sibfreq", sibFS);

    setSize(660,470);
}

RoomCorrectorAudioProcessorEditor::~RoomCorrectorAudioProcessorEditor()
{ setLookAndFeel(nullptr); }

void RoomCorrectorAudioProcessorEditor::setAdvanced (bool on)
{
    advanced=on;
    simpleBtn.selected=!on;
    advBtn.selected=on;
    simpleBtn.repaint(); advBtn.repaint();

    curve.setVisible(on);
    for (auto* c : { &bodyS,&presS,&airS,&sibFS })  c->setVisible(on);
    for (auto* l : { &bodyL,&presL,&airL,&sibFL })  l->setVisible(on);

    setSize(660, on ? 680 : 470);
}

void RoomCorrectorAudioProcessorEditor::applyPreset (float cl,float rp,float tn,
                                                     float cp,float sb,float mx)
{
    auto set=[&](const char* id,float v)
    {
        if (auto* prm=proc.apvts.getParameter(id))
        {
            prm->beginChangeGesture();
            prm->setValueNotifyingHost(prm->convertTo0to1(v));
            prm->endChangeGesture();
        }
    };
    set("clean",cl); set("repair",rp); set("tone",tn);
    set("comp",cp);  set("sibilance",sb); set("mix",mx);
}

//==============================================================================
void RoomCorrectorAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(Col::bg);
    g.setColour(Col::surface);
    g.fillRoundedRectangle(getLocalBounds().reduced(10).toFloat(),12.0f);

    g.setFont(juce::Font(20.0f,juce::Font::bold));
    g.setColour(Col::text);
    g.drawText("ROOM CORRECTOR",28,18,280,24,juce::Justification::left);

    g.setFont(juce::Font(10.0f));
    g.setColour(Col::sub);
    g.drawText("by Uncertain  ·  VST3 64-bit  ·  Voice",28,42,340,14,juce::Justification::left);

    g.setColour(Col::red);
    g.fillRect(juce::Rectangle<float>(28.0f,59.0f,64.0f,1.5f));

    if (advanced)
    {
        g.setFont(juce::Font(8.5f));
        g.setColour(Col::sub.withAlpha(0.7f));
        g.drawText("COURBE DE CORRECTION · TEMPS RÉEL",28,438,260,12,juce::Justification::left);
        g.drawText("RÉGLAGES FINS",28,576,200,12,juce::Justification::left);
    }

    g.setFont(juce::Font(8.5f));
    g.setColour(Col::sub.withAlpha(0.5f));
    g.drawText("GATE · DENOISE · DEREVERB · DE-CLICK · PROXIMITY · TONE[F0] · CORPS · AIR · COMP · DE-ESS",
               0,getHeight()-18,getWidth(),16,juce::Justification::centred);

   #if defined(RC_TRIAL)
    g.setColour(Col::redHi);
    g.setFont(juce::Font(9.5f,juce::Font::bold));
    g.drawText("VERSION D'ESSAI — le son se coupe 2 s toutes les 60 s — version complète : uncertain.fr",
               0,getHeight()-34,getWidth(),14,juce::Justification::centred);
   #endif
}

void RoomCorrectorAudioProcessorEditor::resized()
{
    const int W=getWidth();

    // Header : toggle + bypass
    simpleBtn.setBounds(W-238,20,64,22);
    advBtn   .setBounds(W-170,20,84,22);
    bypassBtn.setBounds(W-78, 20,50,22);

    // Presets
    const int py=68;
    presetBandlab.setBounds(28,       py,96, 22);
    presetHome   .setBounds(28+102,   py,110,22);
    presetPro    .setBounds(28+218,   py,104,22);

    // OUTPUT compact a droite de la rangee presets
    outputL.setBounds(W-150,py-14,120,12);
    outputK.setBounds(W-108,py-16,80, 52);

    // Grille 2x3
    const int gy=104;
    const int kw=(W-44)/3;
    const int rowH=158;
    auto place=[&](juce::Slider& k,juce::Label& l,int col,int row)
    {
        const int x=22+col*kw, y=gy+row*rowH;
        l.setBounds(x,y,kw,16);
        k.setBounds(x,y+16,kw,rowH-22);
    };
    place(cleanK,cleanL,0,0); place(repairK,repairL,1,0); place(toneK,toneL,2,0);
    place(compK,compL,0,1);   place(sibK,sibL,1,1);       place(mixK,mixL,2,1);

    // Zone ADVANCED
    if (advanced)
    {
        curve.setBounds(24,452,W-48,118);
        const int tw=(W-48)/4;
        const int ty=590;
        auto placeT=[&](juce::Slider& s,juce::Label& l,int i)
        {
            const int x=24+i*tw;
            l.setBounds(x,ty,tw,12);
            s.setBounds(x+tw/2-14,ty+13,28,58);
        };
        placeT(bodyS,bodyL,0); placeT(presS,presL,1);
        placeT(airS,airL,2);   placeT(sibFS,sibFL,3);
    }
}
