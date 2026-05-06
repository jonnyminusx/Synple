#pragma once

#include "synth/Parameters.h"
#include <juce_audio_processors/juce_audio_processors.h>

class Parameters
{
  public:
    explicit Parameters(juce::AudioProcessor& processor);

    juce::RangedAudioParameter& getParameter(const juce::ParameterID& id);
    void addStateListener(juce::ValueTree::Listener* listener);
    void removeStateListener(juce::ValueTree::Listener* listener);
    std::unique_ptr<juce::XmlElement> copyStateToXml();
    bool restoreStateFromXml(const juce::XmlElement& parentXml);

    float outputGain() const;

    // Build an engine-ready snapshot of all parameters at the given sample rate.
    // All raw-to-engine-value transformations occur here; the returned struct is
    // directly usable by the synth with no further conversion.
    synth::Parameters createSnapshot(float sampleRate) const;

    // Fills a 26-element array with all parameters in canonical preset order
    void fillParameterArray(juce::RangedAudioParameter** params) const;

    // Apply a normalised [0,1] volume CC to the output level parameter
    void setOutputLevelFromMidi(float normalised0to1);

  private:
    static float multiplierFromParam(float rateScale, float paramValue);

    float oscillatorMix() const;
    float detune() const;
    float tune(float sampleRate) const;
    float filterResonance() const;
    float filterQ() const;
    float filterKeyTracking() const;
    bool shouldIgnoreVelocity() const;
    float filterVelocitySensitivity() const;
    float filterLfoDepth() const;
    float filterEnvelopeDepth() const;
    synth::ADSR envelope(float inverseSampleRate) const;
    synth::ADSR filterEnvelope(float inverseUpdateRate) const;
    float noiseMix() const;
    float volumeTrim() const;
    bool isPolyphonic() const;
    float lfoIncrement(float inverseSampleRate, float updateInterval) const;
    float vibratoAmount() const;
    float pwmDepth() const;
    int glideModeIndex() const;
    float glideBendSemitones() const;
    float glideRateCoefficient(float inverseSampleRate, float updateInterval) const;

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
