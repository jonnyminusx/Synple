#include "Mean.h"

#include <cassert>
#include <numeric>

namespace dsp
{

float mean(std::span<const float> v)
{
    assert(!v.empty());
    return std::accumulate(v.begin(), v.end(), 0.0f) / static_cast<float>(v.size());
}

} // namespace dsp
