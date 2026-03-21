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

} // namespace

//==============================================================================
JLX11AudioProcessorEditor::JLX11AudioProcessorEditor(JLX11AudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webView_(juce::WebBrowserComponent::Options()
                   .withResourceProvider([this](const auto& url) { return getResource(url); })
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
                   .withEventListener("exampleJavaScriptEvent", [this](juce::var objectFromFrontEnd) {
                       labelUpdatedFromJavaScript_.setText(
                           "Received event from JavaScript with data: " +
                               objectFromFrontEnd.getProperty("emittedCount", 0).toString(),
                           juce::dontSendNotification);
                   }))

{
    webView_.goToURL(webView_.getResourceProviderRoot());

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

    addAndMakeVisible(webView_);
    addAndMakeVisible(runJavaScriptButton_);
    addAndMakeVisible(emitJavaScriptEventButton_);
    addAndMakeVisible(labelUpdatedFromJavaScript_);

    setResizable(true, true);
    setSize(800, 600);
}

JLX11AudioProcessorEditor::~JLX11AudioProcessorEditor()
{
    midiLearnButton_.removeListener(this);
    processorRef.midiLearn.store(false);
}

void JLX11AudioProcessorEditor::resized()
{
    auto bounds{getLocalBounds()};
    webView_.setBounds(bounds.removeFromRight(bounds.getWidth() / 2));
    runJavaScriptButton_.setBounds(bounds.removeFromTop(50).reduced(5));
    emitJavaScriptEventButton_.setBounds(bounds.removeFromTop(50).reduced(5));
    labelUpdatedFromJavaScript_.setBounds(bounds.removeFromTop(50).reduced(5));
}

void JLX11AudioProcessorEditor::buttonClicked(juce::Button* button)
{
    button->setButtonText("Waiting...");
    button->setEnabled(false);
    processorRef.midiLearn.store(true);

    startTimerHz(10);
}

void JLX11AudioProcessorEditor::timerCallback()
{
    if (!processorRef.midiLearn.load())
    {
        stopTimer();
        midiLearnButton_.setButtonText("MIDI Learn");
        midiLearnButton_.setEnabled(true);
    }
}

std::optional<JLX11AudioProcessorEditor::Resource> JLX11AudioProcessorEditor::getResource(const juce::String& url)
{
    static const juce::File resourceFileRoot{R"(/Users/jonny/Code/GitHub/JLX11/source/ui/public/)"};
    const auto resourceToRetrieve{url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false)};
    const auto resource{resourceFileRoot.getChildFile(resourceToRetrieve).createInputStream()};

    if (resource)
    {
        const auto extension{resourceToRetrieve.fromLastOccurrenceOf(".", false, false)};
        return Resource(streamToVector(*resource), getMimeForExtension(extension));
    }

    return std::nullopt;
}

void JLX11AudioProcessorEditor::nativeFunction(const juce::Array<juce::var>& args,
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
