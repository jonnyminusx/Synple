#include "Parameters.h"
#include "ParameterIDs.h"
#include "synth/Parameters.h"
#include <cmath>

namespace
{

template <typename T>
inline static void castParameter(juce::AudioProcessorValueTreeState& apvts, const juce::ParameterID& id, T& destination)
{
    destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
    jassert(destination && "parameter does not exist or wrong type");
}

static juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
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

} // namespace

Parameters::Parameters(juce::AudioProcessor& processor) : apvts_(processor, nullptr, "Parameters", createLayout())
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
}

juce::AudioProcessorValueTreeState& Parameters::getApvts()
{
    return apvts_;
}

float Parameters::oscillatorMix() const
{
    return oscMix() / 100.0f;
}

float Parameters::filterResonance() const
{
    return filterReso() / 100.0f;
}

float Parameters::filterEnvelopeDepth() const
{
    return 0.06f * filterEnv();
}

float Parameters::outputGain() const
{
    return juce::Decibels::decibelsToGain(outputLevel());
}

bool Parameters::isPolyphonic() const
{
    return polyMode() == 1;
}

int Parameters::glideModeIndex() const
{
    return glideMode();
}

float Parameters::glideBendSemitones() const
{
    return glideBend();
}

float Parameters::oscMix() const
{
    return oscMixParam_->get();
}

float Parameters::oscTune() const
{
    return oscTuneParam_->get();
}

float Parameters::oscFine() const
{
    return oscFineParam_->get();
}

int Parameters::glideMode() const
{
    return glideModeParam_->getIndex();
}

float Parameters::glideRate() const
{
    return glideRateParam_->get();
}

float Parameters::glideBend() const
{
    return glideBendParam_->get();
}

float Parameters::filterFreq() const
{
    return filterFreqParam_->get();
}

float Parameters::filterReso() const
{
    return filterResoParam_->get();
}

float Parameters::filterEnv() const
{
    return filterEnvParam_->get();
}

float Parameters::filterLFO() const
{
    return filterLFOParam_->get();
}

float Parameters::filterVelocity() const
{
    return filterVelocityParam_->get();
}

float Parameters::filterAttack() const
{
    return filterAttackParam_->get();
}

float Parameters::filterDecay() const
{
    return filterDecayParam_->get();
}

float Parameters::filterSustain() const
{
    return filterSustainParam_->get();
}

float Parameters::filterRelease() const
{
    return filterReleaseParam_->get();
}

float Parameters::envAttack() const
{
    return envAttackParam_->get();
}

float Parameters::envDecay() const
{
    return envDecayParam_->get();
}

float Parameters::envSustain() const
{
    return envSustainParam_->get();
}

float Parameters::envRelease() const
{
    return envReleaseParam_->get();
}

float Parameters::lfoRate() const
{
    return lfoRateParam_->get();
}

float Parameters::vibrato() const
{
    return vibratoParam_->get();
}

float Parameters::noise() const
{
    return noiseParam_->get();
}

float Parameters::octave() const
{
    return octaveParam_->get();
}

float Parameters::tuning() const
{
    return tuningParam_->get();
}

float Parameters::outputLevel() const
{
    return outputLevelParam_->get();
}

int Parameters::polyMode() const
{
    return polyModeParam_->getIndex();
}

float Parameters::multiplierFromParam(const float rateScale, const float paramValue)
{
    return std::exp(-rateScale * std::exp(5.5f - 0.075f * paramValue));
}

float Parameters::detune() const
{
    return std::pow(1.059463094359f, -oscTune() - 0.01f * oscFine());
}

float Parameters::tune(const float sampleRate) const
{
    const float tuneInSemi{-36.3763f - 12.0f * octave() - tuning() / 100.0f};
    return sampleRate * std::exp(0.05776226505f * tuneInSemi);
}

float Parameters::filterQ() const
{
    return std::exp(3.0f * filterResonance());
}

float Parameters::filterKeyTracking() const
{
    return (0.08f * filterFreq()) - 1.5f;
}

bool Parameters::shouldIgnoreVelocity() const
{
    return filterVelocity() < -90.0f;
}

float Parameters::filterVelocitySensitivity() const
{
    return shouldIgnoreVelocity() ? 0.0f : filterVelocity() * 0.0005f;
}

float Parameters::filterLfoDepth() const
{
    const float depth{filterLFO() / 100.0f};
    return 2.5f * depth * depth;
}

synth::ADSR Parameters::envelope(const float inverseSampleRate) const
{
    const float releaseParamValue{envRelease()};
    const float release = releaseParamValue < 1.0f ? 0.75f : multiplierFromParam(inverseSampleRate, releaseParamValue);

    return synth::ADSR{multiplierFromParam(inverseSampleRate, envAttack()),
                       multiplierFromParam(inverseSampleRate, envDecay()),
                       envSustain() / 100.0f,
                       release};
}

synth::ADSR Parameters::filterEnvelope(const float inverseUpdateRate) const
{
    const float sustain{filterSustain() / 100.0f};

    return synth::ADSR{multiplierFromParam(inverseUpdateRate, filterAttack()),
                       multiplierFromParam(inverseUpdateRate, filterDecay()),
                       sustain * sustain,
                       multiplierFromParam(inverseUpdateRate, filterRelease())};
}

float Parameters::noiseMix() const
{
    const float normalisedNoise{noise() / 100.0f};
    return normalisedNoise * normalisedNoise * 0.06f;
}

float Parameters::volumeTrim() const
{
    return 0.0008f * (3.2f - oscillatorMix() - 25.0f * noiseMix()) * (1.5f - 0.5f * filterResonance());
}

float Parameters::lfoIncrement(const float inverseSampleRate, const float updateInterval) const
{
    const float inverseUpdateRate = inverseSampleRate * updateInterval;
    const float lfoHz = std::exp(7.0f * lfoRate() - 4.0f);
    return lfoHz * inverseUpdateRate * juce::MathConstants<float>::twoPi;
}

float Parameters::vibratoAmount() const
{
    const float rawVibrato{vibrato() / 200.0f};
    if (rawVibrato < 0.0f)
    {
        return 0.0f;
    }

    return 0.2f * rawVibrato * rawVibrato;
}

float Parameters::pwmDepth() const
{
    const float rawVibrato{vibrato() / 200.0f};
    return 0.2f * rawVibrato * rawVibrato;
}

float Parameters::glideRateCoefficient(const float inverseSampleRate, const float updateInterval) const
{
    const float inverseUpdateRate = inverseSampleRate * updateInterval;

    if (glideRate() < 2.0f)
    {
        return 1.0f;
    }

    return 1.0f - std::exp(-inverseUpdateRate * std::exp(6.0f - 0.07f * glideRate()));
}

synth::Parameters Parameters::createSnapshot(const float sampleRate) const
{
    constexpr float modulationUpdateInterval{32.0f};
    const float inverseSampleRate{1.0f / sampleRate};
    const float inverseUpdateRate{inverseSampleRate * modulationUpdateInterval};

    synth::Parameters p;

    p.oscillator.mix = oscillatorMix();
    p.oscillator.tune = tune(sampleRate);
    p.oscillator.detune = detune();
    p.oscillator.noiseMix = noiseMix();

    p.filter.keyTracking = filterKeyTracking();
    p.filter.q = filterQ();
    p.filter.lfoDepth = filterLfoDepth();
    p.filter.envelopeDepth = filterEnvelopeDepth();
    p.filter.velocitySensitivity = filterVelocitySensitivity();
    p.filter.envelope = filterEnvelope(inverseUpdateRate);

    p.lfo.increment = lfoIncrement(inverseSampleRate, modulationUpdateInterval);
    p.lfo.vibratoAmount = vibratoAmount();
    p.lfo.pwmDepth = pwmDepth();

    p.glide.mode = static_cast<synth::GlideMode>(glideModeIndex());
    p.glide.rateCoefficient = glideRateCoefficient(inverseSampleRate, modulationUpdateInterval);
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
    params[0] = oscMixParam_;
    params[1] = oscTuneParam_;
    params[2] = oscFineParam_;
    params[3] = glideModeParam_;
    params[4] = glideRateParam_;
    params[5] = glideBendParam_;
    params[6] = filterFreqParam_;
    params[7] = filterResoParam_;
    params[8] = filterEnvParam_;
    params[9] = filterLFOParam_;
    params[10] = filterVelocityParam_;
    params[11] = filterAttackParam_;
    params[12] = filterDecayParam_;
    params[13] = filterSustainParam_;
    params[14] = filterReleaseParam_;
    params[15] = envAttackParam_;
    params[16] = envDecayParam_;
    params[17] = envSustainParam_;
    params[18] = envReleaseParam_;
    params[19] = lfoRateParam_;
    params[20] = vibratoParam_;
    params[21] = noiseParam_;
    params[22] = octaveParam_;
    params[23] = tuningParam_;
    params[24] = outputLevelParam_;
    params[25] = polyModeParam_;
}

void Parameters::setOutputLevelFromMidi(float normalised0to1)
{
    outputLevelParam_->beginChangeGesture();
    outputLevelParam_->setValueNotifyingHost(normalised0to1);
    outputLevelParam_->endChangeGesture();
}
