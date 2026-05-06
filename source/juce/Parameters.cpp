#include "Parameters.h"
#include "ParameterIds.h"
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
        ParameterIds::oscMix,
        "Osc Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction(oscMixStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::oscTune,
                                                           "Osc Tune",
                                                           juce::NormalisableRange<float>(-24.0f, 24.0f, 1.0f),
                                                           -12.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("semi")));

    layout.add(
        std::make_unique<juce::AudioParameterFloat>(ParameterIds::oscFine,
                                                    "Osc Fine",
                                                    juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f, 0.3f, true),
                                                    0.0f,
                                                    juce::AudioParameterFloatAttributes().withLabel("cent")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterIds::glideMode, "Glide Mode", juce::StringArray{"Off", "Legato", "Always"}, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::glideRate,
                                                           "Glide Rate",
                                                           juce::NormalisableRange<float>(0.0f, 100.f, 1.0f),
                                                           35.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(
        std::make_unique<juce::AudioParameterFloat>(ParameterIds::glideBend,
                                                    "Glide Bend",
                                                    juce::NormalisableRange<float>(-36.0f, 36.0f, 0.01f, 0.4f, true),
                                                    0.0f,
                                                    juce::AudioParameterFloatAttributes().withLabel("semi")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::filterFreq,
                                                           "Filter Freq",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
                                                           100.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::filterReso,
                                                           "Filter Reso",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           15.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::filterEnv,
                                                           "Filter Env",
                                                           juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
                                                           50.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::filterLFO,
                                                           "Filter LFO",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           0.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    auto filterVelocityStringFromValue = [](float value, int) {
        return value < -90.0f ? juce::String("OFF") : juce::String(value);
    };

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIds::filterVelocity,
        "Velocity",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction(
            filterVelocityStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::filterAttack,
                                                           "Filter Attack",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           0.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::filterDecay,
                                                           "Filter Decay",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           30.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::filterSustain,
                                                           "Filter Sustain",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           0.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::filterRelease,
                                                           "Filter Release",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           25.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::envAttack,
                                                           "Env Attack",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           0.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::envDecay,
                                                           "Env Decay",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           50.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::envSustain,
                                                           "Env Sustain",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           100.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::envRelease,
                                                           "Env Release",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           30.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    auto lfoRateStringFromValue = [](float value, int) {
        const float lfoHz = std::exp(7.0f * value - 4.0f);
        return juce::String(lfoHz, 3);
    };

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIds::lfoRate,
        "LFO Rate",
        juce::NormalisableRange<float>(),
        0.81f,
        juce::AudioParameterFloatAttributes().withLabel("Hz").withStringFromValueFunction(lfoRateStringFromValue)));

    auto vibratoStringFromValue = [](float value, int) {
        return value < 0.0f ? "PWM " + juce::String(-value, 1) : juce::String(value, 1);
    };

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIds::vibrato,
        "Vibrato",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction(vibratoStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::noise,
                                                           "Noise",
                                                           juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
                                                           0.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIds::octave, "Octave", juce::NormalisableRange<float>(-2.0f, 2.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::tuning,
                                                           "Tuning",
                                                           juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
                                                           0.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("cent")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterIds::outputLevel,
                                                           "Output Level",
                                                           juce::NormalisableRange<float>(-24.0f, 6.0f, 0.1f),
                                                           0.0f,
                                                           juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterIds::polyMode, "Polyphony", juce::StringArray{"Mono", "Poly"}, 1));

    return layout;
}

} // namespace

Parameters::Parameters(juce::AudioProcessor& processor) : apvts_(processor, nullptr, "Parameters", createLayout())
{
    castParameter(apvts_, ParameterIds::oscMix, oscMixParam_);
    castParameter(apvts_, ParameterIds::oscTune, oscTuneParam_);
    castParameter(apvts_, ParameterIds::oscFine, oscFineParam_);
    castParameter(apvts_, ParameterIds::glideMode, glideModeParam_);
    castParameter(apvts_, ParameterIds::glideRate, glideRateParam_);
    castParameter(apvts_, ParameterIds::glideBend, glideBendParam_);
    castParameter(apvts_, ParameterIds::filterFreq, filterFreqParam_);
    castParameter(apvts_, ParameterIds::filterReso, filterResoParam_);
    castParameter(apvts_, ParameterIds::filterEnv, filterEnvParam_);
    castParameter(apvts_, ParameterIds::filterLFO, filterLFOParam_);
    castParameter(apvts_, ParameterIds::filterVelocity, filterVelocityParam_);
    castParameter(apvts_, ParameterIds::filterAttack, filterAttackParam_);
    castParameter(apvts_, ParameterIds::filterDecay, filterDecayParam_);
    castParameter(apvts_, ParameterIds::filterSustain, filterSustainParam_);
    castParameter(apvts_, ParameterIds::filterRelease, filterReleaseParam_);
    castParameter(apvts_, ParameterIds::envAttack, envAttackParam_);
    castParameter(apvts_, ParameterIds::envDecay, envDecayParam_);
    castParameter(apvts_, ParameterIds::envSustain, envSustainParam_);
    castParameter(apvts_, ParameterIds::envRelease, envReleaseParam_);
    castParameter(apvts_, ParameterIds::lfoRate, lfoRateParam_);
    castParameter(apvts_, ParameterIds::vibrato, vibratoParam_);
    castParameter(apvts_, ParameterIds::noise, noiseParam_);
    castParameter(apvts_, ParameterIds::octave, octaveParam_);
    castParameter(apvts_, ParameterIds::tuning, tuningParam_);
    castParameter(apvts_, ParameterIds::outputLevel, outputLevelParam_);
    castParameter(apvts_, ParameterIds::polyMode, polyModeParam_);
}

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

float Parameters::oscillatorMix() const
{
    return oscMixParam_->get() / 100.0f;
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

float Parameters::detune() const
{
    return std::pow(1.059463094359f, -oscTuneParam_->get() - 0.01f * oscFineParam_->get());
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
    return 0.0008f * (3.2f - oscillatorMix() - 25.0f * noiseMix()) * (1.5f - 0.5f * filterResonance());
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
    if (rawVibrato < 0.0f)
    {
        return 0.0f;
    }

    return 0.2f * rawVibrato * rawVibrato;
}

float Parameters::pwmDepth() const
{
    const float rawVibrato{vibratoParam_->get() / 200.0f};
    return 0.2f * rawVibrato * rawVibrato;
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
    constexpr float kModulationUpdateInterval{32.0f};
    const float inverseSampleRate{1.0f / sampleRate};
    const float inverseUpdateRate{inverseSampleRate * kModulationUpdateInterval};

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
