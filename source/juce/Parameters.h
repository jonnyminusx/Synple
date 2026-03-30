#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class Parameters
{
  public:
    explicit Parameters(juce::AudioProcessor& processor);

    juce::AudioProcessorValueTreeState& getApvts()
    {
        return apvts_;
    }

    // Typed value accessors
    float oscMix() const
    {
        return oscMixParam_->get();
    }
    float oscTune() const
    {
        return oscTuneParam_->get();
    }
    float oscFine() const
    {
        return oscFineParam_->get();
    }
    int glideMode() const
    {
        return glideModeParam_->getIndex();
    }
    float glideRate() const
    {
        return glideRateParam_->get();
    }
    float glideBend() const
    {
        return glideBendParam_->get();
    }
    float filterFreq() const
    {
        return filterFreqParam_->get();
    }
    float filterReso() const
    {
        return filterResoParam_->get();
    }
    float filterEnv() const
    {
        return filterEnvParam_->get();
    }
    float filterLFO() const
    {
        return filterLFOParam_->get();
    }
    float filterVelocity() const
    {
        return filterVelocityParam_->get();
    }
    float filterAttack() const
    {
        return filterAttackParam_->get();
    }
    float filterDecay() const
    {
        return filterDecayParam_->get();
    }
    float filterSustain() const
    {
        return filterSustainParam_->get();
    }
    float filterRelease() const
    {
        return filterReleaseParam_->get();
    }
    float envAttack() const
    {
        return envAttackParam_->get();
    }
    float envDecay() const
    {
        return envDecayParam_->get();
    }
    float envSustain() const
    {
        return envSustainParam_->get();
    }
    float envRelease() const
    {
        return envReleaseParam_->get();
    }
    float lfoRate() const
    {
        return lfoRateParam_->get();
    }
    float vibrato() const
    {
        return vibratoParam_->get();
    }
    float noise() const
    {
        return noiseParam_->get();
    }
    float octave() const
    {
        return octaveParam_->get();
    }
    float tuning() const
    {
        return tuningParam_->get();
    }
    float outputLevel() const
    {
        return outputLevelParam_->get();
    }
    int polyMode() const
    {
        return polyModeParam_->getIndex();
    }

    // Fills a 26-element array with all parameters in canonical preset order
    void fillParameterArray(juce::RangedAudioParameter** params) const;

    // Apply a normalised [0,1] volume CC to the output level parameter
    void setOutputLevelFromMidi(float normalised0to1);

  private:
    // apvts_ must be declared first so it is constructed before the raw pointers
    juce::AudioProcessorValueTreeState apvts_;

    juce::AudioParameterFloat* oscMixParam_{};
    juce::AudioParameterFloat* oscTuneParam_{};
    juce::AudioParameterFloat* oscFineParam_{};
    juce::AudioParameterChoice* glideModeParam_{};
    juce::AudioParameterFloat* glideRateParam_{};
    juce::AudioParameterFloat* glideBendParam_{};
    juce::AudioParameterFloat* filterFreqParam_{};
    juce::AudioParameterFloat* filterResoParam_{};
    juce::AudioParameterFloat* filterEnvParam_{};
    juce::AudioParameterFloat* filterLFOParam_{};
    juce::AudioParameterFloat* filterVelocityParam_{};
    juce::AudioParameterFloat* filterAttackParam_{};
    juce::AudioParameterFloat* filterDecayParam_{};
    juce::AudioParameterFloat* filterSustainParam_{};
    juce::AudioParameterFloat* filterReleaseParam_{};
    juce::AudioParameterFloat* envAttackParam_{};
    juce::AudioParameterFloat* envDecayParam_{};
    juce::AudioParameterFloat* envSustainParam_{};
    juce::AudioParameterFloat* envReleaseParam_{};
    juce::AudioParameterFloat* lfoRateParam_{};
    juce::AudioParameterFloat* vibratoParam_{};
    juce::AudioParameterFloat* noiseParam_{};
    juce::AudioParameterFloat* octaveParam_{};
    juce::AudioParameterFloat* tuningParam_{};
    juce::AudioParameterFloat* outputLevelParam_{};
    juce::AudioParameterChoice* polyModeParam_{};
};