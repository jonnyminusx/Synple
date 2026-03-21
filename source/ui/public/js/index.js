import * as Juce from "./juce/index.js";

console.log("This is the Javascript frontend!");
console.log(window.__JUCE__.backend);
window.__JUCE__.backend.addEventListener(
    "exampleEvent",
    (objectFromCppBackend) => {
        console.log("Received event from C++ backend: ", objectFromCppBackend);
    }
)

const data = window.__JUCE__.initialisationData;

document.getElementById("vendor").innerHTML = data.vendor;
document.getElementById("pluginName").innerHTML = data.pluginName;
document.getElementById("pluginVersion").innerHTML = data.pluginVersion;

const nativeFunction = Juce.getNativeFunction("nativeFunction");

fetch(Juce.getBackendResourceAddress("data.json"))
    .then(response => response.text())
    .then(data => {
        console.log("Fetched data: ", data);
    });

document.addEventListener("DOMContentLoaded", () => {
    const button = document.getElementById("nativeFunctionButton");
    button.addEventListener("click", () => {
        nativeFunction("one", 2, null).then(result => {
            console.log("Result from native function: ", result);
        });
    });

    const emitEventButton = document.getElementById("emitEventButton");
    let emittedCount = 0;
    emitEventButton.addEventListener("click", () => {
        emittedCount++;
        window.__JUCE__.backend.emitEvent("exampleJavaScriptEvent", { emittedCount: emittedCount });
    });
});


