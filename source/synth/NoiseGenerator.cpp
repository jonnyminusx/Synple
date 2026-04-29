#include "NoiseGenerator.h"

#include <chrono>

namespace synth
{

void NoiseGenerator::reset()
{
    rng_.seed(static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count()));
}

float NoiseGenerator::nextValue()
{
    return dist_(rng_);
}

} // namespace synth
