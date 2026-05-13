#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "WebViewFiles.h"

#include <ranges>

namespace
{

auto streamToVector(juce::InputStream& stream)
{
    std::vector<std::byte> result((size_t)stream.getTotalLength());
    stream.setPosition(0);
    [[maybe_unused]] const auto bytesRead = stream.read(result.data(), result.size());
    jassert(bytesRead == (ssize_t)result.size());
    return result;
}

const char* getMimeForExtension(const juce::String& extension)
{
    static const std::unordered_map<juce::String, const char*> mimeMap = {{{"htm"}, "text/html"},
                                                                          {{"html"}, "text/html"},
                                                                          {{"txt"}, "text/plain"},
                                                                          {{"jpg"}, "image/jpeg"},
                                                                          {{"jpeg"}, "image/jpeg"},
                                                                          {{"svg"}, "image/svg+xml"},
                                                                          {{"ico"}, "image/vnd.microsoft.icon"},
                                                                          {{"json"}, "application/json"},
                                                                          {{"png"}, "image/png"},
                                                                          {{"css"}, "text/css"},
                                                                          {{"map"}, "application/json"},
                                                                          {{"js"}, "text/javascript"},
                                                                          {{"woff2"}, "font/woff2"}};

    if (const auto it = mimeMap.find(extension.toLowerCase()); it != mimeMap.end())
        return it->second;

    jassertfalse;
    return "";
}

#ifndef ZIPPED_FILES_PREFIX
#error                                                                                                                 \
    "You must provide the prefix of zipped web UI files' paths, e.g., 'public/', in the ZIPPED_FILES_PREFIX compile definition"
#endif

std::vector<std::byte> getWebViewFileAsBytes(const juce::String& filepath)
{
    juce::MemoryInputStream zipStream{webview_files::webview_files_zip, webview_files::webview_files_zipSize, false};
    juce::ZipFile zipFile{zipStream};

    if (auto* zipEntry = zipFile.getEntry(ZIPPED_FILES_PREFIX + filepath))
    {
        const std::unique_ptr<juce::InputStream> entryStream{zipFile.createStreamForEntry(*zipEntry)};

        if (entryStream == nullptr)
        {
            jassertfalse;
            return {};
        }

        return streamToVector(*entryStream);
    }

    return {};
}

constexpr auto kLocalDevServerAddress{"http://localhost:8080"};

} // namespace

//==============================================================================
SynpleAudioProcessorEditor::SynpleAudioProcessorEditor(SynpleAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webView_(juce::WebBrowserComponent::Options()
                   .withResourceProvider([this](const auto& url) { return getResource(url); },
                                         juce::URL(kLocalDevServerAddress).getOrigin())
                   .withNativeIntegrationEnabled()
                   .withInitialisationData("vendor", JUCE_COMPANY_NAME)
                   .withInitialisationData("pluginName", JUCE_PRODUCT_NAME)
                   .withInitialisationData("pluginVersion", JUCE_PRODUCT_VERSION)
                   .withOptionsFrom(webOsc1VolumeRelay_)
                   .withOptionsFrom(webOsc2VolumeRelay_)
                   .withOptionsFrom(webOsc1TuneRelay_)
                   .withOptionsFrom(webOsc2TuneRelay_)
                   .withOptionsFrom(webOsc1FineRelay_)
                   .withOptionsFrom(webOsc2FineRelay_)
                   .withOptionsFrom(webOsc1WaveformRelay_)
                   .withOptionsFrom(webOsc2WaveformRelay_)
                   .withOptionsFrom(webOsc1PulseWidthRelay_)
                   .withOptionsFrom(webOsc2PulseWidthRelay_)
                   .withOptionsFrom(webGlideModeRelay_)
                   .withOptionsFrom(webGlideRateRelay_)
                   .withOptionsFrom(webGlideBendRelay_)
                   .withOptionsFrom(webFilterFreqRelay_)
                   .withOptionsFrom(webFilterResoRelay_)
                   .withOptionsFrom(webFilterEnvRelay_)
                   .withOptionsFrom(webFilterLFORelay_)
                   .withOptionsFrom(webFilterVelocityRelay_)
                   .withOptionsFrom(webFilterAttackRelay_)
                   .withOptionsFrom(webFilterDecayRelay_)
                   .withOptionsFrom(webFilterSustainRelay_)
                   .withOptionsFrom(webFilterReleaseRelay_)
                   .withOptionsFrom(webEnvAttackRelay_)
                   .withOptionsFrom(webEnvDecayRelay_)
                   .withOptionsFrom(webEnvSustainRelay_)
                   .withOptionsFrom(webEnvReleaseRelay_)
                   .withOptionsFrom(webLfoRateRelay_)
                   .withOptionsFrom(webVibratoRelay_)
                   .withOptionsFrom(webNoiseRelay_)
                   .withOptionsFrom(webOctaveRelay_)
                   .withOptionsFrom(webTuningRelay_)
                   .withOptionsFrom(webOutputLevelRelay_)
                   .withOptionsFrom(webPolyModeRelay_)
                   .withNativeFunction("setWindowSize",
                                       [this](const juce::Array<juce::var>& args, auto complete) {
                                           if (args.size() >= 2)
                                               juce::MessageManager::callAsync(
                                                   [this, w = (int)args[0], h = (int)args[1]]() { setSize(w, h); });
                                           complete({});
                                       })),
      webOsc1VolumeAttachment_(p.getParameter(ParameterIds::osc1Volume), webOsc1VolumeRelay_),
      webOsc2VolumeAttachment_(p.getParameter(ParameterIds::osc2Volume), webOsc2VolumeRelay_),
      webOsc1TuneAttachment_(p.getParameter(ParameterIds::osc1Tune), webOsc1TuneRelay_),
      webOsc2TuneAttachment_(p.getParameter(ParameterIds::osc2Tune), webOsc2TuneRelay_),
      webOsc1FineAttachment_(p.getParameter(ParameterIds::osc1Fine), webOsc1FineRelay_),
      webOsc2FineAttachment_(p.getParameter(ParameterIds::osc2Fine), webOsc2FineRelay_),
      webOsc1WaveformAttachment_(p.getParameter(ParameterIds::osc1Waveform), webOsc1WaveformRelay_),
      webOsc2WaveformAttachment_(p.getParameter(ParameterIds::osc2Waveform), webOsc2WaveformRelay_),
      webOsc1PulseWidthAttachment_(p.getParameter(ParameterIds::osc1PulseWidth), webOsc1PulseWidthRelay_),
      webOsc2PulseWidthAttachment_(p.getParameter(ParameterIds::osc2PulseWidth), webOsc2PulseWidthRelay_),
      webGlideModeAttachment_(p.getParameter(ParameterIds::glideMode), webGlideModeRelay_),
      webGlideRateAttachment_(p.getParameter(ParameterIds::glideRate), webGlideRateRelay_),
      webGlideBendAttachment_(p.getParameter(ParameterIds::glideBend), webGlideBendRelay_),
      webFilterFreqAttachment_(p.getParameter(ParameterIds::filterFreq), webFilterFreqRelay_),
      webFilterResoAttachment_(p.getParameter(ParameterIds::filterReso), webFilterResoRelay_),
      webFilterEnvAttachment_(p.getParameter(ParameterIds::filterEnv), webFilterEnvRelay_),
      webFilterLFOAttachment_(p.getParameter(ParameterIds::filterLFO), webFilterLFORelay_),
      webFilterVelocityAttachment_(p.getParameter(ParameterIds::filterVelocity), webFilterVelocityRelay_),
      webFilterAttackAttachment_(p.getParameter(ParameterIds::filterAttack), webFilterAttackRelay_),
      webFilterDecayAttachment_(p.getParameter(ParameterIds::filterDecay), webFilterDecayRelay_),
      webFilterSustainAttachment_(p.getParameter(ParameterIds::filterSustain), webFilterSustainRelay_),
      webFilterReleaseAttachment_(p.getParameter(ParameterIds::filterRelease), webFilterReleaseRelay_),
      webEnvAttackAttachment_(p.getParameter(ParameterIds::envAttack), webEnvAttackRelay_),
      webEnvDecayAttachment_(p.getParameter(ParameterIds::envDecay), webEnvDecayRelay_),
      webEnvSustainAttachment_(p.getParameter(ParameterIds::envSustain), webEnvSustainRelay_),
      webEnvReleaseAttachment_(p.getParameter(ParameterIds::envRelease), webEnvReleaseRelay_),
      webLfoRateAttachment_(p.getParameter(ParameterIds::lfoRate), webLfoRateRelay_),
      webVibratoAttachment_(p.getParameter(ParameterIds::vibrato), webVibratoRelay_),
      webNoiseAttachment_(p.getParameter(ParameterIds::noise), webNoiseRelay_),
      webOctaveAttachment_(p.getParameter(ParameterIds::octave), webOctaveRelay_),
      webTuningAttachment_(p.getParameter(ParameterIds::tuning), webTuningRelay_),
      webOutputLevelAttachment_(p.getParameter(ParameterIds::outputLevel), webOutputLevelRelay_),
      webPolyModeAttachment_(p.getParameter(ParameterIds::polyMode), webPolyModeRelay_)
{
    webView_.goToURL(webView_.getResourceProviderRoot());
    // webView_.goToURL(kLocalDevServerAddress);

    addAndMakeVisible(webView_);
    setResizable(true, true);
    setResizeLimits(400, 150, 2000, 1000);
    setSize(800, 500);
}

SynpleAudioProcessorEditor::~SynpleAudioProcessorEditor()
{
    processorRef.midiLearn.store(false);
}

void SynpleAudioProcessorEditor::resized()
{
    webView_.setBounds(getLocalBounds());
}

std::optional<SynpleAudioProcessorEditor::Resource> SynpleAudioProcessorEditor::getResource(const juce::String& url)
{
    const auto resourceToRetrieve{url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false)};

    const auto resource{getWebViewFileAsBytes(resourceToRetrieve)};

    if (!resource.empty())
    {
        const auto extension{resourceToRetrieve.fromLastOccurrenceOf(".", false, false)};
        return Resource(std::move(resource), getMimeForExtension(extension));
    }

    return std::nullopt;
}
