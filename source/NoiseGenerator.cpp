#include "NoiseGenerator.h"

void NoiseGenerator::reset()
{
    random_.setSeedRandomly();
}

float NoiseGenerator::nextValue()
{
    return random_.nextFloat();
}
