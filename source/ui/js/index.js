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
