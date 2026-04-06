#pragma once

namespace synth
{

struct ADSR
{
    float attack{0.0f};
    float decay{0.0f};
    float sustain{1.0f};
    float release{0.0f};
};

} // namespace synth
