#pragma once

#include <random>

namespace synth
{

class NoiseGenerator
{
  public:
    void reset();
    float nextValue();

  private:
    std::mt19937 rng_;
    std::uniform_real_distribution<float> dist_{0.0f, 1.0f};
};

} // namespace synth
