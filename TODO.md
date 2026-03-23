# TODOs and Ideas

* Test pitch bend
* Test sustain pedal
* Avoid pop when playing legato style (interpolate/crossfade?)
* Unison mode
* Reverb
* Delay
* Chorus
* After touch
* Better stereo widening
* PWM
* Better oscillator implementation
* Voice management (and other) unit tests
* Velocity sensitivity
* Alternative LFO wave shapes
* Make panning glide
* Make the filter self oscillate
* Make the filter zip coefficient based on sample rate
* Check all sliders for zipper noise and smooth if necessary
* Consider whether the filter envelope value should be in the exp or not
* Add different filter types e.g. Moog ladder filter
* Make the plugin more accessible
* Add frequencey analyser https://docs.juce.com/master/tutorial_spectrum_analyser.html
* Testing 
    * https://github.com/Tracktion/pluginval/blob/develop/docs/Testing%20plugins%20with%20pluginval.md
    * auval (native to MacOS)
    * Thread sanitizer compiler feature
* Performance
    * https://github.com/AndrewJJ/DSP-Testbench
    * https://ddmf.eu/plugindoctor/
* Unit tests
* Write a manual testing plan (See page 365 of Creating Synthesizer Plugins)
* Oscillator ideas
    * Subtract osc2 from osc1, as it is now.
    * Add the two oscillators together.
    * Multiply osc1 by osc2. This is known as ring modulation.
    * Use osc2 to determine the phase of osc1. This does the kind of phase modulation that is performed by FM synths.
    * Hard sync: set osc1 to a higher rate than osc2, but whenever osc2 ends a cycle also reset the phase of osc1. 
    * Different waves:
        * sine wave
        * triangle wave
        * arbitrary shape from a look-up table (wavetable)
        * distorted sine wave
    * Sub bass osciallator
    * Different wave types per oscillator
    * Super saw
