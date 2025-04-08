#pragma once

#include "Oscillator.h"
#include <optional>

namespace synth
{

class Voice
{
  public:
    void reset();
    void noteOn(const int note, const int velocity);
    void noteOff(const int note);

    std::optional<int> note() const;
    float render();

    void setSampleRate(const float sampleRate);

  private:
    std::optional<int> note_;
    Oscillator oscillator_;
};

} // namespace synth
