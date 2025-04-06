#include "PluginProcessor.h"
#include "AudioBuffer.h"
#include "NoiseGenerator.h"
#include "PluginEditor.h"

namespace
{

constexpr uint8_t operator""_midi(unsigned long long value) noexcept
{
    return static_cast<uint8_t>(value);
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
                         ),
      synth_(new NoiseGenerator())
{
}

JLX11AudioProcessor::~JLX11AudioProcessor()
{
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
    synth_.allocateResources(sampleRate, samplesPerBlock);
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

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    // for (int channel = 0; channel < totalNumInputChannels; ++channel)
    // {
    //     auto* channelData = buffer.getWritePointer (channel);
    //     juce::ignoreUnused (channelData);
    //     // ..do something to the data...
    // }

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
    return new AudioPluginAudioProcessorEditor(*this);
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
                                 [[maybe_unused]] const int bufferOffset) const
{
    AudioBuffer audioBuffer{buffer, bufferOffset, bufferOffset + sampleCount};
    synth_.render(audioBuffer);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JLX11AudioProcessor();
}
