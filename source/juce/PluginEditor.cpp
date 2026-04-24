#include "PluginEditor.h"
#include "PluginProcessor.h"

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

constexpr auto localDevServerAddress{"http://127.0.0.1:8080"};

} // namespace

//==============================================================================
SynpleAudioProcessorEditor::SynpleAudioProcessorEditor(SynpleAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      outputLevelAttachment_(*processorRef.getApvts().getParameter(parameter_id::outputLevel.getParamID()),
                             outputLevelSlider_),
      polyphonicToggleAttachment_(*processorRef.getApvts().getParameter(parameter_id::polyMode.getParamID()),
                                  polyphonicToggle_),
      glideModeAttachment_(*processorRef.getApvts().getParameter(parameter_id::glideMode.getParamID()),
                           glideModeComboBox_),
      webOutputLevelRelay_(parameter_id::outputLevel.getParamID()),
      webPolyphonicToggleRelay_(parameter_id::polyMode.getParamID()),
      webGlideModeRelay_(parameter_id::glideMode.getParamID()),
      webView_(juce::WebBrowserComponent::Options()
                   .withResourceProvider([this](const auto& url) { return getResource(url); },
                                         juce::URL(localDevServerAddress).getOrigin())
                   .withNativeIntegrationEnabled()
                   .withUserScript(R"(console.log("Hello from injected user script!");)")
                   .withInitialisationData("vendor", JUCE_COMPANY_NAME)
                   .withInitialisationData("pluginName", JUCE_PRODUCT_NAME)
                   .withInitialisationData("pluginVersion", JUCE_PRODUCT_VERSION)
                   .withNativeFunction(juce::Identifier("nativeFunction"),
                                       [this](const juce::Array<juce::var>& args,
                                              juce::WebBrowserComponent::NativeFunctionCompletion completion) {
                                           nativeFunction(args, std::move(completion));
                                       })
                   .withEventListener("exampleJavaScriptEvent",
                                      [this](juce::var objectFromFrontEnd) {
                                          labelUpdatedFromJavaScript_.setText(
                                              "Received event from JavaScript with data: " +
                                                  objectFromFrontEnd.getProperty("emittedCount", 0).toString(),
                                              juce::dontSendNotification);
                                      })
                   .withOptionsFrom(webOutputLevelRelay_)
                   .withOptionsFrom(webPolyphonicToggleRelay_)
                   .withOptionsFrom(webGlideModeRelay_)),
      webOutputLevelAttachment_(*processorRef.getApvts().getParameter(parameter_id::outputLevel.getParamID()),
                                webOutputLevelRelay_),
      webPolyphonicToggleAttachment_(*processorRef.getApvts().getParameter(parameter_id::polyMode.getParamID()),
                                     webPolyphonicToggleRelay_),
      webGlideModeAttachment_(*processorRef.getApvts().getParameter(parameter_id::glideMode.getParamID()),
                              webGlideModeRelay_)
{
    // webView_.goToURL(webView_.getResourceProviderRoot());
    webView_.goToURL(localDevServerAddress);

    runJavaScriptButton_.onClick = [this] {
        constexpr auto javaScriptToRun = "console.log(\"Hello from C++!\");";
        webView_.evaluateJavascript(javaScriptToRun, [](const juce::WebBrowserComponent::EvaluationResult& result) {
            if (const auto* resulPtr = result.getResult())
            {
                std::cout << "JavaScript evaluation result: " << resulPtr->toString() << std::endl;
            }
            else
            {
                std::cout << "JavaScript evaluation error: " << result.getError()->message << std::endl;
            }
        });
    };

    emitJavaScriptEventButton_.onClick = [this] {
        static const juce::Identifier eventId{"exampleEvent"};
        webView_.emitEventIfBrowserIsVisible(eventId, 42.0);
    };

    labelUpdatedFromJavaScript_.setColour(juce::Label::textColourId, juce::Colours::black);
    polyphonicToggle_.setColour(juce::ToggleButton::textColourId, juce::Colours::black);
    polyphonicToggle_.setColour(juce::ToggleButton::tickColourId, juce::Colours::black);
    polyphonicToggle_.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::black);

    addAndMakeVisible(webView_);
    addAndMakeVisible(outputLevelSlider_);
    addAndMakeVisible(polyphonicToggle_);
    addAndMakeVisible(glideModeLabel_);
    addAndMakeVisible(glideModeComboBox_);
    addAndMakeVisible(runJavaScriptButton_);
    addAndMakeVisible(emitJavaScriptEventButton_);
    addAndMakeVisible(labelUpdatedFromJavaScript_);

    outputLevelSlider_.setSliderStyle(juce::Slider::LinearBar);

    juce::AudioParameterChoice* glideModeParam{static_cast<juce::AudioParameterChoice*>(
        processorRef.getApvts().getParameter(parameter_id::glideMode.getParamID()))};
    glideModeComboBox_.addItemList(glideModeParam->choices, 1);
    glideModeComboBox_.setSelectedId(glideModeParam->getIndex() + 1, juce::dontSendNotification);

    setResizable(true, true);
    setSize(800, 600);

    startTimer(60);
}

SynpleAudioProcessorEditor::~SynpleAudioProcessorEditor()
{
    midiLearnButton_.removeListener(this);
    processorRef.midiLearn.store(false);
}

void SynpleAudioProcessorEditor::resized()
{
    auto bounds{getLocalBounds()};
    webView_.setBounds(bounds.removeFromRight(bounds.getWidth() / 2));
    runJavaScriptButton_.setBounds(bounds.removeFromTop(50).reduced(5));
    emitJavaScriptEventButton_.setBounds(bounds.removeFromTop(50).reduced(5));
    labelUpdatedFromJavaScript_.setBounds(bounds.removeFromTop(50).reduced(5));
    outputLevelSlider_.setBounds(bounds.removeFromTop(50).reduced(5));
    polyphonicToggle_.setBounds(bounds.removeFromTop(50).reduced(5));
    glideModeLabel_.setBounds(bounds.removeFromTop(50).reduced(5));
    glideModeComboBox_.setBounds(bounds.removeFromTop(50).reduced(5));
}

void SynpleAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    button->setButtonText("Waiting...");
    button->setEnabled(false);
    processorRef.midiLearn.store(true);

    startTimerHz(10);
}

void SynpleAudioProcessorEditor::timerCallback()
{
    // if (!processorRef.midiLearn.load())
    // {
    //     stopTimer();
    //     midiLearnButton_.setButtonText("MIDI Learn");
    //     midiLearnButton_.setEnabled(true);
    // }

    // NOTE: this call causes an assertion failure in the webview due to passing an empty var.
    webView_.emitEventIfBrowserIsVisible("outputLevel", juce::var());
}

std::optional<SynpleAudioProcessorEditor::Resource> SynpleAudioProcessorEditor::getResource(const juce::String& url)
{
    static const juce::File resourceFileRoot{R"(../ui/public/)"};
    const auto resourceToRetrieve{url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false)};

    if (resourceToRetrieve == "outputLevel.json")
    {
        std::cout << "Resource requested: " << resourceToRetrieve << std::endl;

        juce::DynamicObject::Ptr data(new juce::DynamicObject());
        data->setProperty("left", processorRef.outputLevelLeft.load());
        const juce::String string{juce::JSON::toString(data.get())};
        juce::MemoryInputStream stream{string.getCharPointer(), string.getNumBytesAsUTF8(), false};
        return Resource(streamToVector(stream), juce::String("application/json"));
    }

    if (resourceToRetrieve == "data.json")
    {
        juce::DynamicObject::Ptr data(new juce::DynamicObject());
        data->setProperty("sampleProperty", 30.0);
        const juce::String string{juce::JSON::toString(data.get())};
        juce::MemoryInputStream stream{string.getCharPointer(), string.getNumBytesAsUTF8(), false};
        return Resource(streamToVector(stream), juce::String("application/json"));
    }

    const auto resource{resourceFileRoot.getChildFile(resourceToRetrieve).createInputStream()};

    if (resource)
    {
        const auto extension{resourceToRetrieve.fromLastOccurrenceOf(".", false, false)};
        return Resource(streamToVector(*resource), getMimeForExtension(extension));
    }

    return std::nullopt;
}

void SynpleAudioProcessorEditor::nativeFunction(const juce::Array<juce::var>& args,
                                                juce::WebBrowserComponent::NativeFunctionCompletion completion)
{
    juce::String concatanatedArgs;
    for (const auto& arg : args)
    {
        concatanatedArgs += arg.toString() + " ";
    }

    labelUpdatedFromJavaScript_.setText("Native function called with args: " + concatanatedArgs,
                                        juce::dontSendNotification);

    completion(juce::var("Response from native function"));
}
