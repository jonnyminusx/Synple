#pragma once

#include "ParameterIDs.h"
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
    juce::WebSliderRelay webOscMixRelay_{parameter_id::oscMix.getParamID()};
    juce::WebSliderRelay webOscTuneRelay_{parameter_id::oscTune.getParamID()};
    juce::WebSliderRelay webOscFineRelay_{parameter_id::oscFine.getParamID()};
    juce::WebComboBoxRelay webGlideModeRelay_{parameter_id::glideMode.getParamID()};
    juce::WebSliderRelay webGlideRateRelay_{parameter_id::glideRate.getParamID()};
    juce::WebSliderRelay webGlideBendRelay_{parameter_id::glideBend.getParamID()};
    juce::WebSliderRelay webFilterFreqRelay_{parameter_id::filterFreq.getParamID()};
    juce::WebSliderRelay webFilterResoRelay_{parameter_id::filterReso.getParamID()};
    juce::WebSliderRelay webFilterEnvRelay_{parameter_id::filterEnv.getParamID()};
    juce::WebSliderRelay webFilterLFORelay_{parameter_id::filterLFO.getParamID()};
    juce::WebSliderRelay webFilterVelocityRelay_{parameter_id::filterVelocity.getParamID()};
    juce::WebSliderRelay webFilterAttackRelay_{parameter_id::filterAttack.getParamID()};
    juce::WebSliderRelay webFilterDecayRelay_{parameter_id::filterDecay.getParamID()};
    juce::WebSliderRelay webFilterSustainRelay_{parameter_id::filterSustain.getParamID()};
    juce::WebSliderRelay webFilterReleaseRelay_{parameter_id::filterRelease.getParamID()};
    juce::WebSliderRelay webEnvAttackRelay_{parameter_id::envAttack.getParamID()};
    juce::WebSliderRelay webEnvDecayRelay_{parameter_id::envDecay.getParamID()};
    juce::WebSliderRelay webEnvSustainRelay_{parameter_id::envSustain.getParamID()};
    juce::WebSliderRelay webEnvReleaseRelay_{parameter_id::envRelease.getParamID()};
    juce::WebSliderRelay webLfoRateRelay_{parameter_id::lfoRate.getParamID()};
    juce::WebSliderRelay webVibratoRelay_{parameter_id::vibrato.getParamID()};
    juce::WebSliderRelay webNoiseRelay_{parameter_id::noise.getParamID()};
    juce::WebSliderRelay webOctaveRelay_{parameter_id::octave.getParamID()};
    juce::WebSliderRelay webTuningRelay_{parameter_id::tuning.getParamID()};
    juce::WebSliderRelay webOutputLevelRelay_{parameter_id::outputLevel.getParamID()};
    juce::WebComboBoxRelay webPolyModeRelay_{parameter_id::polyMode.getParamID()};

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
