#pragma once

namespace synth
{

class INoiseGenerator
{
  public:
    virtual ~INoiseGenerator() = default;

    virtual void reset() = 0;
    virtual float nextValue() = 0;
};

} // namespace synth
