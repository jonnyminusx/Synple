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
    juce::WebSliderRelay webOscMixRelay_{ParameterIds::oscMix.getParamID()};
    juce::WebSliderRelay webOscTuneRelay_{ParameterIds::oscTune.getParamID()};
    juce::WebSliderRelay webOscFineRelay_{ParameterIds::oscFine.getParamID()};
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

    juce::WebBrowserComponent webView_;

    // Attachments must be declared after webView_
    juce::WebSliderParameterAttachment webOscMixAttachment_;
    juce::WebSliderParameterAttachment webOscTuneAttachment_;
    juce::WebSliderParameterAttachment webOscFineAttachment_;
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynpleAudioProcessorEditor)
};
