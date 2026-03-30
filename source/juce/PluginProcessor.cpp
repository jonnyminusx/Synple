#include "PluginProcessor.h"
#include "PluginEditor.h"
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
                         ),
      parameters_(*this)
{
    parameters_.getApvts().state.addListener(this);
    setCurrentProgram(0);
}

SynpleAudioProcessor::~SynpleAudioProcessor()
{
    parameters_.getApvts().state.removeListener(this);
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

    juce::RangedAudioParameter* params[NUM_PARAMS]{};
    parameters_.fillParameterArray(params);

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
    synth_.setOutputLevelInstantly(juce::Decibels::decibelsToGain(parameters_.outputLevel()));

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
    auto xml = std::make_unique<juce::XmlElement>(pluginTag);
    std::unique_ptr<juce::XmlElement> parametersXML(parameters_.getApvts().copyState().createXml());
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
        if (auto* parametersXML = xml->getChildByName(parameters_.getApvts().state.getType()))
        {
            parameters_.getApvts().replaceState(juce::ValueTree::fromXml(*parametersXML));
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
    return parameters_.getApvts();
}

void SynpleAudioProcessor::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&)
{
    parametersChanged_.store(true);
}

void SynpleAudioProcessor::update()
{
    const float sampleRate{static_cast<float>(getSampleRate())};
    const float inverseSampleRate{1.0f / sampleRate};

    synth_.setOscillatorMix(parameters_.oscMix() / 100.0f);
    synth_.setDetune(parameters_.oscTune(), parameters_.oscFine());

    const float octave{parameters_.octave()};
    const float tuning{parameters_.tuning()};
    const float tuneInSemi{-36.3763f - 12.0f * octave - tuning / 100.0f};
    synth_.setTune(sampleRate * std::exp(0.05776226505f * tuneInSemi));

    const float filterReso{parameters_.filterReso() / 100.0f};
    synth_.setFilterQ(filterReso);
    synth_.setFilterKeyTracking(parameters_.filterFreq());
    synth_.setFilterVelocity(parameters_.filterVelocity());
    synth_.setFilterLfoDepth(parameters_.filterLFO() / 100.0f);

    synth_.setEnvelopeAttack(std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * parameters_.envAttack())));
    synth_.setEnvelopeDecay(std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * parameters_.envDecay())));
    synth_.setEnvelopeSustain(parameters_.envSustain() / 100.0f);
    const float releaseParamValue{parameters_.envRelease()};
    if (releaseParamValue < 1.0f)
    {
        synth_.setEnvelopeRelease(0.75f);
    }
    else
    {
        synth_.setEnvelopeRelease(std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * releaseParamValue)));
    }

    const float noiseMix{parameters_.noise() / 100.0f};
    synth_.setNoiseMix(noiseMix * noiseMix * 0.06f);

    synth_.setOutputLevel(juce::Decibels::decibelsToGain(parameters_.outputLevel()));
    synth_.setVolumeTrim(filterReso);

    synth_.setPolyphonic(parameters_.polyMode() == 1);
    synth_.setLfoIncrement(parameters_.lfoRate(), inverseSampleRate);
    synth_.setVibratoAmount(parameters_.vibrato());
    synth_.setGlide(parameters_.glideMode(), parameters_.glideRate(), parameters_.glideBend(), inverseSampleRate);

    synth_.setFilterEnvelope(parameters_.filterAttack(),
                             parameters_.filterDecay(),
                             parameters_.filterSustain(),
                             parameters_.filterRelease(),
                             parameters_.filterEnv(),
                             inverseSampleRate);
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
            parameters_.setOutputLevelFromMidi(volumeCtl);
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

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynpleAudioProcessor();
}
