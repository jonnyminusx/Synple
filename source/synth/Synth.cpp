#include "Synth.h"
#include "Output.h"
#include "utils/constants.h"
#include "utils/protectYourEars.h"

namespace synth
{

void Synth::allocateResources(const float sampleRate, [[maybe_unused]] const int samplesPerBlock)
{
    sampleRate_ = sampleRate;

    for (Voice& voice : voices_)
    {
        voice.filter().setSampleRate(sampleRate_);
    }
}

void Synth::deallocateResources() const
{
}

void Synth::reset()
{
    for (Voice& voice : voices_)
    {
        voice.reset();
    }

    noiseGenerator_.reset();
    pitchBend_ = 1.0f;
    modWheel_ = 0.0f;
    sustainPedalPressed_ = false;
    outputLevelSmoother_.reset(sampleRate_, 0.05);
    lfo_ = 0.0f;
    lfoStep_ = 0;
    resonanceCtl_ = 1.0f;
    pressure_ = 0.0f;
    filterControl_ = 0.0f;
    filterZip_ = 0.0f;
}

void Synth::render(AudioBuffer& audioBuffer)
{
    const auto audioBufferLeft{audioBuffer.channelBuffer(0)};
    const auto audioBufferRight{audioBuffer.channelCount() > 1 ? audioBuffer.channelBuffer(1) : std::span<float>{}};

    for (size_t sampleIndex = 0; sampleIndex < audioBuffer.sampleCount(); ++sampleIndex)
    {
        updateLfo();

        const float noise{noiseGenerator_.nextValue() * noiseMix_};

        Output output;

        for (Voice& voice : voices_)
        {
            output += voice.render(noise, pitchBend_, detune_);
        }

        output *= outputLevelSmoother_.getNextValue();

        if (audioBuffer.channelCount() > 1)
        {
            audioBufferLeft[sampleIndex] = output.left;
            audioBufferRight[sampleIndex] = output.right;
        }
        else
        {
            audioBufferLeft[sampleIndex] = (output.left + output.right) * 0.5f;
        }
    }

    utils::protectYourEars(audioBuffer);
}

void Synth::midiMessage(const uint8_t data0, const uint8_t data1, const uint8_t data2)
{
    switch (data0 & 0xF0)
    {
    case 0x80: // Note Off
    {
        const uint8_t note = data1 & 0x7F;
        noteOff(note);
        break;
    }
    case 0x90: // Note On
    {
        const uint8_t note = data1 & 0x7F;
        const uint8_t velocity = data2 & 0x7F;
        if (velocity == 0)
        {
            noteOff(note);
        }
        else
        {
            noteOn(note, velocity);
        }
        break;
    }
    case 0xB0: // Control Change
        controlChange(data1 & 0x7F, data2 & 0x7F);
        break;
    case 0xC0: // Program Change
        break;
    case 0xD0: // Channel Aftertouch
        pressure_ = 0.0001f * float(data1 * data1);
        break;
    case 0xE0: // Pitch Bend
        pitchBend_ = std::exp(-0.000014102f * data1 + (128 * data2) - 8192);
        break;
    case 0x01: // Modulation Wheel
        modWheel_ = 0.000005f * float(data2 * data2);
        break;
    default:
        break;
    }
}

void Synth::controlChange(const uint8_t controller, const uint8_t value)
{
    switch (controller)
    {
    case 0x40: // Sustain pedal
        sustainPedalPressed_ = value >= 64;

        if (!sustainPedalPressed_)
        {
            noteOff(Voice::sustain);
        }

        break;

    case 0x4A: // Filter +
        filterControl_ = 0.02f * float(value);
        break;

    case 0x4B: // Filter -
        filterControl_ = -0.03f * float(value);
        break;

    default: // All notes off
        if (controller >= 0x78)
        {
            for (Voice& voice : voices_)
            {
                voice.reset();
            }
            sustainPedalPressed_ = false;
        }

        break;
    }

    // Resonance
    if (controller == resoCC)
    {
        resonanceCtl_ = 154.0f / float(154 - value);
    }
}

void Synth::setVolumeTrim(const float filterReso)
{
    volumeTrim_ = 0.0008f * (3.2f - oscillatorMix_ - 25.0f * noiseMix_) * (1.5f - 0.5f * filterReso);
}

void Synth::updateLfo()
{
    if (--lfoStep_ < 0)
    {
        lfoStep_ = lfoMaxSamplesPerUpdate_;

        lfo_ += lfoIncrement_;
        if (lfo_ > constants::pi)
        {
            lfo_ -= constants::tau;
        }

        const float sineValue{std::sinf(lfo_)};
        const float vibratoModulation{1.0f + sineValue * (modWheel_ + vibratoAmount_)};
        const float pwm{1.0f + sineValue * (modWheel_ + pwmDepth_)};
        const float filterMod{filterKeyTracking_ + filterControl_ + (filterLfoDepth_ + pressure_) * sineValue};
        filterZip_ += 0.005f * (filterMod - filterZip_);

        for (Voice& voice : voices_)
        {
            voice.setModulation(vibratoModulation, pwm);
            voice.updateLfo(glideRate_, filterZip_, filterQ_ * resonanceCtl_, pitchBend_, filterEnvDepth_);
            voice.updatePeriod(pitchBend_, detune_);
        }
    }
}

void Synth::shiftQueuedNotes()
{
    for (size_t voiceIdx = maxNumVoices_ - 1; voiceIdx > 0; --voiceIdx)
    {
        voices_[voiceIdx].note() = voices_[voiceIdx - 1].note();
        voices_[voiceIdx].release();
    }
}

int Synth::nextQueuedNote()
{
    int note{0};
    for (size_t voiceIdx = 1; voiceIdx < maxNumVoices_; ++voiceIdx)
    {
        const int voiceNote{voices_[voiceIdx].note()};
        if (voiceNote > 0)
        {
            voices_[voiceIdx].note() = 0;
            note = voiceNote;
            break;
        }
    }

    return note;
}

void Synth::noteOn(const int note, int velocity)
{
    if (ignoreVelocity_)
    {
        velocity = 80;
    }

    const size_t voiceIdx{selectVoiceIndexToUse()};
    if (isPolyphonic())
    {
        startVoice(voiceIdx, note, velocity);
    }
    else
    {
        if (voices_[voiceIdx].note() > 0)
        {
            shiftQueuedNotes();
            restartMonoVoice(note, velocity);
        }
        else
        {
            startVoice(voiceIdx, note, velocity);
        }
    }
}

void Synth::startVoice(const size_t voiceIdx, const int note, const int velocity)
{
    if (voiceIdx >= voices_.size())
    {
        return;
    }

    Voice& voice = voices_[voiceIdx];

    Envelope& envelope = voice.envelope();
    envelope.setAttackMultiplier(envelopeAttack_);
    envelope.setDecayMultiplier(envelopeDecay_);
    envelope.setSustainLevel(envelopeSustain_);
    envelope.setReleaseMultiplier(envelopeRelease_);
    envelope.attack();

    Envelope& filterEnvelope = voice.filterEnvelope();
    filterEnvelope.setAttackMultiplier(filterAttack_);
    filterEnvelope.setDecayMultiplier(filterDecay_);
    filterEnvelope.setSustainLevel(filterSustain_);
    filterEnvelope.setReleaseMultiplier(filterRelease_);
    filterEnvelope.attack();

    voice.noteOn(note,
                 lastNote_,
                 velocity,
                 velocitySensitivity_,
                 volumeTrim_,
                 oscillatorMix_,
                 tune_,
                 detune_,
                 glideBend_,
                 sampleRate_,
                 voiceIdx,
                 isInPwmMode(),
                 isPlayingLegatoStyle(),
                 glideMode_);
    lastNote_ = note;
}

void Synth::restartMonoVoice(const int note, [[maybe_unused]] const int velocity)
{
    voices_[0].noteOnRestart(note, velocity, velocitySensitivity_, tune_, detune_, sampleRate_, 0, glideMode_);
}

size_t Synth::selectVoiceIndexToUse() const
{
    if (!isPolyphonic())
    {
        return 0;
    }

    size_t idx{0};
    float minLevel{std::numeric_limits<float>::max()};

    for (size_t i = 1, n = voices_.size(); i < n; ++i)
    {
        const Voice& voice{voices_[i]};

        if (!voice.envelope().isInAttack())
        {
            const float currentLevel{voice.envelope().currentValue()};
            if (currentLevel < minLevel)
            {
                minLevel = currentLevel;
                idx = i;
            }
        }
    }

    return idx;
}

bool Synth::isPolyphonic() const
{
    return numVoices_ > 1;
}

bool Synth::isInPwmMode() const
{
    return vibratoAmount_ == 0.0f && pwmDepth_ > 0.0f;
}

bool Synth::isPlayingLegatoStyle() const
{
    return std::any_of(voices_.begin(), voices_.end(), [](const Voice& voice) { return voice.note() > 0; });
}

void Synth::processLastNotePriority(const int note)
{
    if (isPolyphonic())
    {
        return;
    }

    if (voices_[0].note() != note)
    {
        return;
    }

    int queuedNote{nextQueuedNote()};

    if (queuedNote > 0)
    {
        restartMonoVoice(queuedNote, -1);
    }
}

void Synth::noteOff(const int note)
{
    processLastNotePriority(note);

    for (Voice& voice : voices_)
    {
        voice.noteOff(note, sustainPedalPressed_);
    }
}

void Synth::setNoiseMix(const float noiseMix)
{
    noiseMix_ = noiseMix;
}

void Synth::setOscillatorMix(const float oscillatorMix)
{
    oscillatorMix_ = oscillatorMix;
}

void Synth::setDetune(const float semi, const float cent)
{
    detune_ = std::pow(1.059463094359f, -semi - 0.01f * cent);
}

void Synth::setTune(const float tune)
{
    tune_ = tune;
}

void Synth::setPolyphonic(const bool polyphonic)
{
    numVoices_ = polyphonic ? maxNumVoices_ : 1;
}

void Synth::setOutputLevel(const float outputLevel)
{
    outputLevelSmoother_.setTargetValue(outputLevel);
}

void Synth::setFilterVelocity(const float filterVelocity)
{
    if (filterVelocity < -90.0f)
    {
        ignoreVelocity_ = true;
        velocitySensitivity_ = 0.0f;
    }
    else
    {
        ignoreVelocity_ = false;
        velocitySensitivity_ = filterVelocity * 0.0005f;
    }
}

void Synth::setLfoIncrement(const float lfoRateParam, const float inverseSampleRate)
{
    const float inverseUpdateRate = inverseSampleRate * lfoMaxSamplesPerUpdate_;
    const float lfoRate = std::exp(7.0f * lfoRateParam - 4.0f);
    lfoIncrement_ = lfoRate * inverseUpdateRate * constants::tau;
}

void Synth::setVibratoAmount(const float vibratoParam)
{
    const float vibrato = vibratoParam / 200.0f;
    vibratoAmount_ = 0.2f * vibrato * vibrato;
    pwmDepth_ = vibratoAmount_;
    if (vibrato < 0.0f)
    {
        vibratoAmount_ = 0.0f;
    }
}

void Synth::setGlide(const int glideMode, const float glideRate, const float glideBend, const float inverseSampleRate)
{
    const float inverseUpdateRate = inverseSampleRate * lfoMaxSamplesPerUpdate_;

    glideMode_ = static_cast<GlideMode>(glideMode);

    if (glideRate < 2.0f)
    {
        glideRate_ = 1.0f; // No glide.
    }
    else
    {
        glideRate_ = 1.0f - std::exp(-inverseUpdateRate * std::exp(6.0f - 0.07f * glideRate));
    }

    glideBend_ = glideBend;
}

void Synth::setFilterKeyTracking(const float filterKeyTrackingParam)
{
    filterKeyTracking_ = (0.08f * filterKeyTrackingParam) - 1.5f;
}

void Synth::setFilterQ(const float filterReso)
{
    filterQ_ = std::exp(3.0f * filterReso);
}

void Synth::setFilterLfoDepth(const float filterLfoDepth)
{
    filterLfoDepth_ = 2.5f * filterLfoDepth * filterLfoDepth;
}

void Synth::setFilterEnvelope(const float attack,
                              const float decay,
                              const float sustain,
                              const float release,
                              const float envDepth,
                              const float inverseSampleRate)
{
    const float inverseUpdateRate = inverseSampleRate * lfoMaxSamplesPerUpdate_;

    filterAttack_ = std::exp(-inverseUpdateRate * std::exp(5.5f - 0.075f * attack));
    filterDecay_ = std::exp(-inverseUpdateRate * std::exp(5.5f - 0.075f * decay));
    const float filterSustain{sustain / 100.0f};
    filterSustain_ = filterSustain * filterSustain;
    filterRelease_ = std::exp(-inverseUpdateRate * std::exp(5.5f - 0.075f * release));
    filterEnvDepth_ = 0.06f * envDepth;
}

void Synth::setOutputLevelInstantly(const float outputLevel)
{
    outputLevelSmoother_.setCurrentAndTargetValue(outputLevel);
}

void Synth::setEnvelopeDecay(const float decayTime)
{
    envelopeDecay_ = decayTime;
}

void Synth::setEnvelopeAttack(const float attackTime)
{
    envelopeAttack_ = attackTime;
}

void Synth::setEnvelopeSustain(const float sustainLevel)
{
    envelopeSustain_ = sustainLevel;
}

void Synth::setEnvelopeRelease(const float releaseTime)
{
    envelopeRelease_ = releaseTime;
}

} // namespace synth
