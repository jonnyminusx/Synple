#include "Parameters.h"
#include "ParameterIds.h"
#include "synth/Parameters.h"
#include "synth/Synth.h"
#include <cmath>

namespace
{
constexpr float kSemitoneRatio{1.0594630943592952f}; // 2^(1/12)
} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout Parameters::buildLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto addParam = [&]<typename T, typename... Args>(T*& dest, Args&&... args) {
        auto p = std::make_unique<T>(std::forward<Args>(args)...);
        dest = p.get();
        layout.add(std::move(p));
    };

    addParam(glideModeParam_, parameter_id::glideMode, "Glide Mode", juce::StringArray{"Off", "Legato", "Always"}, 0);

    addParam(glideRateParam_,
             parameter_id::glideRate,
             "Glide Rate",
             juce::NormalisableRange<float>(0.0f, 100.f, 1.0f),
             35.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(glideBendParam_,
             parameter_id::glideBend,
             "Glide Bend",
             juce::NormalisableRange<float>(-36.0f, 36.0f, 0.01f, 0.4f, true),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("semi"));

    addParam(filterFreqParam_,
             parameter_id::filterFreq,
             "Filter Freq",
             juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
             100.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(filterResoParam_,
             parameter_id::filterReso,
             "Filter Reso",
             juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
             15.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(filterEnvParam_,
             parameter_id::filterEnv,
             "Filter Env",
             juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
             50.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(filterLFOParam_,
             parameter_id::filterLFO,
             "Filter LFO",
             juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    auto filterVelocityStringFromValue = [](float value, int) {
        return value < -90.0f ? juce::String("OFF") : juce::String(value);
    };

    addParam(filterVelocityParam_,
             parameter_id::filterVelocity,
             "Velocity",
             juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction(
                 filterVelocityStringFromValue));

    addParam(filterAttackParam_,
             parameter_id::filterAttack,
             "Filter Attack",
             juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(filterDecayParam_,
             parameter_id::filterDecay,
             "Filter Decay",
             juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
             30.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(filterSustainParam_,
             parameter_id::filterSustain,
             "Filter Sustain",
             juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(filterReleaseParam_,
             parameter_id::filterRelease,
             "Filter Release",
             juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
             25.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(envAttackParam_,
             parameter_id::envAttack,
             "Env Attack",
             juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(envDecayParam_,
             parameter_id::envDecay,
             "Env Decay",
             juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
             50.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(envSustainParam_,
             parameter_id::envSustain,
             "Env Sustain",
             juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
             100.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(envReleaseParam_,
             parameter_id::envRelease,
             "Env Release",
             juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
             30.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    auto lfoRateStringFromValue = [](float value, int) {
        const float lfoHz = std::exp(7.0f * value - 4.0f);
        return juce::String(lfoHz, 3);
    };

    addParam(lfoRateParam_,
             parameter_id::lfoRate,
             "LFO Rate",
             juce::NormalisableRange<float>(),
             0.81f,
             juce::AudioParameterFloatAttributes().withLabel("Hz").withStringFromValueFunction(lfoRateStringFromValue));

    addParam(vibratoParam_,
             parameter_id::vibrato,
             "Vibrato",
             juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(noiseParam_,
             parameter_id::noise,
             "Noise",
             juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(octaveParam_, parameter_id::octave, "Octave", juce::NormalisableRange<float>(-2.0f, 2.0f, 1.0f), 0.0f);

    addParam(tuningParam_,
             parameter_id::tuning,
             "Tuning",
             juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("cent"));

    addParam(outputLevelParam_,
             parameter_id::outputLevel,
             "Output Level",
             juce::NormalisableRange<float>(-24.0f, 6.0f, 0.1f),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("dB"));

    addParam(polyModeParam_, parameter_id::polyMode, "Polyphony", juce::StringArray{"Mono", "Poly"}, 1);

    addParam(pwmDepthParam_,
             parameter_id::pwmDepth,
             "PWM Depth",
             juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(osc1VolumeParam_,
             parameter_id::osc1Volume,
             "Osc 1 Volume",
             juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
             100.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(osc2VolumeParam_,
             parameter_id::osc2Volume,
             "Osc 2 Volume",
             juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(osc1TuneParam_,
             parameter_id::osc1Tune,
             "Osc 1 Tune",
             juce::NormalisableRange<float>(-24.0f, 24.0f, 1.0f),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("semi"));

    addParam(osc2TuneParam_,
             parameter_id::osc2Tune,
             "Osc 2 Tune",
             juce::NormalisableRange<float>(-24.0f, 24.0f, 1.0f),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("semi"));

    addParam(osc1FineParam_,
             parameter_id::osc1Fine,
             "Osc 1 Fine",
             juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f, 0.3f, true),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("cent"));

    addParam(osc2FineParam_,
             parameter_id::osc2Fine,
             "Osc 2 Fine",
             juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f, 0.3f, true),
             0.0f,
             juce::AudioParameterFloatAttributes().withLabel("cent"));

    addParam(osc1WaveformParam_,
             parameter_id::osc1Waveform,
             "Osc 1 Waveform",
             juce::StringArray{"Sawtooth", "Sine", "Pulse"},
             0);

    addParam(osc2WaveformParam_,
             parameter_id::osc2Waveform,
             "Osc 2 Waveform",
             juce::StringArray{"Sawtooth", "Sine", "Pulse"},
             0);

    addParam(osc1PulseWidthParam_,
             parameter_id::osc1PulseWidth,
             "Osc 1 Pulse Width",
             juce::NormalisableRange<float>(10.0f, 90.0f, 0.1f),
             50.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    addParam(osc2PulseWidthParam_,
             parameter_id::osc2PulseWidth,
             "Osc 2 Pulse Width",
             juce::NormalisableRange<float>(10.0f, 90.0f, 0.1f),
             50.0f,
             juce::AudioParameterFloatAttributes().withLabel("%"));

    return layout;
}

Parameters::Parameters(juce::AudioProcessor& processor) : apvts_(processor, nullptr, "Parameters", buildLayout()) {}

juce::RangedAudioParameter& Parameters::getParameter(const juce::ParameterID& id)
{
    auto* param = apvts_.getParameter(id.getParamID());
    jassert(param != nullptr);
    return *param;
}

void Parameters::addStateListener(juce::ValueTree::Listener* listener)
{
    apvts_.state.addListener(listener);
}

void Parameters::removeStateListener(juce::ValueTree::Listener* listener)
{
    apvts_.state.removeListener(listener);
}

std::unique_ptr<juce::XmlElement> Parameters::copyStateToXml()
{
    return apvts_.copyState().createXml();
}

bool Parameters::restoreStateFromXml(const juce::XmlElement& parentXml)
{
    if (auto* child = parentXml.getChildByName(apvts_.state.getType()))
    {
        apvts_.replaceState(juce::ValueTree::fromXml(*child));
        return true;
    }
    return false;
}

float Parameters::osc1Volume() const
{
    return osc1VolumeParam_->get() / 100.0f;
}

float Parameters::osc2Volume() const
{
    return osc2VolumeParam_->get() / 100.0f;
}

float Parameters::osc1TuneFactor() const
{
    return std::pow(kSemitoneRatio, -(osc1TuneParam_->get() + 0.01f * osc1FineParam_->get()));
}

float Parameters::osc2TuneFactor() const
{
    return std::pow(kSemitoneRatio, -(osc2TuneParam_->get() + 0.01f * osc2FineParam_->get()));
}

float Parameters::osc1PulseWidth() const
{
    return osc1PulseWidthParam_->get() / 100.0f;
}

float Parameters::osc2PulseWidth() const
{
    return osc2PulseWidthParam_->get() / 100.0f;
}

float Parameters::filterResonance() const
{
    return filterResoParam_->get() / 100.0f;
}

float Parameters::filterEnvelopeDepth() const
{
    return 0.06f * filterEnvParam_->get();
}

float Parameters::outputGain() const
{
    return juce::Decibels::decibelsToGain(outputLevelParam_->get());
}

bool Parameters::isPolyphonic() const
{
    return polyModeParam_->getIndex() == 1;
}

int Parameters::glideModeIndex() const
{
    return glideModeParam_->getIndex();
}

float Parameters::glideBendSemitones() const
{
    return glideBendParam_->get();
}

float Parameters::multiplierFromParam(const float rateScale, const float paramValue)
{
    return std::exp(-rateScale * std::exp(5.5f - 0.075f * paramValue));
}

float Parameters::tune(const float sampleRate) const
{
    const float tuneInSemi{-36.3763f - 12.0f * octaveParam_->get() - tuningParam_->get() / 100.0f};
    return sampleRate * std::exp(0.05776226505f * tuneInSemi);
}

float Parameters::filterQ() const
{
    return std::exp(3.0f * filterResonance());
}

float Parameters::filterKeyTracking() const
{
    return (0.08f * filterFreqParam_->get()) - 1.5f;
}

bool Parameters::shouldIgnoreVelocity() const
{
    return filterVelocityParam_->get() < -90.0f;
}

float Parameters::filterVelocitySensitivity() const
{
    return shouldIgnoreVelocity() ? 0.0f : filterVelocityParam_->get() * 0.0005f;
}

float Parameters::filterLfoDepth() const
{
    const float depth{filterLFOParam_->get() / 100.0f};
    return 2.5f * depth * depth;
}

synth::Parameters::ADSR Parameters::envelope(const float inverseSampleRate) const
{
    const float releaseParamValue{envReleaseParam_->get()};
    const float release = releaseParamValue < 1.0f ? 0.75f : multiplierFromParam(inverseSampleRate, releaseParamValue);

    return synth::Parameters::ADSR{multiplierFromParam(inverseSampleRate, envAttackParam_->get()),
                                   multiplierFromParam(inverseSampleRate, envDecayParam_->get()),
                                   envSustainParam_->get() / 100.0f,
                                   release};
}

synth::Parameters::ADSR Parameters::filterEnvelope(const float inverseUpdateRate) const
{
    const float sustain{filterSustainParam_->get() / 100.0f};

    return synth::Parameters::ADSR{multiplierFromParam(inverseUpdateRate, filterAttackParam_->get()),
                                   multiplierFromParam(inverseUpdateRate, filterDecayParam_->get()),
                                   sustain * sustain,
                                   multiplierFromParam(inverseUpdateRate, filterReleaseParam_->get())};
}

float Parameters::noiseMix() const
{
    const float normalisedNoise{noiseParam_->get() / 100.0f};
    return normalisedNoise * normalisedNoise * 0.06f;
}

float Parameters::volumeTrim() const
{
    return 0.0008f * (3.2f - 25.0f * noiseMix()) * (1.5f - 0.5f * filterResonance());
}

float Parameters::lfoIncrement(const float inverseSampleRate, const float updateInterval) const
{
    const float inverseUpdateRate = inverseSampleRate * updateInterval;
    const float lfoHz = std::exp(7.0f * lfoRateParam_->get() - 4.0f);
    return lfoHz * inverseUpdateRate * juce::MathConstants<float>::twoPi;
}

float Parameters::vibratoAmount() const
{
    const float rawVibrato{vibratoParam_->get() / 200.0f};
    return 0.2f * rawVibrato * rawVibrato;
}

float Parameters::pwmDepth() const
{
    return pwmDepthParam_->get() / 100.0f;
}

float Parameters::glideRateCoefficient(const float inverseSampleRate, const float updateInterval) const
{
    const float inverseUpdateRate = inverseSampleRate * updateInterval;

    if (glideRateParam_->get() < 2.0f)
    {
        return 1.0f;
    }

    return 1.0f - std::exp(-inverseUpdateRate * std::exp(6.0f - 0.07f * glideRateParam_->get()));
}

synth::Parameters Parameters::createSnapshot(const float sampleRate) const
{
    const float kModulationUpdateInterval{static_cast<float>(synth::Synth::lfoMaxSamplesPerUpdate())};
    const float inverseSampleRate{1.0f / sampleRate};
    const float inverseUpdateRate{inverseSampleRate * kModulationUpdateInterval};

    synth::Parameters p;

    p.oscillator.globalTune = tune(sampleRate);
    p.oscillator.osc1.tune = osc1TuneFactor();
    p.oscillator.osc2.tune = osc2TuneFactor();
    p.oscillator.osc1.volume = osc1Volume();
    p.oscillator.osc2.volume = osc2Volume();
    p.oscillator.noiseMix = noiseMix();
    p.oscillator.osc1.pulseWidth = osc1PulseWidth();
    p.oscillator.osc2.pulseWidth = osc2PulseWidth();
    p.oscillator.osc1.waveform = static_cast<synth::WaveformType>(osc1WaveformParam_->getIndex());
    p.oscillator.osc2.waveform = static_cast<synth::WaveformType>(osc2WaveformParam_->getIndex());

    p.filter.keyTracking = filterKeyTracking();
    p.filter.q = filterQ();
    p.filter.lfoDepth = filterLfoDepth();
    p.filter.envelopeDepth = filterEnvelopeDepth();
    p.filter.velocitySensitivity = filterVelocitySensitivity();
    p.filter.envelope = filterEnvelope(inverseUpdateRate);

    p.lfo.increment = lfoIncrement(inverseSampleRate, kModulationUpdateInterval);
    p.lfo.vibratoAmount = vibratoAmount();
    p.lfo.pwmDepth = pwmDepth();

    p.glide.mode = static_cast<synth::GlideMode>(glideModeIndex());
    p.glide.rateCoefficient = glideRateCoefficient(inverseSampleRate, kModulationUpdateInterval);
    p.glide.bendSemitones = glideBendSemitones();

    p.output.gain = outputGain();
    p.output.volumeTrim = volumeTrim();
    p.output.polyphonic = isPolyphonic();
    p.output.ignoreVelocity = shouldIgnoreVelocity();

    p.envelope = envelope(inverseSampleRate);

    return p;
}

void Parameters::fillParameterArray(juce::RangedAudioParameter** params) const
{
    params[0] = glideModeParam_;
    params[1] = glideRateParam_;
    params[2] = glideBendParam_;
    params[3] = filterFreqParam_;
    params[4] = filterResoParam_;
    params[5] = filterEnvParam_;
    params[6] = filterLFOParam_;
    params[7] = filterVelocityParam_;
    params[8] = filterAttackParam_;
    params[9] = filterDecayParam_;
    params[10] = filterSustainParam_;
    params[11] = filterReleaseParam_;
    params[12] = envAttackParam_;
    params[13] = envDecayParam_;
    params[14] = envSustainParam_;
    params[15] = envReleaseParam_;
    params[16] = lfoRateParam_;
    params[17] = vibratoParam_;
    params[18] = noiseParam_;
    params[19] = octaveParam_;
    params[20] = tuningParam_;
    params[21] = outputLevelParam_;
    params[22] = polyModeParam_;
    params[23] = pwmDepthParam_;
    params[24] = osc1VolumeParam_;
    params[25] = osc2VolumeParam_;
    params[26] = osc1TuneParam_;
    params[27] = osc2TuneParam_;
    params[28] = osc1FineParam_;
    params[29] = osc2FineParam_;
    params[30] = osc1WaveformParam_;
    params[31] = osc2WaveformParam_;
    params[32] = osc1PulseWidthParam_;
    params[33] = osc2PulseWidthParam_;
}

void Parameters::setOutputLevelFromMidi(float normalised0to1)
{
    outputLevelParam_->beginChangeGesture();
    outputLevelParam_->setValueNotifyingHost(normalised0to1);
    outputLevelParam_->endChangeGesture();
}
