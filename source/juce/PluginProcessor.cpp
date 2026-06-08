#include "PluginProcessor.h"
#include "ParameterIds.h"
#include "PluginEditor.h"
#include "dsp/AudioBuffer.h"
#include <span>
#include <vector>

namespace
{

const juce::Identifier pluginTag{"PLUGIN"};
const juce::Identifier extraTag{"EXTRA"};

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
    parameters_.addStateListener(this);
    initialiseLearnableParams();
    setCurrentProgram(0);
}

SynpleAudioProcessor::~SynpleAudioProcessor()
{
    parameters_.removeStateListener(this);
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

    juce::RangedAudioParameter* params[synth::kNumParams]{};
    parameters_.fillParameterArray(params);

    const synth::Preset& preset{presets_[static_cast<size_t>(index)]};
    for (size_t i = 0; i < synth::kNumParams; ++i)
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
    renderChannels_.reserve(static_cast<size_t>(getTotalNumOutputChannels()));
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
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    {
        return false;
    }

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    {
        return false;
    }
#endif

    return true;
#endif
}

void SynpleAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;

    int totalNumInputChannels{getTotalNumInputChannels()};
    int totalNumOutputChannels{getTotalNumOutputChannels()};

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    bool expected{true};
    if (isNonRealtime() || parametersChanged_.compare_exchange_strong(expected, false))
    {
        update();
    }

    splitBufferByEvents(buffer, midiMessages);
}

void SynpleAudioProcessor::reset()
{
    synth_.reset();
    synth_.setOutputLevelInstantly(parameters_.outputGain());
}

//==============================================================================
bool SynpleAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SynpleAudioProcessor::createEditor()
{
    return new SynpleAudioProcessorEditor(*this);
}

//==============================================================================
void SynpleAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto xml{std::make_unique<juce::XmlElement>(pluginTag)};
    xml->addChildElement(parameters_.copyStateToXml().release());

    auto extraXML{std::make_unique<juce::XmlElement>(extraTag)};
    midiLearnMap_.forEachAssignment([&](const char* id, uint8_t cc) {
        extraXML->setAttribute(juce::String("cc_") + id, static_cast<int>(cc));
    });
    xml->addChildElement(extraXML.release());

    copyXmlToBinary(*xml, destData);
}

void SynpleAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml{getXmlFromBinary(data, sizeInBytes)};
    if (xml && xml->hasTagName(pluginTag))
    {
        if (parameters_.restoreStateFromXml(*xml))
            parametersChanged_.store(true);

        if (auto* extraXML = xml->getChildByName(extraTag))
        {
            for (size_t i = 0; i < midi::MidiLearnMap::kNumParams; ++i)
            {
                const juce::String attr{juce::String("cc_") + midi::MidiLearnMap::kParamIds[i]};
                if (extraXML->hasAttribute(attr))
                {
                    const int cc{extraXML->getIntAttribute(attr, -1)};
                    if (cc >= 0 && cc <= 127)
                        midiLearnMap_.setCC(i, static_cast<uint8_t>(cc));
                }
            }
        }
    }
}

juce::RangedAudioParameter& SynpleAudioProcessor::getParameter(const juce::ParameterID& id)
{
    return parameters_.getParameter(id);
}

void SynpleAudioProcessor::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&)
{
    parametersChanged_.store(true);
}

void SynpleAudioProcessor::update()
{
    synth_.setParameters(parameters_.createSnapshot(static_cast<float>(getSampleRate())));
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
        const uint8_t controller{static_cast<uint8_t>(data1 & 0x7F)};
        const uint8_t value{static_cast<uint8_t>(data2 & 0x7F)};
        const float normalised{static_cast<float>(value) / 127.0f};

        // MIDI learn capture: assign the incoming CC to the waiting parameter
        const int capturedIdx{midiLearnMap_.tryCaptureLearning(controller)};
        if (capturedIdx >= 0)
            learnableParams_[static_cast<size_t>(capturedIdx)]->setValueNotifyingHost(normalised);

        // Apply any CC-to-parameter assignments (may include the one just learned)
        for (size_t i = 0; i < midi::MidiLearnMap::kNumParams; ++i)
        {
            if (midiLearnMap_.ccAtIndex(i) == controller)
                learnableParams_[i]->setValueNotifyingHost(normalised);
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

    synth_.midiProcessor().process(data0, data1, data2);
}

void SynpleAudioProcessor::render(juce::AudioBuffer<float>& buffer, const int sampleCount, const int bufferOffset)
{
    renderChannels_.resize(static_cast<size_t>(buffer.getNumChannels()));
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        renderChannels_[static_cast<size_t>(ch)] = {buffer.getWritePointer(ch, bufferOffset),
                                                    static_cast<size_t>(sampleCount)};
    dsp::AudioBuffer audioBuffer{renderChannels_};
    synth_.render(audioBuffer);
}

//==============================================================================
void SynpleAudioProcessor::initialiseLearnableParams()
{
    for (size_t i = 0; i < midi::MidiLearnMap::kNumParams; ++i)
        learnableParams_[i] =
            &parameters_.getParameter(juce::ParameterID{midi::MidiLearnMap::kParamIds[i], parameter_id::kVersion});
}

void SynpleAudioProcessor::beginMidiLearn(const juce::String& paramId)
{
    midiLearnMap_.beginLearn(paramId.toRawUTF8());
}

void SynpleAudioProcessor::cancelMidiLearn()
{
    midiLearnMap_.cancelLearn();
}

void SynpleAudioProcessor::clearMidiLearn(const juce::String& paramId)
{
    midiLearnMap_.clearLearn(paramId.toRawUTF8());
}

uint8_t SynpleAudioProcessor::getMidiLearnCC(const juce::String& paramId) const
{
    return midiLearnMap_.ccForParam(paramId.toRawUTF8());
}

juce::String SynpleAudioProcessor::getMidiLearnParamId() const
{
    const char* id{midiLearnMap_.learningParamId()};
    return id != nullptr ? juce::String{id} : juce::String{};
}

juce::var SynpleAudioProcessor::getMidiLearnState() const
{
    juce::DynamicObject::Ptr assignmentsObj{new juce::DynamicObject{}};
    midiLearnMap_.forEachAssignment([&](const char* id, uint8_t cc) {
        assignmentsObj->setProperty(id, static_cast<int>(cc));
    });
    juce::DynamicObject::Ptr obj{new juce::DynamicObject{}};
    obj->setProperty("assignments", juce::var{assignmentsObj.get()});
    obj->setProperty("learningParam", getMidiLearnParamId());
    return juce::var{obj.get()};
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynpleAudioProcessor();
}
