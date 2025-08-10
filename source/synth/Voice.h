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

    std::optional<int> note() const;
    float render(const float input);

    void setSampleRate(const float sampleRate);
    void setEnvelopeDecay(const float decayTime);

  private:
    std::optional<int> note_;
    float sampleRate_{0.0f};
    float saw_{0.0f};
    Oscillator oscillator_;
    Envelope envelope_;
};

} // namespace synth
