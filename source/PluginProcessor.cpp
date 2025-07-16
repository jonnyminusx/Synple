#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "synth/AudioBuffer.h"
#include "synth/NoiseGenerator.h"

namespace
{

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
JLX11AudioProcessor::JLX11AudioProcessor()
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
}

JLX11AudioProcessor::~JLX11AudioProcessor()
{
    apvts_.state.removeListener(this);
}

//==============================================================================
const juce::String JLX11AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JLX11AudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool JLX11AudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool JLX11AudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double JLX11AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JLX11AudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int JLX11AudioProcessor::getCurrentProgram()
{
    return 0;
}

void JLX11AudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String JLX11AudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void JLX11AudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void JLX11AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth_.allocateResources(static_cast<float>(sampleRate), samplesPerBlock);
    parametersChanged_.store(true);
    reset();
}

void JLX11AudioProcessor::releaseResources()
{
    synth_.deallocateResources();
}

bool JLX11AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

void JLX11AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    bool expected = true;
    if (isNonRealtime() || parametersChanged_.compare_exchange_strong(expected, false))
    {
        update();
    }

    splitBufferByEvents(buffer, midiMessages);
}

void JLX11AudioProcessor::reset()
{
    synth_.reset();
}

//==============================================================================
bool JLX11AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* JLX11AudioProcessor::createEditor()
{
    auto editor = new juce::GenericAudioProcessorEditor(*this);
    editor->setSize(500, 1050);
    return editor;
}

//==============================================================================
void JLX11AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused(destData);
}

void JLX11AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused(data, sizeInBytes);
}

void JLX11AudioProcessor::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&)
{
    parametersChanged_.store(true);
}

void JLX11AudioProcessor::update()
{
    const float noiseMix = noiseParam_->get() / 100.0f;
    synth_.setNoiseMix(noiseMix * noiseMix * 0.06f);
}

void JLX11AudioProcessor::splitBufferByEvents(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

void JLX11AudioProcessor::handleMidi(const uint8_t data0, const uint8_t data1, const uint8_t data2)
{
    synth_.midiMessage(data0, data1, data2);
}

void JLX11AudioProcessor::render(juce::AudioBuffer<float>& buffer,
                                 const int sampleCount,
                                 [[maybe_unused]] const int bufferOffset)
{
    synth::AudioBuffer audioBuffer{buffer, bufferOffset, bufferOffset + sampleCount};
    synth_.render(audioBuffer);
}

juce::AudioProcessorValueTreeState::ParameterLayout JLX11AudioProcessor::createParameterLayout()
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
    return new JLX11AudioProcessor();
}
