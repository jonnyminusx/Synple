#include "Parameters.h"
#include "ParameterIDs.h"
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
