#pragma once

#include "ParameterIds.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include <optional>

class SynpleAudioProcessor;

//==============================================================================
class SynpleAudioProcessorEditor final : public juce::AudioProcessorEditor
{
  public:
    explicit SynpleAudioProcessorEditor(SynpleAudioProcessor&);
    ~SynpleAudioProcessorEditor() override;

    void resized() override;

  private:
    using Resource = juce::WebBrowserComponent::Resource;
    std::optional<Resource> getResource(const juce::String& url);

    SynpleAudioProcessor& processorRef;

    // Relays must be declared before webView_
    juce::WebSliderRelay webOsc1VolumeRelay_{ParameterIds::osc1Volume.getParamID()};
    juce::WebSliderRelay webOsc2VolumeRelay_{ParameterIds::osc2Volume.getParamID()};
    juce::WebSliderRelay webOsc1TuneRelay_{ParameterIds::osc1Tune.getParamID()};
    juce::WebSliderRelay webOsc2TuneRelay_{ParameterIds::osc2Tune.getParamID()};
    juce::WebSliderRelay webOsc1FineRelay_{ParameterIds::osc1Fine.getParamID()};
    juce::WebSliderRelay webOsc2FineRelay_{ParameterIds::osc2Fine.getParamID()};
    juce::WebComboBoxRelay webOsc1WaveformRelay_{ParameterIds::osc1Waveform.getParamID()};
    juce::WebComboBoxRelay webOsc2WaveformRelay_{ParameterIds::osc2Waveform.getParamID()};
    juce::WebSliderRelay webOsc1PulseWidthRelay_{ParameterIds::osc1PulseWidth.getParamID()};
    juce::WebSliderRelay webOsc2PulseWidthRelay_{ParameterIds::osc2PulseWidth.getParamID()};
    juce::WebComboBoxRelay webGlideModeRelay_{ParameterIds::glideMode.getParamID()};
    juce::WebSliderRelay webGlideRateRelay_{ParameterIds::glideRate.getParamID()};
    juce::WebSliderRelay webGlideBendRelay_{ParameterIds::glideBend.getParamID()};
    juce::WebSliderRelay webFilterFreqRelay_{ParameterIds::filterFreq.getParamID()};
    juce::WebSliderRelay webFilterResoRelay_{ParameterIds::filterReso.getParamID()};
    juce::WebSliderRelay webFilterEnvRelay_{ParameterIds::filterEnv.getParamID()};
    juce::WebSliderRelay webFilterLFORelay_{ParameterIds::filterLFO.getParamID()};
    juce::WebSliderRelay webFilterVelocityRelay_{ParameterIds::filterVelocity.getParamID()};
    juce::WebSliderRelay webFilterAttackRelay_{ParameterIds::filterAttack.getParamID()};
    juce::WebSliderRelay webFilterDecayRelay_{ParameterIds::filterDecay.getParamID()};
    juce::WebSliderRelay webFilterSustainRelay_{ParameterIds::filterSustain.getParamID()};
    juce::WebSliderRelay webFilterReleaseRelay_{ParameterIds::filterRelease.getParamID()};
    juce::WebSliderRelay webEnvAttackRelay_{ParameterIds::envAttack.getParamID()};
    juce::WebSliderRelay webEnvDecayRelay_{ParameterIds::envDecay.getParamID()};
    juce::WebSliderRelay webEnvSustainRelay_{ParameterIds::envSustain.getParamID()};
    juce::WebSliderRelay webEnvReleaseRelay_{ParameterIds::envRelease.getParamID()};
    juce::WebSliderRelay webLfoRateRelay_{ParameterIds::lfoRate.getParamID()};
    juce::WebSliderRelay webVibratoRelay_{ParameterIds::vibrato.getParamID()};
    juce::WebSliderRelay webNoiseRelay_{ParameterIds::noise.getParamID()};
    juce::WebSliderRelay webOctaveRelay_{ParameterIds::octave.getParamID()};
    juce::WebSliderRelay webTuningRelay_{ParameterIds::tuning.getParamID()};
    juce::WebSliderRelay webOutputLevelRelay_{ParameterIds::outputLevel.getParamID()};
    juce::WebComboBoxRelay webPolyModeRelay_{ParameterIds::polyMode.getParamID()};
    juce::WebSliderRelay webPwmDepthRelay_{ParameterIds::pwmDepth.getParamID()};

    juce::WebBrowserComponent webView_;

    // Attachments must be declared after webView_
    juce::WebSliderParameterAttachment webOsc1VolumeAttachment_;
    juce::WebSliderParameterAttachment webOsc2VolumeAttachment_;
    juce::WebSliderParameterAttachment webOsc1TuneAttachment_;
    juce::WebSliderParameterAttachment webOsc2TuneAttachment_;
    juce::WebSliderParameterAttachment webOsc1FineAttachment_;
    juce::WebSliderParameterAttachment webOsc2FineAttachment_;
    juce::WebComboBoxParameterAttachment webOsc1WaveformAttachment_;
    juce::WebComboBoxParameterAttachment webOsc2WaveformAttachment_;
    juce::WebSliderParameterAttachment webOsc1PulseWidthAttachment_;
    juce::WebSliderParameterAttachment webOsc2PulseWidthAttachment_;
    juce::WebComboBoxParameterAttachment webGlideModeAttachment_;
    juce::WebSliderParameterAttachment webGlideRateAttachment_;
    juce::WebSliderParameterAttachment webGlideBendAttachment_;
    juce::WebSliderParameterAttachment webFilterFreqAttachment_;
    juce::WebSliderParameterAttachment webFilterResoAttachment_;
    juce::WebSliderParameterAttachment webFilterEnvAttachment_;
    juce::WebSliderParameterAttachment webFilterLFOAttachment_;
    juce::WebSliderParameterAttachment webFilterVelocityAttachment_;
    juce::WebSliderParameterAttachment webFilterAttackAttachment_;
    juce::WebSliderParameterAttachment webFilterDecayAttachment_;
    juce::WebSliderParameterAttachment webFilterSustainAttachment_;
    juce::WebSliderParameterAttachment webFilterReleaseAttachment_;
    juce::WebSliderParameterAttachment webEnvAttackAttachment_;
    juce::WebSliderParameterAttachment webEnvDecayAttachment_;
    juce::WebSliderParameterAttachment webEnvSustainAttachment_;
    juce::WebSliderParameterAttachment webEnvReleaseAttachment_;
    juce::WebSliderParameterAttachment webLfoRateAttachment_;
    juce::WebSliderParameterAttachment webVibratoAttachment_;
    juce::WebSliderParameterAttachment webNoiseAttachment_;
    juce::WebSliderParameterAttachment webOctaveAttachment_;
    juce::WebSliderParameterAttachment webTuningAttachment_;
    juce::WebSliderParameterAttachment webOutputLevelAttachment_;
    juce::WebComboBoxParameterAttachment webPolyModeAttachment_;
    juce::WebSliderParameterAttachment webPwmDepthAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynpleAudioProcessorEditor)
};
