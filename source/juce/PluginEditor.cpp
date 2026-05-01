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

constexpr auto localDevServerAddress{"http://localhost:8080"};

} // namespace

//==============================================================================
SynpleAudioProcessorEditor::SynpleAudioProcessorEditor(SynpleAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webView_(juce::WebBrowserComponent::Options()
                   .withResourceProvider([this](const auto& url) { return getResource(url); },
                                         juce::URL(localDevServerAddress).getOrigin())
                   .withNativeIntegrationEnabled()
                   .withInitialisationData("vendor", JUCE_COMPANY_NAME)
                   .withInitialisationData("pluginName", JUCE_PRODUCT_NAME)
                   .withInitialisationData("pluginVersion", JUCE_PRODUCT_VERSION)
                   .withOptionsFrom(webOscMixRelay_)
                   .withOptionsFrom(webOscTuneRelay_)
                   .withOptionsFrom(webOscFineRelay_)
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
      webOscMixAttachment_(*p.getApvts().getParameter(parameter_id::oscMix.getParamID()), webOscMixRelay_),
      webOscTuneAttachment_(*p.getApvts().getParameter(parameter_id::oscTune.getParamID()), webOscTuneRelay_),
      webOscFineAttachment_(*p.getApvts().getParameter(parameter_id::oscFine.getParamID()), webOscFineRelay_),
      webGlideModeAttachment_(*p.getApvts().getParameter(parameter_id::glideMode.getParamID()), webGlideModeRelay_),
      webGlideRateAttachment_(*p.getApvts().getParameter(parameter_id::glideRate.getParamID()), webGlideRateRelay_),
      webGlideBendAttachment_(*p.getApvts().getParameter(parameter_id::glideBend.getParamID()), webGlideBendRelay_),
      webFilterFreqAttachment_(*p.getApvts().getParameter(parameter_id::filterFreq.getParamID()), webFilterFreqRelay_),
      webFilterResoAttachment_(*p.getApvts().getParameter(parameter_id::filterReso.getParamID()), webFilterResoRelay_),
      webFilterEnvAttachment_(*p.getApvts().getParameter(parameter_id::filterEnv.getParamID()), webFilterEnvRelay_),
      webFilterLFOAttachment_(*p.getApvts().getParameter(parameter_id::filterLFO.getParamID()), webFilterLFORelay_),
      webFilterVelocityAttachment_(*p.getApvts().getParameter(parameter_id::filterVelocity.getParamID()),
                                   webFilterVelocityRelay_),
      webFilterAttackAttachment_(*p.getApvts().getParameter(parameter_id::filterAttack.getParamID()),
                                 webFilterAttackRelay_),
      webFilterDecayAttachment_(*p.getApvts().getParameter(parameter_id::filterDecay.getParamID()),
                                webFilterDecayRelay_),
      webFilterSustainAttachment_(*p.getApvts().getParameter(parameter_id::filterSustain.getParamID()),
                                  webFilterSustainRelay_),
      webFilterReleaseAttachment_(*p.getApvts().getParameter(parameter_id::filterRelease.getParamID()),
                                  webFilterReleaseRelay_),
      webEnvAttackAttachment_(*p.getApvts().getParameter(parameter_id::envAttack.getParamID()), webEnvAttackRelay_),
      webEnvDecayAttachment_(*p.getApvts().getParameter(parameter_id::envDecay.getParamID()), webEnvDecayRelay_),
      webEnvSustainAttachment_(*p.getApvts().getParameter(parameter_id::envSustain.getParamID()), webEnvSustainRelay_),
      webEnvReleaseAttachment_(*p.getApvts().getParameter(parameter_id::envRelease.getParamID()), webEnvReleaseRelay_),
      webLfoRateAttachment_(*p.getApvts().getParameter(parameter_id::lfoRate.getParamID()), webLfoRateRelay_),
      webVibratoAttachment_(*p.getApvts().getParameter(parameter_id::vibrato.getParamID()), webVibratoRelay_),
      webNoiseAttachment_(*p.getApvts().getParameter(parameter_id::noise.getParamID()), webNoiseRelay_),
      webOctaveAttachment_(*p.getApvts().getParameter(parameter_id::octave.getParamID()), webOctaveRelay_),
      webTuningAttachment_(*p.getApvts().getParameter(parameter_id::tuning.getParamID()), webTuningRelay_),
      webOutputLevelAttachment_(*p.getApvts().getParameter(parameter_id::outputLevel.getParamID()),
                                webOutputLevelRelay_),
      webPolyModeAttachment_(*p.getApvts().getParameter(parameter_id::polyMode.getParamID()), webPolyModeRelay_)
{
    webView_.goToURL(webView_.getResourceProviderRoot());
    // webView_.goToURL(localDevServerAddress);

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
