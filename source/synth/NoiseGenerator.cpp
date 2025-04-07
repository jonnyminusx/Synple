#include "NoiseGenerator.h"

namespace synth
{

void NoiseGenerator::reset()
{
    random_.setSeedRandomly();
}

float NoiseGenerator::nextValue()
{
    return random_.nextFloat();
}

} // namespace synth
