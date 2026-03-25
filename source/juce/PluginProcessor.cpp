#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Preset.h"
#include "synth/AudioBuffer.h"
#include "synth/NoiseGenerator.h"
#include <span>
#include <vector>

namespace
{

const juce::Identifier pluginTag{"PLUGIN"};
const juce::Identifier extraTag{"EXTRA"};
const juce::Identifier midiCCAttribute{"midiCC"};

constexpr uint8_t operator""_midi(unsigned long long value) noexcept
{
    return static_cast<uint8_t>(value);
}

template <typename T>
inline static void castParameter(juce::AudioProcessorValueTreeState& apvts, const juce::ParameterID& id, T& destination)
{
    destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
    jassert(destination && "parameter does not exist or wrong type");
}

} // namespace

//==============================================================================
SynpleAudioProcessor::SynpleAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
{
    castParameter(apvts_, parameter_id::oscMix, oscMixParam_);
    castParameter(apvts_, parameter_id::oscTune, oscTuneParam_);
    castParameter(apvts_, parameter_id::oscFine, oscFineParam_);
    castParameter(apvts_, parameter_id::glideMode, glideModeParam_);
    castParameter(apvts_, parameter_id::glideRate, glideRateParam_);
    castParameter(apvts_, parameter_id::glideBend, glideBendParam_);
    castParameter(apvts_, parameter_id::filterFreq, filterFreqParam_);
    castParameter(apvts_, parameter_id::filterReso, filterResoParam_);
    castParameter(apvts_, parameter_id::filterEnv, filterEnvParam_);
    castParameter(apvts_, parameter_id::filterLFO, filterLFOParam_);
    castParameter(apvts_, parameter_id::filterVelocity, filterVelocityParam_);
    castParameter(apvts_, parameter_id::filterAttack, filterAttackParam_);
    castParameter(apvts_, parameter_id::filterDecay, filterDecayParam_);
    castParameter(apvts_, parameter_id::filterSustain, filterSustainParam_);
    castParameter(apvts_, parameter_id::filterRelease, filterReleaseParam_);
    castParameter(apvts_, parameter_id::envAttack, envAttackParam_);
    castParameter(apvts_, parameter_id::envDecay, envDecayParam_);
    castParameter(apvts_, parameter_id::envSustain, envSustainParam_);
    castParameter(apvts_, parameter_id::envRelease, envReleaseParam_);
    castParameter(apvts_, parameter_id::lfoRate, lfoRateParam_);
    castParameter(apvts_, parameter_id::vibrato, vibratoParam_);
    castParameter(apvts_, parameter_id::noise, noiseParam_);
    castParameter(apvts_, parameter_id::octave, octaveParam_);
    castParameter(apvts_, parameter_id::tuning, tuningParam_);
    castParameter(apvts_, parameter_id::outputLevel, outputLevelParam_);
    castParameter(apvts_, parameter_id::polyMode, polyModeParam_);

    apvts_.state.addListener(this);

    createPrograms();
    setCurrentProgram(0);
}

SynpleAudioProcessor::~SynpleAudioProcessor()
{
    apvts_.state.removeListener(this);
}

//==============================================================================
const juce::String SynpleAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SynpleAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SynpleAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SynpleAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SynpleAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SynpleAudioProcessor::getNumPrograms()
{
    return static_cast<int>(presets_.size());
}

int SynpleAudioProcessor::getCurrentProgram()
{
    return currentProgram_;
}

void SynpleAudioProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= getNumPrograms())
    {
        jassert(false && "Invalid program index");
        return;
    }

    currentProgram_ = index;

    juce::RangedAudioParameter* params[NUM_PARAMS] = {
        oscMixParam_,
        oscTuneParam_,
        oscFineParam_,
        glideModeParam_,
        glideRateParam_,
        glideBendParam_,
        filterFreqParam_,
        filterResoParam_,
        filterEnvParam_,
        filterLFOParam_,
        filterVelocityParam_,
        filterAttackParam_,
        filterDecayParam_,
        filterSustainParam_,
        filterReleaseParam_,
        envAttackParam_,
        envDecayParam_,
        envSustainParam_,
        envReleaseParam_,
        lfoRateParam_,
        vibratoParam_,
        noiseParam_,
        octaveParam_,
        tuningParam_,
        outputLevelParam_,
        polyModeParam_,
    };

    const Preset& preset = presets_[static_cast<size_t>(index)];
    for (size_t i = 0; i < NUM_PARAMS; ++i)
    {
        params[i]->setValueNotifyingHost(params[i]->convertTo0to1(preset.parameters()[i]));
    }

    reset();
}

const juce::String SynpleAudioProcessor::getProgramName(int index)
{
    return {presets_[static_cast<size_t>(index)].name()};
}

void SynpleAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void SynpleAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth_.allocateResources(static_cast<float>(sampleRate), samplesPerBlock);
    parametersChanged_.store(true);
    reset();

    envelopeFollower_.prepare(
        juce::dsp::ProcessSpec{.sampleRate = sampleRate,
                               .maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock),
                               .numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels())});
    envelopeFollower_.setAttackTime(200.0f);
    envelopeFollower_.setReleaseTime(200.0f);
    envelopeFollower_.setLevelCalculationType(juce::dsp::BallisticsFilter<float>::LevelCalculationType::peak);
    envelopeFollowerOutputBuffer_.setSize(getTotalNumOutputChannels(), samplesPerBlock);
}

void SynpleAudioProcessor::releaseResources()
{
    synth_.deallocateResources();
}

bool SynpleAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void SynpleAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    synth_.resoCC = midiLearnCC_.load();

    bool expected = true;
    if (isNonRealtime() || parametersChanged_.compare_exchange_strong(expected, false))
    {
        update();
    }

    splitBufferByEvents(buffer, midiMessages);

    const auto inBlock =
        juce::dsp::AudioBlock<float>(buffer).getSubsetChannelBlock(0u, static_cast<size_t>(totalNumOutputChannels));
    auto outputBlock = juce::dsp::AudioBlock<float>(envelopeFollowerOutputBuffer_);

    envelopeFollower_.process(juce::dsp::ProcessContextNonReplacing<float>(inBlock, outputBlock));
    outputLevelLeft.store(
        juce::Decibels::gainToDecibels(outputBlock.getSample(0u, static_cast<int>(outputBlock.getNumSamples() - 1))));
}

void SynpleAudioProcessor::reset()
{
    synth_.reset();
    synth_.setOutputLevelInstantly(juce::Decibels::decibelsToGain(outputLevelParam_->get()));

    midiLearn.store(false);
    midiLearnCC_.store(synth_.resoCC);
}

//==============================================================================
bool SynpleAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SynpleAudioProcessor::createEditor()
{
    return new SynpleAudioProcessorEditor(*this);
}

//==============================================================================
void SynpleAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    copyXmlToBinary(*apvts_.copyState().createXml(), destData);

    auto xml = std::make_unique<juce::XmlElement>(pluginTag);
    std::unique_ptr<juce::XmlElement> parametersXML(apvts_.copyState().createXml());
    xml->addChildElement(parametersXML.release());

    auto extraXML = std::make_unique<juce::XmlElement>(extraTag);
    extraXML->setAttribute(midiCCAttribute, midiLearnCC_.load());
    xml->addChildElement(extraXML.release());

    copyXmlToBinary(*xml, destData);
}

void SynpleAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml{getXmlFromBinary(data, sizeInBytes)};
    if (xml && xml->hasTagName(pluginTag))
    {
        if (auto* parametersXML = xml->getChildByName(apvts_.state.getType()))
        {
            apvts_.replaceState(juce::ValueTree::fromXml(*parametersXML));
            parametersChanged_.store(true);
        }

        if (auto* extraXML = xml->getChildByName(extraTag))
        {
            int midiCC = extraXML->getIntAttribute(midiCCAttribute);
            if (midiCC != 0)
            {
                midiLearnCC_.store(static_cast<uint8_t>(midiCC));
            }
        }
    }
}

juce::AudioProcessorValueTreeState& SynpleAudioProcessor::getApvts()
{
    return apvts_;
}

void SynpleAudioProcessor::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&)
{
    parametersChanged_.store(true);
}

void SynpleAudioProcessor::update()
{
    const float sampleRate{static_cast<float>(getSampleRate())};
    const float inverseSampleRate{1.0f / sampleRate};

    synth_.setOscillatorMix(oscMixParam_->get() / 100.0f);
    synth_.setDetune(oscTuneParam_->get(), oscFineParam_->get());

    const float octave{octaveParam_->get()};
    const float tuning{tuningParam_->get()};
    const float tuneInSemi{-36.3763f - 12.0f * octave - tuning / 100.0f};
    synth_.setTune(sampleRate * std::exp(0.05776226505f * tuneInSemi));

    const float filterReso{filterResoParam_->get() / 100.0f};
    synth_.setFilterQ(filterReso);
    synth_.setFilterKeyTracking(filterFreqParam_->get());
    synth_.setFilterVelocity(filterVelocityParam_->get());
    synth_.setFilterLfoDepth(filterLFOParam_->get() / 100.0f);

    synth_.setEnvelopeAttack(std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * envAttackParam_->get())));
    synth_.setEnvelopeDecay(std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * envDecayParam_->get())));
    synth_.setEnvelopeSustain(envSustainParam_->get() / 100.0f);
    const float releaseParamValue{envReleaseParam_->get()};
    if (releaseParamValue < 1.0f)
    {
        synth_.setEnvelopeRelease(0.75f);
    }
    else
    {
        synth_.setEnvelopeRelease(std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * releaseParamValue)));
    }

    const float noiseMix{noiseParam_->get() / 100.0f};
    synth_.setNoiseMix(noiseMix * noiseMix * 0.06f);

    synth_.setOutputLevel(juce::Decibels::decibelsToGain(outputLevelParam_->get()));
    synth_.setVolumeTrim(filterReso);

    synth_.setPolyphonic(polyModeParam_->getIndex() == 1);
    synth_.setLfoIncrement(lfoRateParam_->get(), inverseSampleRate);
    synth_.setVibratoAmount(vibratoParam_->get());
    synth_.setGlide(glideModeParam_->getIndex(), glideRateParam_->get(), glideBendParam_->get(), inverseSampleRate);

    synth_.setFilterEnvelope(filterAttackParam_->get(),
                             filterDecayParam_->get(),
                             filterSustainParam_->get(),
                             filterReleaseParam_->get(),
                             filterEnvParam_->get(),
                             inverseSampleRate);
}

void SynpleAudioProcessor::createPrograms()
{
    // clang-format off
    presets_.clear();
    presets_.reserve(53);
    presets_.emplace_back("Init", 0.00f, -12.00f, 0.00f, 0.00f, 35.00f, 0.00f, 100.00f, 15.00f, 50.00f, 0.00f, 0.00f, 0.00f, 30.00f, 0.00f, 25.00f, 0.00f, 50.00f, 100.00f, 30.00f, 0.81f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("5th Sweep Pad", 100.00f, -7.00f, -6.30f, 1.00f, 32.00f, 0.00f, 90.00f, 60.00f, -76.00f, 0.00f, 0.00f, 90.00f, 89.00f, 90.00f, 73.00f, 0.00f, 50.00f, 100.00f, 71.00f, 0.81f, 30.00f, 0.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Echo Pad [SA]", 88.00f, 0.00f, 0.00f, 0.00f, 49.00f, 0.00f, 46.00f, 76.00f, 38.00f, 10.00f, 38.00f, 100.00f, 86.00f, 76.00f, 57.00f, 30.00f, 80.00f, 68.00f, 66.00f, 0.79f, -74.00f, 25.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Space Chimes [SA]", 88.00f, 0.00f, 0.00f, 0.00f, 49.00f, 0.00f, 49.00f, 82.00f, 32.00f, 8.00f, 78.00f, 85.00f, 69.00f, 76.00f, 47.00f, 12.00f, 22.00f, 55.00f, 66.00f, 0.89f, -32.00f, 0.00f, 2.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Solid Backing", 100.00f, -12.00f, -18.70f, 0.00f, 35.00f, 0.00f, 30.00f, 25.00f, 40.00f, 0.00f, 26.00f, 0.00f, 35.00f, 0.00f, 25.00f, 0.00f, 50.00f, 100.00f, 30.00f, 0.81f, 0.00f, 50.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Velocity Backing [SA]", 41.00f, 0.00f, 9.70f, 0.00f, 8.00f, -1.68f, 49.00f, 1.00f, -32.00f, 0.00f, 86.00f, 61.00f, 87.00f, 100.00f, 93.00f, 11.00f, 48.00f, 98.00f, 32.00f, 0.81f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Rubber Backing [ZF]", 29.00f, 12.00f, -5.60f, 0.00f, 18.00f, 5.06f, 35.00f, 15.00f, 54.00f, 14.00f, 8.00f, 0.00f, 42.00f, 13.00f, 21.00f, 0.00f, 56.00f, 0.00f, 32.00f, 0.20f, 16.00f, 22.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("808 State Lead", 100.00f, 7.00f, -7.10f, 2.00f, 34.00f, 12.35f, 65.00f, 63.00f, 50.00f, 16.00f, 0.00f, 0.00f, 30.00f, 0.00f, 25.00f, 17.00f, 50.00f, 100.00f, 3.00f, 0.81f, 0.00f, 0.00f, 1.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Mono Glide", 0.00f, -12.00f, 0.00f, 2.00f, 46.00f, 0.00f, 51.00f, 0.00f, 0.00f, 0.00f, -100.00f, 0.00f, 30.00f, 0.00f, 25.00f, 37.00f, 50.00f, 100.00f, 38.00f, 0.81f, 24.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("Detuned Techno Lead", 84.00f, 0.00f, -17.20f, 2.00f, 41.00f, -0.15f, 54.00f, 1.00f, 16.00f, 21.00f, 34.00f, 0.00f, 9.00f, 100.00f, 25.00f, 20.00f, 85.00f, 100.00f, 30.00f, 0.83f, -82.00f, 40.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Hard Lead [SA]", 71.00f, 12.00f, 0.00f, 0.00f, 24.00f, 36.00f, 56.00f, 52.00f, 38.00f, 19.00f, 40.00f, 100.00f, 14.00f, 65.00f, 95.00f, 7.00f, 91.00f, 100.00f, 15.00f, 0.84f, -34.00f, 0.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Bubble", 0.00f, -12.00f, -0.20f, 0.00f, 71.00f, -0.00f, 23.00f, 77.00f, 60.00f, 32.00f, 26.00f, 40.00f, 18.00f, 66.00f, 14.00f, 0.00f, 38.00f, 65.00f, 16.00f, 0.48f, 0.00f, 0.00f, 1.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Monosynth", 62.00f, -12.00f, 0.00f, 1.00f, 35.00f, 0.02f, 64.00f, 39.00f, 2.00f, 65.00f, -100.00f, 7.00f, 52.00f, 24.00f, 84.00f, 13.00f, 30.00f, 76.00f, 21.00f, 0.58f, -40.00f, 0.00f, -1.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("Moogcury Lite", 81.00f, 24.00f, -9.80f, 1.00f, 15.00f, -0.97f, 39.00f, 17.00f, 38.00f, 40.00f, 24.00f, 0.00f, 47.00f, 19.00f, 37.00f, 0.00f, 50.00f, 20.00f, 33.00f, 0.38f, 6.00f, 0.00f, -2.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("Gangsta Whine", 0.00f, 0.00f, 0.00f, 2.00f, 44.00f, 0.00f, 41.00f, 46.00f, 0.00f, 0.00f, -100.00f, 0.00f, 0.00f, 100.00f, 25.00f, 15.00f, 50.00f, 100.00f, 32.00f, 0.81f, -2.00f, 0.00f, 2.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("Higher Synth [ZF]", 48.00f, 0.00f, -8.80f, 0.00f, 0.00f, 0.00f, 50.00f, 47.00f, 46.00f, 30.00f, 60.00f, 0.00f, 10.00f, 0.00f, 7.00f, 0.00f, 42.00f, 0.00f, 22.00f, 0.21f, 18.00f, 16.00f, 2.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("303 Saw Bass", 0.00f, 0.00f, 0.00f, 1.00f, 49.00f, 0.00f, 55.00f, 75.00f, 38.00f, 35.00f, 0.00f, 0.00f, 56.00f, 0.00f, 56.00f, 0.00f, 80.00f, 100.00f, 24.00f, 0.26f, -2.00f, 0.00f, -2.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("303 Square Bass", 75.00f, 0.00f, 0.00f, 1.00f, 49.00f, 0.00f, 55.00f, 75.00f, 38.00f, 35.00f, 0.00f, 14.00f, 49.00f, 0.00f, 39.00f, 0.00f, 80.00f, 100.00f, 24.00f, 0.26f, -2.00f, 0.00f, -2.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("Analog Bass", 100.00f, -12.00f, -10.90f, 1.00f, 19.00f, 0.00f, 30.00f, 51.00f, 70.00f, 9.00f, -100.00f, 0.00f, 88.00f, 0.00f, 21.00f, 0.00f, 50.00f, 100.00f, 46.00f, 0.81f, 0.00f, 0.00f, -1.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("Analog Bass 2", 100.00f, -12.00f, -10.90f, 0.00f, 19.00f, 13.44f, 48.00f, 43.00f, 88.00f, 0.00f, 60.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 61.00f, 100.00f, 32.00f, 0.81f, 0.00f, 0.00f, -1.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("Low Pulses", 97.00f, -12.00f, -3.30f, 0.00f, 35.00f, 0.00f, 80.00f, 40.00f, 4.00f, 0.00f, 0.00f, 0.00f, 77.00f, 0.00f, 25.00f, 0.00f, 50.00f, 100.00f, 30.00f, 0.81f, -68.00f, 0.00f, -2.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Sine Infra-Bass", 0.00f, -12.00f, 0.00f, 0.00f, 35.00f, 0.00f, 33.00f, 76.00f, 6.00f, 0.00f, 0.00f, 0.00f, 30.00f, 0.00f, 25.00f, 0.00f, 55.00f, 25.00f, 30.00f, 0.81f, 4.00f, 0.00f, -2.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("Wobble Bass [SA]", 100.00f, -12.00f, -8.80f, 0.00f, 82.00f, 0.21f, 72.00f, 47.00f, -32.00f, 34.00f, 64.00f, 20.00f, 69.00f, 100.00f, 15.00f, 9.00f, 50.00f, 100.00f, 7.00f, 0.81f, -8.00f, 0.00f, -1.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("Squelch Bass", 100.00f, -12.00f, -8.80f, 0.00f, 35.00f, 0.00f, 67.00f, 70.00f, -48.00f, 0.00f, 0.00f, 48.00f, 69.00f, 100.00f, 15.00f, 0.00f, 50.00f, 100.00f, 7.00f, 0.81f, -8.00f, 0.00f, -1.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("Rubber Bass [ZF]", 49.00f, -12.00f, 1.60f, 1.00f, 35.00f, 0.00f, 36.00f, 15.00f, 50.00f, 20.00f, 0.00f, 0.00f, 38.00f, 0.00f, 25.00f, 0.00f, 60.00f, 100.00f, 22.00f, 0.19f, 0.00f, 0.00f, -2.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("Soft Pick Bass", 37.00f, 0.00f, 7.80f, 0.00f, 22.00f, 0.00f, 33.00f, 47.00f, 42.00f, 16.00f, 18.00f, 0.00f, 0.00f, 0.00f, 25.00f, 4.00f, 58.00f, 0.00f, 22.00f, 0.15f, -12.00f, 33.00f, -2.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("Fretless Bass", 50.00f, 0.00f, -14.40f, 1.00f, 34.00f, 0.00f, 51.00f, 0.00f, 16.00f, 0.00f, 34.00f, 0.00f, 9.00f, 0.00f, 25.00f, 20.00f, 85.00f, 0.00f, 30.00f, 0.81f, 40.00f, 0.00f, -2.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("Whistler", 23.00f, 0.00f, -0.70f, 0.00f, 35.00f, 0.00f, 33.00f, 100.00f, 0.00f, 0.00f, 0.00f, 0.00f, 29.00f, 0.00f, 25.00f, 68.00f, 39.00f, 58.00f, 36.00f, 0.81f, 28.00f, 38.00f, 2.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Very Soft Pad", 39.00f, 0.00f, -4.90f, 2.00f, 12.00f, 0.00f, 35.00f, 78.00f, 0.00f, 0.00f, 0.00f, 0.00f, 30.00f, 0.00f, 25.00f, 35.00f, 50.00f, 80.00f, 70.00f, 0.81f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Pizzicato", 0.00f, -12.00f, 0.00f, 0.00f, 35.00f, 0.00f, 23.00f, 20.00f, 50.00f, 0.00f, 0.00f, 0.00f, 22.00f, 0.00f, 25.00f, 0.00f, 47.00f, 0.00f, 30.00f, 0.81f, 0.00f, 80.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Synth Strings", 100.00f, 0.00f, -7.10f, 0.00f, 0.00f, -0.97f, 42.00f, 26.00f, 50.00f, 14.00f, 38.00f, 0.00f, 67.00f, 55.00f, 97.00f, 82.00f, 70.00f, 100.00f, 42.00f, 0.84f, 34.00f, 30.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Synth Strings 2", 75.00f, 0.00f, -3.80f, 0.00f, 49.00f, 0.00f, 55.00f, 16.00f, 38.00f, 8.00f, -60.00f, 76.00f, 29.00f, 76.00f, 100.00f, 46.00f, 80.00f, 100.00f, 39.00f, 0.79f, -46.00f, 0.00f, 1.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Leslie Organ", 0.00f, 0.00f, 0.00f, 0.00f, 13.00f, -0.38f, 38.00f, 74.00f, 8.00f, 20.00f, -100.00f, 0.00f, 55.00f, 52.00f, 31.00f, 0.00f, 17.00f, 73.00f, 28.00f, 0.87f, -52.00f, 0.00f, -1.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Click Organ", 50.00f, 12.00f, 0.00f, 0.00f, 35.00f, 0.00f, 44.00f, 50.00f, 30.00f, 16.00f, -100.00f, 0.00f, 0.00f, 18.00f, 0.00f, 0.00f, 75.00f, 80.00f, 0.00f, 0.81f, -2.00f, 0.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Hard Organ", 89.00f, 19.00f, -0.90f, 0.00f, 35.00f, 0.00f, 51.00f, 62.00f, 8.00f, 0.00f, -100.00f, 0.00f, 37.00f, 0.00f, 100.00f, 4.00f, 8.00f, 72.00f, 4.00f, 0.77f, -2.00f, 0.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Bass Clarinet", 100.00f, 0.00f, 0.00f, 1.00f, 0.00f, 0.00f, 51.00f, 10.00f, 0.00f, 11.00f, 0.00f, 0.00f, 0.00f, 0.00f, 25.00f, 35.00f, 65.00f, 65.00f, 32.00f, 0.79f, -2.00f, 20.00f, -1.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Trumpet", 0.00f, 0.00f, 0.00f, 1.00f, 6.00f, 0.00f, 57.00f, 0.00f, -36.00f, 15.00f, 0.00f, 21.00f, 15.00f, 0.00f, 25.00f, 24.00f, 60.00f, 80.00f, 10.00f, 0.75f, 10.00f, 25.00f, 1.00f, 0.00f, 0.00f, 0.00f);
    presets_.emplace_back("Soft Horn", 12.00f, 19.00f, 1.90f, 0.00f, 35.00f, 0.00f, 50.00f, 21.00f, -42.00f, 12.00f, 20.00f, 0.00f, 35.00f, 36.00f, 25.00f, 8.00f, 50.00f, 100.00f, 27.00f, 0.83f, 2.00f, 10.00f, -1.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Brass Section", 43.00f, 12.00f, -7.90f, 0.00f, 28.00f, -0.79f, 50.00f, 0.00f, 18.00f, 0.00f, 0.00f, 24.00f, 16.00f, 91.00f, 8.00f, 17.00f, 50.00f, 80.00f, 45.00f, 0.81f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Synth Brass", 40.00f, 0.00f, -6.30f, 0.00f, 30.00f, -3.07f, 39.00f, 15.00f, 50.00f, 0.00f, 0.00f, 39.00f, 30.00f, 82.00f, 25.00f, 33.00f, 74.00f, 76.00f, 41.00f, 0.81f, -6.00f, 23.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Detuned Syn Brass [ZF]", 68.00f, 0.00f, 31.80f, 0.00f, 31.00f, 0.50f, 26.00f, 7.00f, 70.00f, 0.00f, 32.00f, 0.00f, 83.00f, 0.00f, 5.00f, 0.00f, 75.00f, 54.00f, 32.00f, 0.76f, -26.00f, 29.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Power PWM", 100.00f, -12.00f, -8.80f, 0.00f, 35.00f, 0.00f, 82.00f, 13.00f, 50.00f, 0.00f, -100.00f, 24.00f, 30.00f, 88.00f, 34.00f, 0.00f, 50.00f, 100.00f, 48.00f, 0.71f, -26.00f, 0.00f, -1.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Water Velocity [SA]", 76.00f, 0.00f, -1.40f, 0.00f, 49.00f, 0.00f, 87.00f, 67.00f, 100.00f, 32.00f, -82.00f, 95.00f, 56.00f, 72.00f, 100.00f, 4.00f, 76.00f, 11.00f, 46.00f, 0.88f, 44.00f, 0.00f, -1.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Ghost [SA]", 75.00f, 0.00f, -7.10f, 2.00f, 16.00f, -0.00f, 38.00f, 58.00f, 50.00f, 16.00f, 62.00f, 0.00f, 30.00f, 40.00f, 31.00f, 37.00f, 50.00f, 100.00f, 54.00f, 0.85f, 66.00f, 43.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Soft E.Piano", 31.00f, 0.00f, -0.20f, 0.00f, 35.00f, 0.00f, 34.00f, 26.00f, 6.00f, 0.00f, 26.00f, 0.00f, 22.00f, 0.00f, 39.00f, 0.00f, 80.00f, 0.00f, 44.00f, 0.81f, 2.00f, 0.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Thumb Piano", 72.00f, 15.00f, 50.00f, 0.00f, 35.00f, 0.00f, 37.00f, 47.00f, 8.00f, 0.00f, 0.00f, 0.00f, 45.00f, 0.00f, 39.00f, 0.00f, 39.00f, 0.00f, 48.00f, 0.81f, 20.00f, 0.00f, 1.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Steel Drums [ZF]", 81.00f, 12.00f, -12.00f, 0.00f, 18.00f, 2.30f, 40.00f, 30.00f, 8.00f, 17.00f, -20.00f, 0.00f, 42.00f, 23.00f, 47.00f, 12.00f, 48.00f, 0.00f, 49.00f, 0.53f, -28.00f, 34.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Car Horn", 57.00f, -1.00f, -2.80f, 0.00f, 35.00f, 0.00f, 46.00f, 0.00f, 36.00f, 0.00f, 0.00f, 46.00f, 30.00f, 100.00f, 23.00f, 30.00f, 50.00f, 100.00f, 31.00f, 1.00f, -24.00f, 0.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Helicopter", 0.00f, -12.00f, 0.00f, 0.00f, 35.00f, 0.00f, 8.00f, 36.00f, 38.00f, 100.00f, 0.00f, 100.00f, 100.00f, 0.00f, 100.00f, 96.00f, 50.00f, 100.00f, 92.00f, 0.97f, 0.00f, 100.00f, -2.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Arctic Wind", 0.00f, -12.00f, 0.00f, 0.00f, 35.00f, 0.00f, 16.00f, 85.00f, 0.00f, 28.00f, 0.00f, 37.00f, 30.00f, 0.00f, 25.00f, 89.00f, 50.00f, 100.00f, 89.00f, 0.24f, 0.00f, 100.00f, 2.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Thip", 100.00f, -7.00f, 0.00f, 0.00f, 35.00f, 0.00f, 0.00f, 100.00f, 94.00f, 0.00f, 0.00f, 2.00f, 20.00f, 0.00f, 20.00f, 0.00f, 46.00f, 0.00f, 30.00f, 0.81f, 0.00f, 78.00f, 0.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Synth Tom", 0.00f, -12.00f, 0.00f, 0.00f, 76.00f, 24.53f, 30.00f, 33.00f, 52.00f, 0.00f, 36.00f, 0.00f, 59.00f, 0.00f, 59.00f, 10.00f, 50.00f, 0.00f, 50.00f, 0.81f, 0.00f, 70.00f, -2.00f, 0.00f, 0.00f, 1.00f);
    presets_.emplace_back("Squelchy Frog", 50.00f, -5.00f, -7.90f, 2.00f, 77.00f, -36.00f, 40.00f, 65.00f, 90.00f, 0.00f, 0.00f, 33.00f, 50.00f, 0.00f, 25.00f, 0.00f, 70.00f, 65.00f, 18.00f, 0.32f, 100.00f, 0.00f, -2.00f, 0.00f, 0.00f, 1.00f);
    // clang-format on
}

void SynpleAudioProcessor::splitBufferByEvents(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    int bufferOffset{0};

    for (const auto& midiMetaData : midiMessages)
    {
        // Render the audio before this MIDI event, if any.
        const int samplesThisSegment{midiMetaData.samplePosition - bufferOffset};
        if (samplesThisSegment > 0)
        {
            render(buffer, samplesThisSegment, bufferOffset);
            bufferOffset += samplesThisSegment;
        }

        // Handle the MIDI message. Ignore messages such as sysex.
        if (midiMetaData.numBytes <= 3)
        {
            const uint8_t data0{midiMetaData.data[0]};
            const uint8_t data1{midiMetaData.numBytes >= 2 ? midiMetaData.data[1] : 0_midi};
            const uint8_t data2{midiMetaData.numBytes == 3 ? midiMetaData.data[2] : 0_midi};
            handleMidi(data0, data1, data2);
        }
    }

    // Render the audio after the last MIDI event, or the full buffer if there were none.
    const int samplesLastSegment{buffer.getNumSamples() - bufferOffset};
    if (samplesLastSegment > 0)
    {
        render(buffer, samplesLastSegment, bufferOffset);
    }

    midiMessages.clear();
}

void SynpleAudioProcessor::handleMidi(const uint8_t data0, const uint8_t data1, const uint8_t data2)
{
    // Control Change
    if ((data0 & 0xF0) == 0xB0)
    {
        if (midiLearn)
        {
            DBG("learned a MIDI CC");
            midiLearnCC_.store(data1);
            midiLearn.store(false);
            return;
        }

        if (data1 == 0x07) // volume
        {
            const float volumeCtl = static_cast<float>(data2) / 127.0f;
            outputLevelParam_->beginChangeGesture();
            outputLevelParam_->setValueNotifyingHost(volumeCtl);
            outputLevelParam_->endChangeGesture();
        }
    }

    // Program Change
    if ((data0 & 0xF0) == 0xC0)
    {
        if (data1 < presets_.size())
        {
            setCurrentProgram(data1);
        }
    }

    synth_.midiMessage(data0, data1, data2);
}

void SynpleAudioProcessor::render(juce::AudioBuffer<float>& buffer, const int sampleCount, const int bufferOffset)
{
    std::vector<std::span<float>> channels;
    channels.reserve(static_cast<size_t>(buffer.getNumChannels()));
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto ptr = buffer.getWritePointer(ch, bufferOffset);
        channels.emplace_back(ptr, sampleCount);
    }
    synth::AudioBuffer audioBuffer{channels};
    synth_.render(audioBuffer);
}

juce::AudioProcessorValueTreeState::ParameterLayout SynpleAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto oscMixStringFromValue = [](const float value, int) {
        char s[16] = {0};
        snprintf(s, 16, "%4.0f:%2.0f", 100.0 - 0.5f * value, 0.5f * value);
        return juce::String(s);
    };

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameter_id::oscMix,
        "Osc Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction(oscMixStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::oscTune,
                                                           "Osc Tune",
                                                           juce::NormalisableRange<float>(-24.0f, 24.0f, 1.0f),
                                                           -12.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("semi")));

    layout.add(
        std::make_unique<juce::AudioParameterFloat>(parameter_id::oscFine,
                                                    "Osc Fine",
                                                    juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f, 0.3f, true),
                                                    0.0f,
                                                    juce::AudioParameterFloatAttributes().withLabel("cent")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        parameter_id::glideMode, "Glide Mode", juce::StringArray{"Off", "Legato", "Always"}, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::glideRate,
                                                           "Glide Rate",
                                                           juce::NormalisableRange<float>(0.0f, 100.f, 1.0f),
                                                           35.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(
        std::make_unique<juce::AudioParameterFloat>(parameter_id::glideBend,
                                                    "Glide Bend",
                                                    juce::NormalisableRange<float>(-36.0f, 36.0f, 0.01f, 0.4f, true),
                                                    0.0f,
                                                    juce::AudioParameterFloatAttributes().withLabel("semi")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::filterFreq,
                                                           "Filter Freq",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
                                                           100.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::filterReso,
                                                           "Filter Reso",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           15.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::filterEnv,
                                                           "Filter Env",
                                                           juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
                                                           50.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::filterLFO,
                                                           "Filter LFO",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           0.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    auto filterVelocityStringFromValue = [](float value, int) {
        return value < -90.0f ? juce::String("OFF") : juce::String(value);
    };

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameter_id::filterVelocity,
        "Velocity",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction(
            filterVelocityStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::filterAttack,
                                                           "Filter Attack",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           0.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::filterDecay,
                                                           "Filter Decay",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           30.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::filterSustain,
                                                           "Filter Sustain",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           0.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::filterRelease,
                                                           "Filter Release",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           25.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::envAttack,
                                                           "Env Attack",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           0.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::envDecay,
                                                           "Env Decay",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           50.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::envSustain,
                                                           "Env Sustain",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           100.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::envRelease,
                                                           "Env Release",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           30.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    auto lfoRateStringFromValue = [](float value, int) {
        const float lfoHz = std::exp(7.0f * value - 4.0f);
        return juce::String(lfoHz, 3);
    };

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameter_id::lfoRate,
        "LFO Rate",
        juce::NormalisableRange<float>(),
        0.81f,
        juce::AudioParameterFloatAttributes().withLabel("Hz").withStringFromValueFunction(lfoRateStringFromValue)));

    auto vibratoStringFromValue = [](float value, int) {
        return value < 0.0f ? "PWM " + juce::String(-value, 1) : juce::String(value, 1);
    };

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameter_id::vibrato,
        "Vibrato",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction(vibratoStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::noise,
                                                           "Noise",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           0.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        parameter_id::octave, "Octave", juce::NormalisableRange<float>(-2.0f, 2.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::tuning,
                                                           "Tuning",
                                                           juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
                                                           0.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("cent")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(parameter_id::outputLevel,
                                                           "Output Level",
                                                           juce::NormalisableRange<float>(-24.0f, 6.0f, 0.1f),
                                                           0.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        parameter_id::polyMode, "Polyphony", juce::StringArray{"Mono", "Poly"}, 1));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynpleAudioProcessor();
}
