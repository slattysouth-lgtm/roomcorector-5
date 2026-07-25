/*
  ==============================================================================
   PluginProcessor.cpp  —  Room Corrector (Uncertain)  v6

   Signal path :
   [Dry]──► DelayLine (1536 smpl) ────────────────────────────────►──┐
   [Wet]  Gate[c] ► FFT[c](clean+repair+tone) ► Enhance[c] ► MixReady ──► blend ► out
  ==============================================================================
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RoomCorrectorAudioProcessor::RoomCorrectorAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{}

juce::AudioProcessorValueTreeState::ParameterLayout
    RoomCorrectorAudioProcessor::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;
    auto pf = [&] (const char* id, const char* name,
                   float lo, float hi, float def)
    {
        layout.add (std::make_unique<AudioParameterFloat>(
            ParameterID{id,1}, name,
            NormalisableRange<float>(lo,hi,0.1f), def));
    };

    pf("clean",    "Clean",     0.f,100.f,  0.f);  // Gate+Denoise+Dereverb+DeClick
    pf("repair",   "Repair",    0.f,100.f,  0.f);  // Proximity fix + micro trop proche
    pf("tone",     "Tone",      0.f,100.f,  0.f);  // Air adaptatif + Corps (F0)
    pf("comp",     "Comp",      0.f,100.f,  0.f);  // Compresseur + Limiteur
    pf("sibilance","Sibilance", 0.f,100.f,  0.f);  // De-esser
    pf("mix",      "Mix",       0.f,100.f,  0.f);  // Dry/Wet
    pf("output",   "Output",  -24.f, 24.f,  0.f);  // Gain de sortie (dB)

    // ── Mode ADVANCED : reglages fins (defaut neutre) ──
    pf("body",     "Body",     -6.f,  6.f,  0.f);  // trim corps (dB)
    pf("presence", "Presence", -6.f,  6.f,  0.f);  // trim presence (dB)
    pf("air",      "Air",      -6.f,  6.f,  0.f);  // trim air (dB)
    pf("sibfreq",  "DeEss Freq",5000.f,9000.f,7000.f); // frequence de-esser (Hz)

    layout.add (std::make_unique<AudioParameterBool>(
        ParameterID{"bypass",1}, "Bypass", false));

    return layout;
}

//==============================================================================
void RoomCorrectorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    for (int c=0;c<2;++c)
    {
        cleaner[c].prepare(sampleRate);
        enhancer[c].prepare(sampleRate);
        gate[c].prepare(sampleRate);
    }
    mixReady.prepare(sampleRate,2);

    const int maxBlock=juce::jmax(512,samplesPerBlock);
    dryDelay.prepare(SpectralCleaner::latency,maxBlock);
    dryDelay.setDelay(SpectralCleaner::latency);

    dryBuf.setSize(2,maxBlock,false,true,false);

    const double ramp=0.020;
    auto initSm=[&](juce::SmoothedValue<float>& sm, float v)
    { sm.reset(sampleRate,ramp); sm.setCurrentAndTargetValue(v); };

    initSm(cleanSm,  0.00f);
    initSm(repairSm, 0.00f);
    initSm(toneSm,   0.00f);
    initSm(compSm,   0.00f);
    initSm(sibSm,    0.00f);
    initSm(mixSm,    0.00f);
    initSm(outSm,    1.00f);

    setLatencySamples(SpectralCleaner::latency);
}

bool RoomCorrectorAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto in=l.getMainInputChannelSet(), out=l.getMainOutputChannelSet();
    if (in!=out) return false;
    return in==juce::AudioChannelSet::mono()||in==juce::AudioChannelSet::stereo();
}

//==============================================================================
void RoomCorrectorAudioProcessor::processBlock (juce::AudioBuffer<float>& buf,
                                                juce::MidiBuffer&)
{
    juce::ScopedNoDenormals nd;
    const int totalIn =getTotalNumInputChannels();
    const int totalOut=getTotalNumOutputChannels();
    const int n=buf.getNumSamples();
    for (int i=totalIn;i<totalOut;++i) buf.clear(i,0,n);

    if (apvts.getRawParameterValue("bypass")->load()>0.5f) return;

    const int numCh=juce::jmin(2,buf.getNumChannels());

    // Cibles des lisseurs
    cleanSm .setTargetValue(apvts.getRawParameterValue("clean")->load()    *0.01f);
    repairSm.setTargetValue(apvts.getRawParameterValue("repair")->load()   *0.01f);
    toneSm  .setTargetValue(apvts.getRawParameterValue("tone")->load()     *0.01f);
    compSm  .setTargetValue(apvts.getRawParameterValue("comp")->load()     *0.01f);
    sibSm   .setTargetValue(apvts.getRawParameterValue("sibilance")->load()*0.01f);
    mixSm   .setTargetValue(apvts.getRawParameterValue("mix")->load()      *0.01f);
    outSm   .setTargetValue(juce::Decibels::decibelsToGain(
                            apvts.getRawParameterValue("output")->load()));

    // ── 1. Stocke le signal dry retarde dans dryBuf ───────────────────────
    for (int s=0;s<n;++s)
    {
        for (int c=0;c<numCh;++c)
            dryBuf.setSample(c,s,dryDelay.read(c,buf.getSample(c,s)));
        dryDelay.advance();
    }

    // ── 2. Gate + FFT (clean+repair+tone) + Enhance ──────────────────────
    const float trBody=apvts.getRawParameterValue("body")->load();
    const float trPres=apvts.getRawParameterValue("presence")->load();
    const float trAir =apvts.getRawParameterValue("air")->load();
    for (int s=0;s<n;++s)
    {
        const float ca=cleanSm .getNextValue();
        const float ra=repairSm.getNextValue();
        const float ta=toneSm  .getNextValue();

        for (int c=0;c<numCh;++c)
        {
            float x=buf.getSample(c,s);
            x=gate[c].process(x,ca);
            x=cleaner[c].processSample(x,ca,ra,ta);
            x=enhancer[c].processSample(x,ta,trBody,trPres,trAir);
            buf.setSample(c,s,x);
        }
    }

    // ── 3. MixReady (bloc, detection stereo liee) ─────────────────────────
    // Les smoothers comp/sib sont avances dans le processBlock du MixReady.
    // On passe les valeurs cibles — MixReady les lisse en interne.
    {
        const float cA=compSm.getNextValue();
        const float sA=sibSm .getNextValue();
        // Rembobine les smoothers au debut du prochain bloc (avance deja consomme)
        compSm.setCurrentAndTargetValue(cA);
        sibSm .setCurrentAndTargetValue(sA);
        mixReady.process(buf,cA,sA,apvts.getRawParameterValue("sibfreq")->load());
    }

    // ── 4. Blend dry/wet + gain de sortie + securite true-peak ────────────
    const float tpCeil=0.89f; // ~ -1 dBFS
    for (int s=0;s<n;++s)
    {
        const float mw=mixSm.getNextValue();
        const float g =outSm.getNextValue();
        for (int c=0;c<numCh;++c)
        {
            const float wet=buf.getSample(c,s);
            const float dry=dryBuf.getSample(c,s);
            float out=(dry*(1.0f-mw)+wet*mw)*g;

            // Plafond de securite TOUJOURS actif : rien ne depasse -1 dBFS
            const float a=std::fabs(out);
            if (a>tpCeil) { const float gg=tpCeil/a; if (gg<tpGain) tpGain=gg; }
            tpGain+=(1.0f-tpGain)*0.0009f;
            out=juce::jlimit(-tpCeil,tpCeil,out*tpGain);

            buf.setSample(c,s,out);
        }
    }

   #if defined(RC_TRIAL)
    // ── VERSION D'ESSAI : son complet 60 s, coupure douce 2 s, en boucle ──
    {
        const double srD=getSampleRate();
        const int muteStart=(int)(srD*60.0);
        const int period   =(int)(srD*62.0);
        for (int s=0;s<n;++s)
        {
            const float tgt=(trialCtr>=muteStart)?0.0f:1.0f;
            trialGain+=(tgt-trialGain)*0.0015f;   // fondu, jamais de clic
            for (int c=0;c<numCh;++c)
                buf.setSample(c,s,buf.getSample(c,s)*trialGain);
            if (++trialCtr>=period) trialCtr=0;
        }
    }
   #endif
}

//==============================================================================
juce::AudioProcessorEditor* RoomCorrectorAudioProcessor::createEditor()
{ return new RoomCorrectorAudioProcessorEditor(*this); }

void RoomCorrectorAudioProcessor::getStateInformation (juce::MemoryBlock& d)
{ if (auto x=apvts.copyState().createXml()) copyXmlToBinary(*x,d); }

void RoomCorrectorAudioProcessor::setStateInformation (const void* d, int sz)
{
    if (auto x=getXmlFromBinary(d,sz))
        if (x->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*x));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{ return new RoomCorrectorAudioProcessor(); }
