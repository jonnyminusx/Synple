#pragma once

#include "Envelope.h"
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
    void release();
    float render(const float input);

    const Envelope& envelope() const;
    Envelope& envelope();

    void setSampleRate(const float sampleRate);

  private:
    int note_{0};

    float sampleRate_{0.0f};
    float saw_{0.0f};

    Oscillator oscillator_;
    Envelope envelope_;
};

} // namespace synth
