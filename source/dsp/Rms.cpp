#include "Rms.h"

#include <cassert>
#include <cmath>

namespace dsp
{

float rms(std::span<const float> v)
{
    assert(!v.empty());
    float sum{0.0f};
    for (float s : v)
        sum += s * s;
    return std::sqrt(sum / static_cast<float>(v.size()));
}

} // namespace dsp
