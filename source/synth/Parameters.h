#pragma once

#include "GlideMode.h"

namespace synth
{

struct Parameters
{
    float velocitySensitivity{0.0f};
    float volumeTrim{0.0f};
    float oscillatorMix{0.0f};
    float tune{0.0f};
    float detune{0.0f};
    float glideBend{0.0f};
    GlideMode glideMode{GlideMode::Off};
};

} // namespace synth
